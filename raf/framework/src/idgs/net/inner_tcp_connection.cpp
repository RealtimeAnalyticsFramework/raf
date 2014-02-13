
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "inner_tcp_connection.h"
#include "idgs/net/network_statistics.h"
#include "inner_tcp_server.h"
#include "idgs/actor/actor_message_queue.h"
#include "idgs/cluster/cluster_framework.h"


namespace idgs {
namespace net {

InnerTcpServer* InnerTcpConnection::innerTcpServer = NULL;

InnerTcpConnection::InnerTcpConnection(asio::io_service& ios) :io_service(ios), socket(ios) {
  peerMemberId = -1;
  queue = NULL;
  state.store(INITIAL);
}

InnerTcpConnection::~InnerTcpConnection() {
  function_footprint();

  if (socket.is_open()) {
    socket.close();
  }
}

void InnerTcpConnection::terminate() {
  auto oldState = state.load();
  auto memberId = peerMemberId;
  state.store(TERMINATED);
  if (socket.is_open()) {
    socket.close();
  }
  if(peerMemberId != -1) {
    if(!InnerTcpConnection::innerTcpServer->resetConnection(peerMemberId, shared_from_this())) {
      LOG(ERROR) << "Failed to reset connection, Connection to peer " << memberId << " has been set to other connection.";
    }
  } else {
    LOG(ERROR) << "Connection state: " << oldState;
  }
}


tbb::concurrent_queue<idgs::actor::ActorMessagePtr>& InnerTcpConnection::getQueue() {
  if(unlikely(!queue)) {
    queue = &(InnerTcpConnection::innerTcpServer->getQueue(peerMemberId));
  }
  return *queue;
}


int InnerTcpConnection::accept() {
  InnerTcpHandShake handshake;
  DVLOG(2) << "begin to receive hand shake.";
  state.store(CONNECTING);
  asio::error_code error;

  // recv handshake
  try {
    asio::read(socket, asio::buffer((&handshake), sizeof(InnerTcpHandShake)), asio::transfer_all(), error);
  } catch (std::exception& e) {
    LOG(ERROR) << "Failed to receive hand shake, exception: " << e.what();
    terminate();
    return RC_CLIENT_SERVER_IS_NOT_AVAILABLE;
  }

  if(error) {
    LOG(ERROR) << "Failed to receive hand shake, error: " << error << "(" << error.message() << ")";
    terminate();
    return RC_CLIENT_SERVER_IS_NOT_AVAILABLE;
  }
  VLOG(2) << "Received hand shake from remote peer: " << handshake.memberId;
  setPeerMemberId(handshake.memberId);
  auto conn = shared_from_this();
  state.store(READY);

  if(InnerTcpConnection::innerTcpServer->setConnection(handshake.memberId, conn)) {
    // begin to receive header
    startRecvHeader();
    realSendMessage();
  } else {
    terminate();
    return RC_CLIENT_SERVER_IS_NOT_AVAILABLE;
  }

  return 0;
}

void InnerTcpConnection::startRecvHeader() {
  auto conn = shared_from_this();
  RpcBuffer* readBuffer(new RpcBuffer());
  try {
    if(unlikely(!socket.is_open())) {
      delete readBuffer;
      terminate();
      return;
    }
    auto handler = [conn, readBuffer] (const asio::error_code& error, const std::size_t& length) {
      conn->handleRecvHeader(error, readBuffer);
    };

    asio::async_read(socket,
        asio::buffer(reinterpret_cast<void*>(readBuffer->getHeader()), sizeof(idgs::net::TcpHeader)),
        asio::transfer_all(),
        handler);
  } catch (std::exception& e) {
    LOG(ERROR) << "Failed to receive header, exception: " << e.what();
    delete readBuffer;
    terminate();
  }

}

void InnerTcpConnection::handleRecvHeader(const asio::error_code& error, RpcBuffer* readBuffer) {
  // handle header
  if (unlikely(error)) {
    LOG_IF(ERROR, error != asio::error::eof && error != asio::error::operation_aborted) << "handle read header data error, caused by " << error.message() << ", close current socket";
    delete readBuffer;
    terminate();
  } else {
    DVLOG(3) << "recv header, size: " << sizeof(idgs::net::TcpHeader) << ", content: " << dumpBinaryBuffer(std::string((char*)readBuffer->getHeader(), sizeof(idgs::net::TcpHeader)));
    readBuffer->decodeHeader();

    // begin to receive body
    try {
      if(unlikely(!socket.is_open())) {
        delete readBuffer;
        terminate();
        return;
      }
      auto conn = shared_from_this();
      asio::async_read(socket, asio::buffer(readBuffer->getBody(), readBuffer->getBodyLength()),
          asio::transfer_all(),
          [conn, readBuffer] (const asio::error_code& error, const std::size_t& length) {
        conn->handleRecvBody(error, readBuffer);
      }
      );
    } catch (std::exception& e) {
      LOG(ERROR) << "Failed to receive body, exception: " << e.what();
      delete readBuffer;
      terminate();
    }
  }
}

void InnerTcpConnection::handleRecvBody(const asio::error_code& error, RpcBuffer* readBuffer) {
  // handle body
  if (unlikely(error)) {
    LOG(ERROR) << "handle read body error , error: " << error << '(' << error.message() << ')';
    delete readBuffer;
    terminate();
  } else {
    // receive next header
    startRecvHeader();

    DVLOG(3) << "recv body size: " << readBuffer->getBodyLength() <<  ", content: " << dumpBinaryBuffer(std::string(readBuffer->getBody(), readBuffer->getBodyLength()));

    uint32_t pos = 0;
    uint32_t totalLength = readBuffer->getBodyLength();
    char* buff = readBuffer->getBody();
    while(pos < totalLength) {
      RpcBuffer* msgBuffer = new RpcBuffer();
      msgBuffer->setBodyLength(*((uint32_t*)(buff + pos)));
      pos += sizeof(idgs::net::TcpHeader);
      msgBuffer->decodeHeader();
      memcpy(msgBuffer->getBody(), (buff + pos), msgBuffer->getBodyLength());
      pos += msgBuffer->getBodyLength();

      std::shared_ptr<idgs::actor::ActorMessage> actorMsg = std::make_shared<idgs::actor::ActorMessage>();
      actorMsg->setRpcBuffer(msgBuffer);
      actorMsg->setMessageOrietation(idgs::actor::ActorMessage::INNER_TCP);

      static NetworkStatistics* stats = InnerTcpConnection::innerTcpServer->network->getNetworkStatistics();
      stats->innerTcpBytesRecv.fetch_add(msgBuffer->getBodyLength() + sizeof(idgs::net::TcpHeader));
      stats->innerTcpPacketRecv.fetch_add(1);
      // dispatch message
      idgs::actor::relayMessage(actorMsg);
    }
    delete readBuffer;
  }
}

int32_t InnerTcpConnection::sendMessage(idgs::actor::ActorMessagePtr& message) {
  realSendMessage();
  return 0;
}

void InnerTcpConnection::realSendMessage() {
  function_footprint();
  static uint32_t BATCH_TCP_MESSAGES = InnerTcpConnection::innerTcpServer->getTcpBatchSize();

  InnerTcpConnectionState expectedState = READY;
  if(!state.compare_exchange_strong(expectedState, WRITING)) {
    VLOG(3) << "Connection is not ready: " << state.load();
    return;
  }
  auto conn = shared_from_this();
  if(unlikely(!socket.is_open())) {
    terminate();
    return;
  }

  idgs::actor::ActorMessagePtr msg;
  std::vector<idgs::actor::ActorMessagePtr>* msgs = new std::vector<idgs::actor::ActorMessagePtr>();
  msgs->reserve(BATCH_TCP_MESSAGES);
  std::vector<asio::const_buffer> outBuffers;
  outBuffers.reserve(BATCH_TCP_MESSAGES * 2 + 1);

  uint32_t sendLength;
  sendLength = 0;
  outBuffers.push_back(asio::buffer(reinterpret_cast<void*>(&sendLength), sizeof(sendLength)));

  while(getQueue().try_pop(msg)) {
    DVLOG(2) << "send message: " << msg->toString();
    DVLOG(3) << "Send head size: " << sizeof(idgs::net::TcpHeader) <<  ", content: " << dumpBinaryBuffer(std::string((char*)msg->getRpcBuffer()->getHeader(), sizeof(idgs::net::TcpHeader)));
    DVLOG(3) << "Send body size: " << msg->getRpcBuffer()->getBodyLength() <<  ", content: " << dumpBinaryBuffer(std::string(msg->getRpcBuffer()->getBody(), msg->getRpcBuffer()->getBodyLength()));

    msgs->push_back(msg);

    outBuffers.push_back(asio::buffer(reinterpret_cast<void*>(msg->getRpcBuffer()->getHeader()), sizeof(idgs::net::TcpHeader)));
    outBuffers.push_back(asio::buffer(reinterpret_cast<void*>(msg->getRpcBuffer()->getBody()), msg->getRpcBuffer()->getBodyLength()));
    sendLength += sizeof(idgs::net::TcpHeader) + msg->getRpcBuffer()->getBodyLength();

    //    if(msgs->size() >= BATCH_TCP_MESSAGES || sendLength > (1024 * 50)) {
    if(msgs->size() >= BATCH_TCP_MESSAGES) {
      break;
    }
  }

  if(msgs->empty()) {
    DVLOG(2) << "No availabe message to send.";
    state.store(READY);
    delete msgs;
    return;
  }
  try {
    DVLOG(2) << "Send " << msgs->size() << " messages";
    if(unlikely(!socket.is_open())) {
      // @fixme messagea lost
      delete msgs;
      terminate();
      return;
    }
    asio::async_write(
        socket,
        outBuffers,
        asio::transfer_all(),
        [msgs, conn] (const asio::error_code& error, const std::size_t& bytes_transferred) {
      DVLOG(2) << "Sent " << msgs->size() << " messages";
      conn->handleSendMessage(error);

      static NetworkStatistics* stats = InnerTcpConnection::innerTcpServer->network->getNetworkStatistics();
      stats->innerTcpBytesSent.fetch_add(bytes_transferred);
      stats->innerTcpPacketSent.fetch_add(msgs->size());

      delete msgs;
    }
    );
  } catch (std::exception& e) {
    LOG(ERROR) << "send message error, exception: " << e.what() << ", messages: " << msgs->size();
    delete msgs;
    terminate();
  } catch (...) {
    LOG(ERROR) << "send message error " << ", messages: " << msgs->size();
    catchUnknownException();
    delete msgs;
    terminate();
  }
}

void InnerTcpConnection::handleSendMessage(const asio::error_code& error) {
  if(unlikely(error)) {
    LOG(ERROR) << "Failed to send message, error: " << error << '(' << error.message() << ')';
    terminate();
  } {
    DVLOG(3) << "Send message successfully";
    state.store(READY);
    realSendMessage();
  }
}

uint32_t InnerTcpConnection::connect(uint32_t memberId, int retry) {
  InnerTcpConnectionState expectedState = INITIAL;
  if(!state.compare_exchange_strong(expectedState, CONNECTING)) {
    DVLOG(2) << "Already connecting to remote peer: " << memberId;
    return 0;
  }
  setPeerMemberId(memberId);
  /// connect to remote peer
  auto ep = ::idgs::util::singleton<idgs::actor::RpcFramework>::getInstance().getNetwork()->getEndPoint(memberId);
  if(ep == NULL) {
    LOG(ERROR) << "Network endpoint of member " << memberId << " is not available.";
    terminate();
    return RC_CLIENT_SERVER_IS_NOT_AVAILABLE;
  }
  auto& end_point = ep->tcpEndPoint;
  DVLOG(0) << "Connecting to remote peer " << memberId << '(' << end_point << ")";
  auto conn = shared_from_this();
  try {
    socket.async_connect(end_point, [conn, retry](const asio::error_code& error) {
      conn->handleConnect(error, retry);
    });
  } catch (std::exception& e) {
    LOG(ERROR) << "Failed to connect to remote peer " << memberId << ", exception: " << e.what();
    terminate();
  }

  return 0;
}

void InnerTcpConnection::handleConnect(const asio::error_code& error, int retry) {
  if (error) {
    LOG(ERROR) << "Failed to connect to remote member " << this->peerMemberId << ", error: " << error << "(" << error.message() << ")";

    if (retry < 3) {
      state.store(INITIAL);
      connect(peerMemberId, retry + 1);
    } else {
      terminate();
    }
    return;
  }
  VLOG(0) << "connected to remote member " << this->peerMemberId << ", local endpoint: " << socket.local_endpoint();

  // start to send handshake
  InnerTcpHandShake handshake;
  handshake.memberId = ::idgs::util::singleton<idgs::cluster::ClusterFramework>::getInstance().getMemberManager()->getLocalMemberId();

  VLOG(1) << "begin to send hand shake to remote peer: " << peerMemberId << ", local id: " << handshake.memberId;
  asio::error_code errorCode;
  try {
    asio::write(socket, asio::buffer(&handshake, sizeof(InnerTcpHandShake)), asio::transfer_all(), errorCode);
  } catch (std::exception& e) {
    LOG(ERROR) << "failed to send handshake, exception: " << e.what();
    terminate();
    return;
  }
  if(errorCode) {
    LOG(ERROR) << "Failed to send handshake to member " << peerMemberId << ", error: " << errorCode << '(' << errorCode.message() << ')';
    terminate();
    return;
  } else {
    VLOG(1) << "sent hand shake to remote peer: " << peerMemberId;
    state.store(READY);
    startRecvHeader();
    realSendMessage();
  }
}


std::string InnerTcpConnection::toString() {
  std::stringstream ss;
  if (peerMemberId != -1 && state.load() >= READY) {
    ss << "Peer(" << peerMemberId << "), state:" << state << ", socket: " << socket.is_open();
    ss << ", queue: " << getQueue().unsafe_size();
  }
  return ss.str();
}

} // namespace net 
} // namespace idgs 
