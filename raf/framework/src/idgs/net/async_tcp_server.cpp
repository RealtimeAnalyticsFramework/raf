
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "async_tcp_server.h"

#include "idgs/application.h"
#include "idgs/httpserver/http_server.h"

using namespace idgs::actor;

namespace idgs {
namespace net {
AsyncTcpServer::AsyncTcpServer(NetworkModelAsio* net, asio::io_service& _io_service) : network(net), io_service(_io_service), acceptor(NULL), cfg(NULL) {
  StatefulTcpActor::outerTcpServer = this;
}

AsyncTcpServer::~AsyncTcpServer() {
  function_footprint();
  StatefulTcpActor::outerTcpServer = NULL;
  stop();
}

void AsyncTcpServer::init(idgs::pb::ClusterConfig* cfg) {
  this->cfg = cfg;
}

int32_t AsyncTcpServer::start() {
  auto inner_addr = cfg->member().public_address();
  auto af = inner_addr.af();
  auto address = inner_addr.host();
  auto port = inner_addr.port();
  do {
    try {
      if(af == idgs::pb::EndPoint_AddressFamily_PAF_INET || af == idgs::pb::EndPoint_AddressFamily_PAF_INET6) {
        acceptor = new asio::ip::tcp::acceptor(io_service, asio::ip::tcp::endpoint(asio::ip::address::from_string(address), port));
      } else {
        LOG(FATAL) << "Unsupported network family " << idgs::pb::EndPoint_AddressFamily_Name(af);
        return 1;
      }
      run();
    } catch (asio::system_error& error) {
      LOG(ERROR) << "$$$$$$$$$$Failed to start outer TCP server at port " << port << ", caused by " << error.what();
      if(error.code().value() == EADDRINUSE) {
        ++port;
        continue;
      }
      return 1;
    }
    break;
  }while(1);
  cfg->mutable_member()->mutable_public_address()->set_port(port);
  DVLOG(2) << "Outer TCP server has started at address: " << address << ", port: " << port;
  return 0;
}

void AsyncTcpServer::handle_accept(asio::ip::tcp::socket *sock, const asio::error_code& error) {
  // accept next session
  run();

  if (!error) {
    StatefulTcpActor* tcpActor = NULL;
    uint32_t cookie;
    uint8_t serdes = 0;
    try {
      asio::read(*sock, asio::buffer(reinterpret_cast<void*>(&cookie), sizeof(cookie)), asio::transfer_all());

      if(cookie == IDGS_COOKIE) {
        asio::read(*sock, asio::buffer(reinterpret_cast<void*>(&serdes), sizeof(serdes)), asio::transfer_all());
        if(serdes >= 3) {
          LOG(ERROR) << "Invalid serdes " << serdes;
          return;
        }


        // handle current session
        static ActorManager* af = idgs_application()->getRpcFramework()->getActorManager();
        tcpActor = new StatefulTcpActor(io_service, sock);
        tcpActor->setActorId(af->generateActorId(tcpActor));
        af->registerSessionActor(tcpActor->getActorId(), tcpActor);

        NetworkModelAsio::setTcpSocketOption(tcpActor->socket());

        tcpActor->setClientSerdes(static_cast<protobuf::SerdesMode>(serdes));


        tcpActor->startReceiveHeader();
      } else {
        VLOG(2) << "Get Http Connection";
        idgs_application()->getRpcFramework()->getNetwork()->getHttpServer()->process(
            sock, reinterpret_cast<char*>(&cookie), sizeof(cookie));
      }
    } catch (std::exception& e) {
      asio::ip::tcp::endpoint endpoint = sock->remote_endpoint();
      LOG(ERROR) << "Failed to handle incoming socket:" << endpoint.address().to_string() << ':' << endpoint.port() << " " << e.what();

      if(tcpActor) {
        tcpActor->terminate();
      } else {
        delete sock;
      }
    }
  } else {
    LOG_IF(ERROR, error.value() != asio::error::operation_aborted) << "Failed to accept,  " << error.message();
    delete sock;
  }
}

void AsyncTcpServer::run() {
  function_footprint();
  asio::ip::tcp::socket* sock = NULL;
  try {
    sock = new asio::ip::tcp::socket(io_service);

    if(!acceptor || !acceptor->is_open()) {
      LOG(INFO) << "Shutdown server, terminate TCP actor. ";
      delete sock;
      sock = NULL;
      return;
    }

    acceptor->async_accept(*sock,
        [sock, this] (const asio::error_code& error) {
      this->handle_accept(sock, error);
    });
  } catch (std::exception &e) {
    LOG(ERROR) << "Get exception when accept: " << e.what();
    if(sock) {
      delete sock;
    }
  } catch (...) {
    if(sock) {
      delete sock;
    }
    catchUnknownException();
  }
}

void AsyncTcpServer::stop() {
  function_footprint();
  DVLOG(2)<<"TcpAsynServer is stopped ";
  if(acceptor != NULL ) {
    auto bak = acceptor;
    acceptor = NULL;
    if(bak->is_open()) {
      bak->close();
    }
    delete bak;
  }
}
} // namespace net
} // namespace idgs
