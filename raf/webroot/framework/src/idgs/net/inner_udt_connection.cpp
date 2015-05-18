
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(WITH_UDT)

#include "inner_udt_connection.h"


namespace idgs {
namespace net {

InnerUdtServer* InnerUdtConnection::innerUdtServer = NULL;

InnerUdtConnection::InnerUdtConnection() : sendSocket(0), recvSocket(0), peerMemberId(-1), try_pop_count(0) {
  queue.reset();
  state.store(INITIAL);
}

InnerUdtConnection::~InnerUdtConnection() {
  function_footprint();
}

void InnerUdtConnection::terminate() {
  auto conn = shared_from_this();
  auto oldState = state.load();
  if(oldState == TERMINATED) {
    return;
  }
  state.store(TERMINATED);
  UDT::close(sendSocket);
  UDT::close(recvSocket);

  if(peerMemberId != -1 && InnerUdtConnection::innerUdtServer) {
    if (!InnerUdtConnection::innerUdtServer->resetConnectionIfEq(peerMemberId, conn)) {
      LOG(ERROR) << "Failed to reset connection, Connection to peer " << peerMemberId << " has been set to other connection.";
    }
  } else {
    LOG(ERROR) << "Connection state: " << oldState;
  }
  peerMemberId = -1;
  queue.reset();
}

std::shared_ptr<tbb::concurrent_queue<idgs::actor::ActorMessagePtr>> InnerUdtConnection::getQueue() {
  if(likely(InnerUdtConnection::innerUdtServer)) {
    if(unlikely(!queue)) {
      queue = (InnerUdtConnection::innerUdtServer->getQueue(peerMemberId));
    }
  } else {
    queue.reset();
  }

  return queue;
}

int InnerUdtConnection::accept() {
  sockaddr_storage clientaddr;
  int addrlen = sizeof(clientaddr);

  UDTSOCKET recver;

  if ((recver = UDT::accept(recvSocket, (sockaddr*) &clientaddr, &addrlen)) == UDT::INVALID_SOCK) {
    LOG(ERROR) << "accept: " << UDT::getlasterror().getErrorMessage();
    return -1;
  }

  auto conn = shared_from_this();
  DVLOG(2) << "begin to receive hand shake.";
  state.store(CONNECTING);
  asio::error_code error;

  if (InnerUdtConnection::innerUdtServer->setConnection(0, conn)) {
    // begin to receive header
    startRecvHeader(&recver);
    realSendMessage();
  } else {
    terminate();
    return RC_CLIENT_SERVER_IS_NOT_AVAILABLE;
  }

  return 0;
}

void InnerUdtConnection::startRecvHeader(UDTSOCKET* recver) {
  auto conn = shared_from_this();
  std::shared_ptr<RpcBuffer> readBuffer = std::make_shared<RpcBuffer>();
  try {
    if (recver) {
      terminate();
      return;
    }

    handleRecvHeader(recver, readBuffer);
  } catch (std::exception& e) {
    LOG(ERROR) << "Failed to receive header, exception: " << e.what();
    terminate();
  }
}

void InnerUdtConnection::handleRecvHeader(UDTSOCKET* recver, std::shared_ptr<RpcBuffer> readBuffer) {
  char header[readBuffer->getBodyLength()];
  if (UDT::recv(* recver, header, readBuffer->getBodyLength(), 0) == UDT::ERROR) {
    LOG(ERROR) << "recv:" << UDT::getlasterror().getErrorMessage();
    return;
  }

  readBuffer->getHeader()->size = 0;
  readBuffer->decodeHeader();
  handleRecvBody(recver, readBuffer);
}

void InnerUdtConnection::handleRecvBody(UDTSOCKET* recver, std::shared_ptr<RpcBuffer> readBuffer) {
  if (UDT::recv(* recver, readBuffer->getBody(), readBuffer->getBodyLength(), 0) == UDT::ERROR) {
    LOG(ERROR) << "recv:" << UDT::getlasterror().getErrorMessage();
    return;
  }

  accept();

  uint32_t pos = 0;
  uint32_t totalLength = readBuffer->getBodyLength();
  char* buff = readBuffer->getBody();
  while(pos < totalLength) {
    std::shared_ptr<RpcBuffer> msgBuffer = std::make_shared<RpcBuffer>();
    msgBuffer->setBodyLength(*((uint32_t*)(buff + pos)));
    pos += sizeof(idgs::net::TcpHeader);
    msgBuffer->decodeHeader();
    memcpy(msgBuffer->getBody(), (buff + pos), msgBuffer->getBodyLength());
    pos += msgBuffer->getBodyLength();

    std::shared_ptr<idgs::actor::ActorMessage> actorMsg = std::make_shared<idgs::actor::ActorMessage>();
    actorMsg->setRpcBuffer(msgBuffer);
    actorMsg->setMessageOrietation(idgs::actor::ActorMessage::UDP_ORIENTED);

    static NetworkStatistics* stats = InnerUdtConnection::innerUdtServer->network->getNetworkStatistics();
    stats->udpBytesRecv.fetch_add(msgBuffer->getBodyLength() + sizeof(idgs::net::TcpHeader));
    stats->udpPacketRecv.fetch_add(1);
    // dispatch message
    idgs::actor::relayMessage(actorMsg);
  }
}

int32_t InnerUdtConnection::sendMessage(idgs::actor::ActorMessagePtr& message) {
  realSendMessage();
  return 0;
}

void InnerUdtConnection::realSendMessage() {
  idgs::actor::ActorMessagePtr msg;
  while(getQueue() && getQueue()->try_pop(msg)) {
    auto rpcBuf = msg->getRpcBuffer();
    if (UDT::send(sendSocket, (char*)&rpcBuf->getHeader()->size, sizeof(idgs::net::TcpHeader), 0) == UDT::ERROR) {
      LOG(ERROR) << "send: " << UDT::getlasterror().getErrorMessage();
      return;
    }

    if (UDT::send(sendSocket, rpcBuf->getBody(), rpcBuf->getBodyLength(), 0) == UDT::ERROR) {
      LOG(ERROR) << "send: " << UDT::getlasterror().getErrorMessage();
      return;
    }
  }
}

void InnerUdtConnection::handleSendMessage() {
}

uint32_t InnerUdtConnection::connect(uint32_t memberId, int retry) {
  struct addrinfo hints, *peer;

  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  peerMemberId = memberId;

  auto ep = idgs_application()->getRpcFramework()->getNetwork()->getEndPoint(memberId);
  if(ep == NULL) {
    LOG(ERROR) << "Network endpoint of member " << memberId << " is not available.";
    terminate();
    return RC_CLIENT_SERVER_IS_NOT_AVAILABLE;
  }
  auto& end_point = ep->tcpEndPoint;
  auto ip = end_point.address().to_string();
  auto port = std::to_string(end_point.port());

  if (getaddrinfo(ip.c_str(), port.c_str(), &hints, &peer) != 0) {
    LOG(ERROR) << "incorrect server/peer address. " << ip << ":" << port;
    return 0;
  }

  if (UDT::connect(sendSocket, peer->ai_addr, peer->ai_addrlen) == UDT::ERROR) {
    LOG(ERROR) << "connect: " << UDT::getlasterror().getErrorMessage();
    return 0;
  }

  freeaddrinfo(peer);

  state.store(READY);
  return 0;
}

void InnerUdtConnection::handleConnect(int retry) {
}

std::string InnerUdtConnection::toString() {
  return "InnerUdtConnection";
}

} // namespace net 
} // namespace idgs 
#endif // defined(WITH_UDT)
