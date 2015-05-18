
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__)
#include "idgs_gch.h"
#endif // GNUC_ $

#include "inner_tcp_server.h"
#include "idgs/application.h"

namespace idgs {
namespace net {

#define MAX_MEMBERS 100

InnerTcpServer::InnerTcpServer(NetworkModelAsio* net, asio::io_service& _io_service) : network(net), io_service(_io_service), acceptor(NULL),cfg(NULL) {
  connections.resize(MAX_MEMBERS);
  for(int i = 0; i < MAX_MEMBERS; ++i) {
    queues.push_back(std::make_shared<tbb::concurrent_queue<idgs::actor::ActorMessagePtr> >());
  }
  queues.resize(MAX_MEMBERS);
  InnerTcpConnection::innerTcpServer = this;
}

InnerTcpServer::~InnerTcpServer() {
  function_footprint();
  stop();
}


void InnerTcpServer::init(idgs::pb::ClusterConfig* cfg) {
  this->cfg = cfg;
}

std::shared_ptr<tbb::concurrent_queue<idgs::actor::ActorMessagePtr> > InnerTcpServer::getQueue(int memberId) {
  DVLOG_FIRST_N(5, 20) << "Queue size: " << queues.size() << ", dest member: " << memberId;
  if(memberId < 0) {
    return std::shared_ptr<tbb::concurrent_queue<idgs::actor::ActorMessagePtr> >();
  }
  size_t size;
  {
//    tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
    size = queues.size();
  }

  if(unlikely(memberId >= size)) {
//    tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
    size = queues.size();

    for (int i = 0; i < (memberId - size + 1); ++i) {
      queues.push_back(std::make_shared<tbb::concurrent_queue<idgs::actor::ActorMessagePtr> >());
    }
  }
  return queues[memberId];
}

bool InnerTcpServer::setConnection(uint32_t memberId, std::shared_ptr<InnerTcpConnection> conn) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  if(connections.size() <= memberId) {
    // new connection
    connections.resize(memberId + 1);
    connections[memberId] = conn;
    return true;
  } else {
    connections[memberId] = conn;
    return true;
  }
}

bool InnerTcpServer::resetConnectionIfEq(uint32_t memberId, std::shared_ptr<InnerTcpConnection> conn) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  if(unlikely(connections.size() <= memberId)) {
    connections.resize(memberId + 1);
    return true;
  }
  if (connections[memberId] && connections[memberId] == conn) {
    connections[memberId].reset();
    return true;
  }
  return false;
}

std::shared_ptr<InnerTcpConnection>& InnerTcpServer::getConnection(uint32_t memberId) {
  size_t size;
  {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
    size = connections.size();
  }
  if (unlikely(size <= memberId)) {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
    size = connections.size(); // double check
    if (likely(size <= memberId)) {
      connections.resize(memberId + 1);
    }
  }


  return connections[memberId];
}

void InnerTcpServer::connect(int peerId) {
  auto memberMgr = idgs_application()->getMemberManager();
  int32_t localMemberId = memberMgr->getLocalMemberId();

  if (peerId > localMemberId) {
    return;
  }

  getConnection(peerId);

  // connect if necessary
  if(unlikely(!connections[peerId])) {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
    if(likely(!connections[peerId])) {
      connections[peerId] = std::make_shared<InnerTcpConnection>(io_service);
    }
  }

  connections[peerId]->connect(peerId);
}


int InnerTcpServer::start() {
  auto inner_addr = cfg->member().inner_address();
  auto af = inner_addr.af();
  auto address = inner_addr.host();
  auto port = inner_addr.port();
  do{
    try {
      if(af == idgs::pb::EndPoint_AddressFamily_PAF_INET || af == idgs::pb::EndPoint_AddressFamily_PAF_INET6) {
        acceptor = new asio::ip::tcp::acceptor(io_service, asio::ip::tcp::endpoint(asio::ip::address::from_string(address), port));
      } else {
        LOG(FATAL) << "Unsupported network family " << idgs::pb::EndPoint_AddressFamily_Name(af);
        return 1;
      }
      accept();
    } catch (asio::system_error& error) {
      LOG(ERROR) << "Failed to start inner tcp server at port " << port << ", caused by " << error.what();
      if(error.code().value() == EADDRINUSE) {
        ++port;
        continue;
      }
      return 1;
    }
    break;
  } while(1);
  cfg->mutable_member()->mutable_inner_address()->set_port(port);
  DVLOG(0) << "Inner TCP server has started at address: " << address << ", port: " << port;
  return 0;
}


int InnerTcpServer::stop() {
  function_footprint();
  for (auto& q : queues) {
    q->clear();
  }
  if(acceptor) {
    auto bak = acceptor;
    acceptor = NULL;
    if(bak->is_open()) {
      bak->close();
    }
    delete bak;
  }

  for(auto& c: connections) {
    if(c) {
//      c->terminate();
      c.reset();
    }
  }
  InnerTcpConnection::innerTcpServer = NULL;
  connections.clear();
  return 0;
}

void InnerTcpServer::accept() {
  function_footprint();
  try {
    if(!acceptor || !acceptor->is_open()) {
      return;
    }
    std::shared_ptr<InnerTcpConnection> conn = std::make_shared<InnerTcpConnection>(io_service);
    if(!acceptor || !acceptor->is_open()) {
      return;
    }
    acceptor->async_accept(conn->getSocket(),
        [conn, this] (const asio::error_code& error) {
          this->handle_accept(conn, error);
    });
  } catch (std::exception &e) {
    LOG(ERROR) << "Get exception when accept: " << e.what();
  } catch (...) {
    catchUnknownException();
  }

}

void InnerTcpServer::handle_accept(std::shared_ptr<InnerTcpConnection> conn, const asio::error_code& error) {
  // accept next
  accept();

  if (error) {
    LOG_IF(ERROR, error.value() != asio::error::operation_aborted) << "accept inner TCP connection error: " << error << "(" << error.message() << ")";
  } else {
    if (conn->accept()) {
    }
  }
}

int32_t InnerTcpServer::sendMessage(idgs::actor::ActorMessagePtr& msg) {
//  DVLOG(2) << "Inner TCP server send actor message: " << msg->toString();

  int32_t memberId = msg->getDestMemberId();
  if(memberId < 0) {
    LOG(ERROR) << "Invalid member ID: " << memberId;
    return RC_ERROR;
  }
  auto q = getQueue(memberId);

#if defined(NDEBUG)
  msg->freePbMemory();
#endif

  q->push(msg);

  std::shared_ptr<InnerTcpConnection> conn = getConnection(memberId);

  if(conn) {
    conn->sendMessage(msg);
  }
  return 0;
}

std::string InnerTcpServer::toString() {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);

  std::stringstream ss;
  for (std::shared_ptr<InnerTcpConnection> c : connections) {
    if(c) {
      ss << c->toString() << std::endl;
    }
  }

  return ss.str();
}


} // namespace net 
} // namespace idgs 
