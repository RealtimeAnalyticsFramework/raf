
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/net/network_model_asio.h"

#include "idgs/application.h"

#include "idgs/httpserver/http_server.h"

#include "idgs/net/async_tcp_server.h"
#include "idgs/net/inner_tcp_server.h"
#include "idgs/net/rpc_member_listener.h"
#include "idgs/net/network_statistics.h"

using namespace idgs::pb;
using namespace idgs::actor;

namespace idgs {
namespace net {

int NetworkModelAsio::SOCKET_BUFFER_SIZE = 1024 * 1024 * 5;

NetworkModelAsio::NetworkModelAsio():
        cfg(NULL),
        ioService(),
        innerTcpServer(new InnerTcpServer(this, ioService)),
        outerTcpServer(new AsyncTcpServer(this, ioService)),
#if defined(WITH_UDT)
        innerUdtServer(new InnerUdtServer(this)),
#endif // defined(WITH_UDT)
        rpcMemberListener(new RpcMemberListener(this)),
        networkStatistics(new NetworkStatistics()) {
  std::string webroot = "webroot";
  std::string conf_dir = "conf/";
  char* temp = getenv("IDGS_HOME");
  if (temp) {
    webroot = temp;
    webroot = webroot + "/webroot";

    conf_dir = temp;
    conf_dir = conf_dir + "/conf/";
  }
  httpServer = new idgs::httpserver::HttpServer(webroot);
  httpServer->addVirtualDir("/conf/", conf_dir);
}

NetworkModelAsio::~NetworkModelAsio() {
  function_footprint();
  shutdown();
}

int32_t NetworkModelAsio::init(idgs::pb::ClusterConfig* cfg) {
  function_footprint();
  this->cfg = cfg;
  NetworkModelAsio::SOCKET_BUFFER_SIZE = cfg->socket_buffer_size();
  idgs_application()->getMemberManager()->addListener(rpcMemberListener);
  innerTcpServer->init(cfg);
  outerTcpServer->init(cfg);
#if defined(WITH_UDT)
  innerUdtServer->init(cfg);
#endif // defined(WITH_UDT)
  return 0;
}

//return RC_SUCCESS if init success, return RC_ERROR if init failed.
int32_t NetworkModelAsio::start() {

///  start inner tcp server
  idgs::ResultCode rc;
  rc = static_cast<idgs::ResultCode>(innerTcpServer->start());
  CHECK_RC(rc);

/// start outer tcp server
  rc = static_cast<idgs::ResultCode>(outerTcpServer->start());
  CHECK_RC(rc);

/// start inner udt server
#if defined(WITH_UDT)
  rc =  static_cast<idgs::ResultCode>(innerUdtServer->start());
  CHECK_RC(rc);
#endif // defined(WITH_UDT)

/// start IO threads
  int io_thread_count = cfg->io_thread_count();
  DVLOG(2) << "IO Thread count is " << io_thread_count;
  ioThreads.reserve(io_thread_count);
  for(int i = 0; i < io_thread_count; i++) {
    std::shared_ptr<std::thread> ioThread = std::make_shared<std::thread>([this, i](){
      try {
        set_thread_name("idgs_io");
        ioService.run();
      } catch (std::exception &e) {
        LOG(ERROR) << "Get exception in IO thread: " << e.what();
      } catch (...) {
        catchUnknownException();
      }
    });
    ioThreads.push_back(ioThread);
  }
/// start IO threads
  return RC_SUCCESS;
}

//return 1 if init success
int32_t NetworkModelAsio::shutdown() {
  function_footprint();

  if(httpServer) {
    httpServer->stop();
    delete httpServer;
    httpServer = NULL;
  }

  auto backup_outerTcpServer = outerTcpServer;
  auto backup_innerTcpServer = innerTcpServer;
#if defined(WITH_UDT)
  auto backup_udtServer = innerUdtServer;
#endif // defined(WITH_UDT)

  outerTcpServer = NULL;
  innerTcpServer = NULL;

#if defined(WITH_UDT)
  innerUdtServer = NULL;
#endif // defined(WITH_UDT)

  if(backup_outerTcpServer) {
    backup_outerTcpServer->stop();
  }

  if(backup_innerTcpServer) {
    backup_innerTcpServer->stop();
  }

#if defined(WITH_UDT)
  if (backup_udtServer) {
    backup_udtServer->stop();
  }
#endif // defined(WITH_UDT)

  LOG(INFO) << "Stopping IO threads.";
  ioService.stop();

  for (std::shared_ptr<std::thread>& mThread : ioThreads) {
    mThread->join();
  }
  ioThreads.clear();

  LOG(INFO) << "Unregister cluster linster callback.";
  if (rpcMemberListener) {
    idgs_application()->getMemberManager()->removeListener(rpcMemberListener);
    delete rpcMemberListener;
    rpcMemberListener = NULL;
  }

  if(backup_outerTcpServer) {
    delete backup_outerTcpServer;
  }

  if(backup_innerTcpServer) {
    delete backup_innerTcpServer;
  }

#if defined(WITH_UDT)
  if(backup_udtServer) {
    delete backup_udtServer;
  }
#endif // defined(WITH_UDT)

  if(networkStatistics) {
    delete networkStatistics;
    networkStatistics = NULL;
  }


  return RC_SUCCESS;
}

int32_t NetworkModelAsio::send(ActorMessagePtr msg) {
//  DVLOG(2) << "Network layer send actor message: " << msg->toString();
//  static uint32_t MTU = idgs_application()->getClusterFramework()->getClusterConfig()->mtu();

#if defined(WITH_UDT)
  if(!innerUdtServer || !innerTcpServer) {
#else  // defined(WITH_UDT)
  if(!innerTcpServer) {
#endif // defined(WITH_UDT)
    LOG(WARNING) << "Network layer has been shutdown.";
    return RC_ERROR;
  }


  int32_t transfer_channel = msg->getRpcMessage()->channel();

  ResultCode rc = msg->toRpcBuffer();
  if(rc) {
    return rc;
  }

  switch (transfer_channel) {
  case TC_AUTO: {
    return innerTcpServer->sendMessage(msg);
//    size_t size = msg->getRpcBuffer()->getBodyLength();
//    if (size <= MTU) {
//      return innerUdtServer->sendMessage(msg);
//    } else {
//      return innerTcpServer->sendMessage(msg);
//    }
  }
  break;
  case TC_TCP: {
    return innerTcpServer->sendMessage(msg);
  }
  break;
  case TC_UNICAST: {
#if defined(WITH_UDT)
    return innerUdtServer->sendMessage(msg);
#else // defined(WITH_UDT)
    return innerTcpServer->sendMessage(msg);
#endif // defined(WITH_UDT)
  }
  break;
  case TC_MULTICAST: {
    return idgs_application()->multicastMessage(msg);
  }
  break;
  default: {
    LOG(WARNING) << "not support transport type";
    return RC_TRANSPORT_TYPE_NOT_SUPPORT;
  }
  }
  return RC_SUCCESS;
}
void NetworkModelAsio::putEndPoint(int32_t memberId, const ::idgs::pb::EndPoint& endPoint) {
  MemberEndPoint ep;
  ep.memberId = memberId;
  std::stringstream ss;
  ss << endPoint.port();
  std::string strPort = ss.str();
  asio::io_service io;
  {
    asio::ip::tcp::resolver resolver(io);
    asio::ip::tcp::resolver::query query(endPoint.host(), strPort);
    try {
      auto it = resolver.resolve(query);
      ep.tcpEndPoint = *it;
    } catch(asio::system_error& error) {
      LOG(ERROR) << "resolve tcp end point at address: " << endPoint.host() << ", port: " << strPort << " error, caused by " << error.what();
      return;
    }
  }
  {
    asio::ip::udp::resolver resolver(io);
    asio::ip::udp::resolver::query query(endPoint.host(), strPort);
    try {
      auto it = resolver.resolve(query);
      ep.udpEndPoint = *it;
    } catch(asio::system_error& error) {
      LOG(ERROR) << "resolve udp end point at address: " << endPoint.host() << ", port: " << strPort << " error, caused by " << error.what();
      return;
    }
  }
  endPointCache[memberId] = ep;
}

/// set socket options, e.g. receive/send buffer size
void NetworkModelAsio::setTcpSocketOption(asio::ip::tcp::socket& s) {
  asio::error_code ec;
  {
    asio::socket_base::receive_buffer_size opt1;
    s.get_option(opt1);
    LOG_FIRST_N(INFO, 1) << "Default TCP socket receive buffer size: " << opt1.value();
    if ( SOCKET_BUFFER_SIZE > opt1.value()) {
      asio::socket_base::receive_buffer_size opt2(SOCKET_BUFFER_SIZE);
      DVLOG(3) << "TCP Socket receive buffer size is set to " << opt2.value();
      s.set_option(opt2, ec);
      if(ec) {
        LOG(ERROR) << "Failed to set TCP socket receive buffer size: " << opt2.value()
                        << ", please tune the OS kernel: sysctl -w net.core.rmem_max=" << SOCKET_BUFFER_SIZE;
      }
    }
  }
  {
    asio::socket_base::send_buffer_size opt1;
    s.get_option(opt1);
    LOG_FIRST_N(INFO, 1) << "Default TCP socket send buffer size: " << opt1.value();
    if ( SOCKET_BUFFER_SIZE > opt1.value()) {
      asio::socket_base::send_buffer_size opt2(SOCKET_BUFFER_SIZE);
      DVLOG(3) << "TCP Socket send buffer size is set to " << opt2.value();
      s.set_option(opt2, ec);
      if(ec) {
        LOG(ERROR) << "Failed to set TCP socket send buffer size: " << opt2.value()
                        << ", please tune the OS kernel: sysctl -w net.core.wmem_max=" << SOCKET_BUFFER_SIZE;
      }
    }
  }

}


} // namespace rpc
} // namespace idgs

