
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/net/async_udp_server.h"
#include "idgs/net/network_statistics.h"

#include "idgs/actor/actor_message_queue.h"
#include "idgs/application.h"

#if defined(__unix__)
#endif

//#define UDP_RESEND

using asio::ip::udp;
using namespace idgs::actor;

namespace idgs {
namespace net {

AsyncUdpServer::AsyncUdpServer(NetworkModelAsio* net, asio::io_service& _io_service) :
    network(net),io_service(_io_service), recvSocket(NULL), sendSocket(NULL),cfg(NULL) {
  messageId.store(0L);
}

AsyncUdpServer::~AsyncUdpServer() {
  function_footprint();
  stop();
}

void AsyncUdpServer::init(idgs::pb::ClusterConfig* cfg) {
  this->cfg = cfg;
}

int32_t AsyncUdpServer::start() {
  auto inner_addr = cfg->member().inneraddress();
  auto af = inner_addr.af();
  auto address = inner_addr.host();
  auto port = inner_addr.port();
  do {
    // recv socket
    try {
      if(af == idgs::pb::EndPoint_AddressFamily_PAF_INET || af == idgs::pb::EndPoint_AddressFamily_PAF_INET6) {
        recvSocket = new asio::ip::udp::socket(io_service, asio::ip::udp::endpoint(asio::ip::address::from_string(address), port));
      } else {
        LOG(FATAL) << "Temporary not support network family!";
        return 1;
      }
      NetworkModelAsio::setUdpSocketOption(*recvSocket, true);
    } catch (asio::system_error& error) {
      LOG(ERROR) << "$$$$$$$$$$Failed to start inner UDP server at port " << port << ", caused by " << error.what();
      if(error.code().value() == EADDRINUSE) {
        ++port;
        continue;
      }
      return 1;
    }
    break;
  }while(1);
  DVLOG(0) << "Inner UDP receive server has started at address: " << address << ", port: " << port;

  do {
    ++port;
    // send socket
    try {
      if(af == idgs::pb::EndPoint_AddressFamily_PAF_INET || af == idgs::pb::EndPoint_AddressFamily_PAF_INET6) {
        sendSocket = new asio::ip::udp::socket(io_service, asio::ip::udp::endpoint(asio::ip::address::from_string(address), port));
      } else {
        LOG(FATAL) << "Temporary not support network family!";
        return 1;
      }
      NetworkModelAsio::setUdpSocketOption(*sendSocket, false);
    } catch (asio::system_error& error) {
      LOG(ERROR) << "$$$$$$$$$$Failed to start inner UDP server at port " << port << ", caused by " << error.what();
      if(error.code().value() == EADDRINUSE) {
        ++port;
        continue;
      }
      return 1;
    }
    break;
  } while(1);
  DVLOG(0) << "Inner UDP send server has started at address: " << address << ", port: " << port;

  try {
    startRecv();
  } catch (std::exception& e) {
    LOG(ERROR) << "Failed to read UDP socket, the error is " << e.what();
    return 1;
  }
  return 0;
}

void AsyncUdpServer::stop() {
  function_footprint();

  if (recvSocket != NULL) {
    auto bak = recvSocket;
    recvSocket = NULL;
    delete bak;
  }
  if (sendSocket != NULL) {
    auto bak = sendSocket;
    sendSocket = NULL;
    delete bak;
  }
}

void AsyncUdpServer::handle_send(const asio::error_code& error, const ActorMessagePtr& message, const std::size_t& size) {
  if(error) {
    LOG_EVERY_N(ERROR, 20) << "Send message error, caused by "<< error.message() << ", actor message: " << message->toString();
    return;
  }

  static NetworkStatistics* stats = network->getNetworkStatistics();
  if(!message->getResendCount()) {
    stats->udpBytesSent.fetch_add(size);
    stats->udpPacketSent.fetch_add(1);
  } else {
    // resend
    stats->udpBytesResent.fetch_add(size);
    stats->udpPacketResent.fetch_add(1);
  }
  message->setResendCount(message->getResendCount() + 1);
#if defined(UDP_RESEND)
  static ResendScheduler* resend = network->getResendScheduler();
  resend->scheduleResend(message);
#endif
}

idgs::ResultCode AsyncUdpServer::sendMessage(ActorMessagePtr message) {
  if(!recvSocket) {
    LOG(INFO) << "UDP server has shutdown.";
    return RC_ERROR;
  }
  int32_t localMemId = ::idgs::util::singleton<idgs::cluster::ClusterFramework>::getInstance().getMemberManager()->getLocalMemberId();
  int32_t destMemberId = message->getDestMemberId();
  auto ep = network->getEndPoint(destMemberId);
  if (ep == NULL) {
    LOG(ERROR) << " Local Member: " << localMemId << " send message to Dest Member: " << destMemberId
        << " error, caused by " << idgs::getErrorDescription(RC_ENDPOINT_NOT_FOUND) << ", actor message: " << message->toString();
    return RC_ENDPOINT_NOT_FOUND;
  }
  auto& end_point = ep->udpEndPoint;
  idgs::ResultCode rc;
  if(!message->getResendCount()) {
    message->getRpcMessage()->set_message_id(messageId.fetch_add(1));
  }

  if(!sendSocket) {
    return RC_ERROR;
  }

  message->freePbMemory();

  sendSocket->async_send_to(
      asio::buffer(message->getRpcBuffer()->getBody(), message->getRpcBuffer()->getBodyLength()),
      end_point,
      [message, this] (const asio::error_code& error, const std::size_t& size) {
        this->handle_send(error, message, size);
      }
  );
  return RC_OK;
}

void AsyncUdpServer::startRecv() {
  static uint32_t MTU = ::idgs::util::singleton<idgs::cluster::ClusterFramework>::getInstance().getClusterConfig()->mtu();

  RpcBuffer* readBuffer = new RpcBuffer();
  readBuffer->setBodyLength(MTU);
  readBuffer->decodeHeader();

  if(recvSocket && recvSocket->is_open()) {
    recvSocket->async_receive_from(
        asio::buffer(readBuffer->getBody(), MTU), remoteEndpoint,
        [this, readBuffer] (const asio::error_code& error, const std::size_t& size) {
      this->handleRecv(error, readBuffer, size);
    }
    );
  } else {
    delete readBuffer;
    LOG(ERROR) << "Failed to async receive packet.";
  }


}
void AsyncUdpServer::handleRecv(const asio::error_code& error, RpcBuffer* buff, const std::size_t& bytes_transferred) {
  if (recvSocket) {
    startRecv();
  }
  if (error || bytes_transferred <= 0) {
    LOG_IF(ERROR, error.value() != asio::error::operation_aborted) << "Failed to receive UDP packet: " << error.message() << ", size = " << bytes_transferred;
    delete buff;
  } else {
    buff->setBodyLength(bytes_transferred);
    std::shared_ptr<ActorMessage> actorMsg(new ActorMessage());
    actorMsg->setRpcBuffer(buff);
    actorMsg->setMessageOrietation(ActorMessage::UDP_ORIENTED);

    static NetworkStatistics* stats = network->getNetworkStatistics();
    stats->udpBytesRecv.fetch_add(bytes_transferred);
    stats->udpPacketRecv.fetch_add(1);


#if defined(UDP_RESEND)
    actorMsg->parseRpcBuffer();

    if(actorMsg->getOperationName() == OP_ACK_MSG) {
      static ResendScheduler* resend = network->getResendScheduler();
      resend->cancelResendTimer(actorMsg->getRpcMessage()->message_id());
    } else {
      ActorMessagePtr ack = actorMsg->createResponse();
      ack->setOperationName(OP_ACK_MSG);

      /// set ACK via TCP
      // msg->setChannel(TC_TCP);
      static ActorFramework* actorFramework = ::idgs::util::singleton<RpcFramework>::getInstance().getActorFramework();
      actorFramework->send(ack);

      idgs::actor::relayMessage(actorMsg);
    }
#else
    idgs::actor::relayMessage(actorMsg);
#endif
  }
}
} // namespace net
} // namespace idgs
