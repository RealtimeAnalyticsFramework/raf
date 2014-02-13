
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/net/stateful_tcp_actor.h"
#include "idgs/net/network_statistics.h"
#include "idgs/net/async_tcp_server.h"

#include "idgs/actor/rpc_framework.h"
#include "idgs/actor/actor_message_queue.h"


using namespace idgs::pb;
using namespace idgs::actor;

namespace idgs {
namespace net {

AsyncTcpServer* StatefulTcpActor::outerTcpServer = NULL;

StatefulTcpActor::StatefulTcpActor(asio::io_service& io_service, asio::ip::tcp::socket *sock) : io_service_(io_service), socket_(sock) {
  writing.clear();
  static NetworkStatistics* stats = StatefulTcpActor::outerTcpServer->network->getNetworkStatistics();
  stats->outerTcpConnections.fetch_add(1);
}

StatefulTcpActor::~StatefulTcpActor() {
  static NetworkStatistics* stats = StatefulTcpActor::outerTcpServer->network->getNetworkStatistics();
  stats->outerTcpConnections.fetch_sub(1);

  if(socket_) {
    delete socket_;
    socket_ = NULL;
  }
}

/*
 * The innerProcess in StatefulTcpActor is to send the message out
 */
void StatefulTcpActor::process(const ActorMessagePtr& msg)  {
  try {
    if(handleSystemOperation(msg)) {
      DVLOG(5) << "System message: " << msg->toString();
      return;
    }

    RpcMessage* rpcMessage = msg->getRpcMessage().get();

    rpcMessage->mutable_dest_actor()->CopyFrom(rpcMessage->client_actor());
    rpcMessage->clear_client_actor();

    msg->toRpcBuffer();

    DVLOG(3) << "write message: " << msg->toString();

    msg->freePbMemory();

    putMessage(msg);

    realSend();
  } catch(std::exception& e) {
    LOG(ERROR) << e.what();
  } catch(...) {
    catchUnknownException();
  }
}

asio::ip::tcp::socket& StatefulTcpActor::socket() {
  return *socket_;
}

void StatefulTcpActor::startReceiveHeader() {
  RpcBuffer* readBuffer = new RpcBuffer();

  asio::async_read(socket(),
      asio::buffer(reinterpret_cast<void*>(readBuffer->getHeader()), sizeof(idgs::net::TcpHeader)),
      asio::transfer_all(),
      [this, readBuffer] (const asio::error_code& error, const std::size_t& ) {
    this->handle_read_header(error, readBuffer);
  }
  );
}


void StatefulTcpActor::handle_read_header(const asio::error_code& error, RpcBuffer* readBuffer) {
  if (!error && readBuffer->decodeHeader()) {
    DVLOG(3) << "Recv header size: " << sizeof(TcpHeader);
    asio::async_read(socket(), asio::buffer(readBuffer->getBody(), readBuffer->getBodyLength()),
        asio::transfer_all(),
        [this, readBuffer] (const asio::error_code& error, const std::size_t& ) {
      this->handle_read_body(error, readBuffer);
    }
    );
  } else {
    LOG_IF(ERROR, error != asio::error::eof) << "handle read header data error, caused by " << error.message() << ", close current socket";
    /// close socket
    delete readBuffer;
    terminate();
  }
}

void StatefulTcpActor::handle_read_body(const asio::error_code& error, RpcBuffer* readBuffer) {
  if (!error) {
    /// start to recv next header
    startReceiveHeader();
    DVLOG(3) << "Recv body size: " << readBuffer->getBodyLength() <<  ", content: " << dumpBinaryBuffer2(readBuffer->getBody(), readBuffer->getBodyLength());
    std::shared_ptr<ActorMessage> actorMsg(new ActorMessage());
    actorMsg->setRpcBuffer(readBuffer);
    actorMsg->setMessageOrietation(ActorMessage::OUTER_TCP);
    actorMsg->setTcpActorId(getActorId());

    static NetworkStatistics* stats = StatefulTcpActor::outerTcpServer->network->getNetworkStatistics();
    stats->outerTcpBytesRecv.fetch_add(readBuffer->getBodyLength());
    stats->outerTcpPacketRecv.fetch_add(1);

    DVLOG(3) << "Tcp actor '" << getActorId() << "' receive message" << actorMsg->toString() << " end";
    idgs::actor::relayMessage(actorMsg);
  } else {
    LOG(ERROR) << "handle read body error , caused by " << error.message();
    delete readBuffer;
    terminate();
  }
}

void StatefulTcpActor::handle_write(const asio::error_code& error, const idgs::actor::ActorMessagePtr& message, size_t bytes_transferred) {
  writing.clear();

  if (!error) {
    DVLOG(3) << "write Byte size: " << bytes_transferred;
    realSend();
  } else {
    LOG(ERROR) << "Write message error, caused by " << error.message() << ", message: " << message->toString();
    terminate();
  }
}

void StatefulTcpActor::realSend() {
  if(writing.test_and_set()) {
    DVLOG(5) << "writing";
    return;
  }
  idgs::actor::ActorMessagePtr message;
  bool ret = popMessage(&message);
  if (!ret) {
    writing.clear();
    return;
  }


  try {
    idgs::ResultCode rc;

    std::vector<asio::const_buffer> outBuffers = {
        asio::buffer(reinterpret_cast<void*>(message->getRpcBuffer()->getHeader()), sizeof(idgs::net::TcpHeader)),
        asio::buffer(message->getRpcBuffer()->getBody(), message->getRpcBuffer()->getBodyLength())
    };
    DVLOG(3) << "send body byte size: " << message->getRpcBuffer()->getBodyLength() <<  ", content: " << dumpBinaryBuffer2(message->getRpcBuffer()->getBody(), message->getRpcBuffer()->getBodyLength());

    asio::async_write(socket(), outBuffers, asio::transfer_all(),
        [message, this] (const asio::error_code& error, const std::size_t& bytes_transferred) {
      static NetworkStatistics* stats = StatefulTcpActor::outerTcpServer->network->getNetworkStatistics();
      stats->outerTcpBytesSent.fetch_add(bytes_transferred);
      stats->outerTcpPacketSent.fetch_add(1);

      this->handle_write(error, message, bytes_transferred);
    });
  } catch (std::exception& e) {
    LOG(ERROR) << "send message error, caused by " << e.what() << ", message: " << message->toString();
    writing.clear();
  } catch (...) {
    LOG(ERROR) << "send message error " << ", message: " << message->toString();
    catchUnknownException();
    writing.clear();
  }
}

} // namespace net
} // namespace idgs




