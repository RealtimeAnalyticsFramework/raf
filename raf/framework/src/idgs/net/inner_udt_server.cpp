/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#if defined(WITH_UDT)

#include "inner_udt_server.h"




namespace idgs {
namespace net {

#define MAX_MEMBERS 100

InnerUdtServer::InnerUdtServer(NetworkModelAsio* net) : network(net), cfg(NULL) {
  connections.resize(MAX_MEMBERS);
  for(int i = 0; i < MAX_MEMBERS; ++i) {
    queues.push_back(std::make_shared<tbb::concurrent_queue<idgs::actor::ActorMessagePtr> >());
  }
  queues.resize(MAX_MEMBERS);
  InnerUdtConnection::innerUdtServer = this;
}

InnerUdtServer::~InnerUdtServer() {
  stop();
}

void InnerUdtServer::init(idgs::pb::ClusterConfig* cfg) {
  this->cfg = cfg;
}

int32_t InnerUdtServer::start() {
//  UDT::startup();
  return 0;
}

void InnerUdtServer::stop() {
//  UDT::cleanup();
//  for (auto& q : queues) {
//    q->clear();
//  }
//  for(auto& c: connections) {
//    if(c) {
//      c.reset();
//    }
//  }
//  InnerUdtConnection::innerUdtServer = NULL;
//  connections.clear();
}

bool InnerUdtServer::setConnection(uint32_t memberId, std::shared_ptr<InnerUdtConnection> conn) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  if(connections.size() <= memberId) {
    // new connection
    connections.resize(memberId + 1);
    connections[memberId] = conn;
    return true;
  } else {
    if(connections[memberId]) {
      // connection exists
      return false;
    } else {
      // connection reset
      connections[memberId] = conn;
      return true;
    }
  }
}

bool InnerUdtServer::resetConnectionIfEq(uint32_t memberId, std::shared_ptr<InnerUdtConnection> conn) {
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

std::shared_ptr<InnerUdtConnection>& InnerUdtServer::getConnection(uint32_t memberId) {
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

  // connect if necessary
  if(unlikely(!connections[memberId])) {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
    if(likely(!connections[memberId])) {
      connections[memberId] = std::make_shared<InnerUdtConnection>();
    }
  }

  return connections[memberId];
}

std::shared_ptr<tbb::concurrent_queue<idgs::actor::ActorMessagePtr>> InnerUdtServer::getQueue(int memberId) {
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

void InnerUdtServer::accept() {
  std::shared_ptr<InnerUdtConnection> conn = std::make_shared<InnerUdtConnection>();
  std::thread th([this, conn](){
    try {
      set_thread_name("idgs_io");
      this->handle_accept(conn);
    } catch (std::exception &e) {
      LOG(ERROR) << "Get exception in IO thread: " << e.what();
    } catch (...) {
      catchUnknownException();
    }
  });
}

void InnerUdtServer::handle_accept(std::shared_ptr<InnerUdtConnection> conn) {
  accept();
  conn->accept();
}

int32_t InnerUdtServer::sendMessage(idgs::actor::ActorMessagePtr& msg) {
  int32_t memberId = msg->getDestMemberId();
  if(memberId < 0) {
    LOG(ERROR) << "Invalid member ID: " << memberId;
    return RC_ERROR;
  }
  auto q = getQueue(memberId);

  msg->freePbMemory();

  q->push(msg);

  std::shared_ptr<InnerUdtConnection> conn = getConnection(memberId);
  if(conn) {
    conn->sendMessage(msg);
  }

  return 0;
}

std::string InnerUdtServer::toString() {
  return "InnerUdtServer";
}

} // namespace net
} // namespace idgs

#endif // defined(WITH_UDT)
