
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(WITH_COROSYNC)

#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/cluster/adapter/corosync_cluster_adapter.h"

#include "idgs/application.h"
#include "idgs/net/network_statistics.h"

namespace idgs {
namespace cluster {

CorosyncClusterAdapter::CorosyncClusterAdapter() :
    cpg(*this), config(NULL), initialized(false), dispatching(true), corosyncThread(NULL) {
  writing.clear();

}

CorosyncClusterAdapter::~CorosyncClusterAdapter() {
  dispatching = false;
  stop();
}

ResultCode CorosyncClusterAdapter::init(idgs::pb::ClusterConfig* cfg) {
  this->config = cfg;
  if (!initialized) {
    cs_error_t rs = cpg.init();
    if (rs != CS_OK) {
      dispatching = false;
      LOG(ERROR)<< getErrorDescription(RC_CLUSTER_ERR_CPG_INIT);
      return RC_CLUSTER_ERR_CPG_INIT;
    }
    initialized = true;
  }
  return RC_SUCCESS;
}

ResultCode CorosyncClusterAdapter::start() {
  CorosyncMemberId self = cpg.self();
  auto member = config->mutable_member();
  // set member's node id and process id
  member->set_node_id(self.getNode());
  member->set_pid(self.getPid());
  // join cpg group
  cs_error_t rs = cpg.join(config->group_name());
  if (rs != CS_OK) {
    return RC_CLUSTER_ERR_JOIN;
  }
  //create a dispatcher thread
  auto f = ::std::bind(&idgs::cluster::CorosyncClusterAdapter::dispatch, this);
  std::thread* t1 = new std::thread(f);
  corosyncThread = t1;
  return RC_SUCCESS;
}

void CorosyncClusterAdapter::stop() {
  if (!initialized) {
    return;
  }
  initialized = false;
  if (dispatching) {
    dispatching = false;
    // wait for thread
    if (corosyncThread) {
      auto bak = corosyncThread;
      corosyncThread = NULL;
      int fd = getCpg().getFd();
      if (fd > 0) {
        LOG(INFO)<< "force to close FD of corosync: " << fd;
        close(fd);
      }
      DVLOG(3) << "before join corosync thread";
      if (bak->joinable()) {
        bak->join();
      }
      delete bak;
    }

    // close cpg
    getCpg().leave();
    getCpg().shutdown();
  }
}

void CorosyncClusterAdapter::processMessage(std::shared_ptr<idgs::actor::ActorMessage>& actorMsg) {
  function_footprint();
  idgs::actor::ActorManager* af = idgs_application()->getRpcFramework()->getActorManager();
  idgs::actor::Actor* actor = af->getActor(actorMsg->getDestActorId());
  if (actor) {
    actor->process(actorMsg);
  } else {
    LOG(ERROR)<< "corosync process message error, caused by actor " << actorMsg->getDestActorId() << " not found.";
  }
}

void CorosyncClusterAdapter::realMulticastMessage() {
  idgs::actor::ActorMessagePtr msg;

#if !defined(RETRY_LOCK_TIMES)
#define RETRY_LOCK_TIMES 5
#endif // !defined(RETRY_LOCK_TIMES)
  int i = 0;
  for (; i < RETRY_LOCK_TIMES; ++i) {
    if(!writing.test_and_set()) {
      break;
    }
  }
  if(i == RETRY_LOCK_TIMES) {
    return;
  }
  try {
    while (queue.try_pop(msg)) {

      struct iovec iov;
      iov.iov_base = (void*) msg->getRpcBuffer()->getBody();
      iov.iov_len = msg->getRpcBuffer()->getBodyLength();

      static ::idgs::net::NetworkStatistics* stats = idgs_application()->getRpcFramework()->getNetwork()->getNetworkStatistics();
      stats->multicastPacketSent.fetch_add(1);
      stats->multicastBytesSent.fetch_add(iov.iov_len);

      if (CS_OK != cpg.multicast(&iov, 1)) {
        LOG(ERROR) << "Failed to multicast message.";
        break;
      }
    }
  } catch (...) {
    catchUnknownException();
  }
  writing.clear();

}

ResultCode CorosyncClusterAdapter::multicastMessage(const std::shared_ptr<idgs::actor::ActorMessage>& msg) {
  function_footprint();
  idgs::ResultCode code = RC_SUCCESS;
  if (!msg->getRpcBuffer()) {
    code = msg->toRpcBuffer();
  }
  if (code != RC_SUCCESS) {
    LOG(ERROR)<< "send message error with code " << code;
    return code;
  }
  queue.push(msg);
  realMulticastMessage();

  return RC_SUCCESS;
}

void CorosyncClusterAdapter::deliver(cpg_handle_t handle, const struct cpg_name* group_name, uint32_t nodeid,
    uint32_t pid, void* msg, int msg_len) {
  function_footprint();
  static ::idgs::net::NetworkStatistics* stats =
      idgs_application()->getRpcFramework()->getNetwork()->getNetworkStatistics();
  stats->multicastPacketRecv.fetch_add(1);
  stats->multicastBytesRecv.fetch_add(msg_len);

  if(dispatching) {
//    LOG(INFO) << "multicast message: " << dumpBinaryBuffer2((const char*)msg, msg_len);
#if DEFAULT_PB_SERDES == 0  // PB_BINARY
#define SIZE_BEFORE_ACTORID 8  // PB_BINARY
#else
#define SIZE_BEFORE_ACTORID 32 // PB_JSON or PB_TEXT
#endif
    const static size_t PREFIX_SIZE = std::min((size_t)64, (std::max(AID_MEMBER.length(), AID_PARTITION.length()) + (size_t)SIZE_BEFORE_ACTORID));
    std::string prefix((const char*)msg, std::min((size_t)msg_len, PREFIX_SIZE));
    if (prefix.find(AID_MEMBER) != std::string::npos || prefix.find(AID_PARTITION) != std::string::npos) {
//      LOG(INFO) << "multicast message prefix: " << dumpBinaryBuffer(prefix);
      std::shared_ptr<idgs::pb::RpcMessage> rpcMsg = std::make_shared<idgs::pb::RpcMessage>();
      bool flag = protobuf::ProtoSerdes<DEFAULT_PB_SERDES>::deserializeFromArray(static_cast<const char*>(msg), msg_len, rpcMsg.get());
      std::shared_ptr<idgs::actor::ActorMessage> actorMsg = std::make_shared<idgs::actor::ActorMessage>(rpcMsg);


      if (!flag) {
        LOG(ERROR)<< getErrorDescription(RC_CLUSTER_ERR_DESERIALIZE_MSG);
        return;
      }

      if (!idgs_application()->isRunning()) {
        LOG(INFO)<< "ignore multicast messages during shutting down.";
        return;
      }

      processMessage(actorMsg);
    } else {
      std::shared_ptr<idgs::net::RpcBuffer> readBuffer = std::make_shared<idgs::net::RpcBuffer>();
      readBuffer->setBodyLength(msg_len);
      readBuffer->decodeHeader();
      memcpy(readBuffer->getBody(), msg, msg_len);

      std::shared_ptr<idgs::actor::ActorMessage> actorMsg = std::make_shared<idgs::actor::ActorMessage>();
      actorMsg->setRpcBuffer(readBuffer);
      actorMsg->setMessageOrietation(idgs::actor::ActorMessage::UDP_ORIENTED);

      idgs::actor::relayMessage(actorMsg);
    }
  } else {
    LOG(INFO)<< "dispatch has been closed, not deliver any message.";
  }
}

void CorosyncClusterAdapter::configChange(cpg_handle_t handle, const struct cpg_name* group_name,
    const struct cpg_address* member_list, int member_list_entries, const struct cpg_address* left_list,
    int left_list_entries, const struct cpg_address* joined_list, int joined_list_entries) {
  if (!idgs_application()->isRunning()) {
    LOG(INFO)<< "application is not running, ignore config change call back.";
    return;
  }
  DVLOG(0) << "corosync config changed callback occurs, joined entries: " << joined_list_entries
      << ", current entries: " << member_list_entries << ", left entries: " << left_list_entries;

  actor::ActorMessagePtr msg = std::make_shared<actor::ActorMessage>();
  msg->getRpcMessage();

  std::shared_ptr<google::protobuf::Message> ev = std::make_shared<idgs::pb::CpgConfigChangeEvent>();
  msg->setPayload(ev);
  msg->setOperationName(OID_CPG_CONFIG_CHANGE);
  msg->setSourceActorId(AID_MEMBER);
  msg->setDestActorId(AID_MEMBER);
  msg->setSourceMemberId(idgs::pb::ANY_MEMBER);
  msg->setDestMemberId(idgs::pb::ANY_MEMBER);

  idgs::pb::CpgConfigChangeEvent* event = dynamic_cast<idgs::pb::CpgConfigChangeEvent*>(ev.get());
  for (int i = 0; i < member_list_entries; ++i) {
    auto m = event->add_member_list();
    m->set_nodeid(member_list[i].nodeid);
    m->set_pid(member_list[i].pid);
    m->set_reason(member_list[i].reason);
  }

  for (int i = 0; i < left_list_entries; ++i) {
    auto m = event->add_left_list();
    m->set_nodeid(left_list[i].nodeid);
    m->set_pid(left_list[i].pid);
    m->set_reason(left_list[i].reason);
  }

  for (int i = 0; i < joined_list_entries; ++i) {
    auto m = event->add_joined_list();
    m->set_nodeid(joined_list[i].nodeid);
    m->set_pid(joined_list[i].pid);
    m->set_reason(joined_list[i].reason);
  }

  auto memberManager = idgs_application()->getMemberManager();
  memberManager->handleCpgConfigChange(msg);
  return;
}

void CorosyncClusterAdapter::dispatch() {
  set_thread_name("idgs_corosync");
  cs_error_t rs;
  CorosyncClusterAdapter* corosync = this;
  fd_set readfds;
  struct timeval timeout;
  while (corosync->dispatching) {
    int cpgfd = corosync->getCpg().getFd();
    if (cpgfd < 0) {
      break;
    }
    FD_ZERO(&readfds);
    FD_SET(cpgfd, &readfds);
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    if((select(cpgfd + 1, &readfds, NULL, NULL, &timeout)) > 0) {
      try {
        if (FD_ISSET(cpgfd, &readfds)) {
          DVLOG(2) << "enter corosync select";
          cpgfd = corosync->getCpg().getFd();
          if (cpgfd < 0) {
            break;
          }
          rs = corosync->getCpg().dispatchAll();
          DVLOG(2) << "corosync dispatch messages done.";
          if (rs != CS_OK) {
            LOG(ERROR)<< CorosyncCpg::errorStr(rs, getErrorDescription(RC_CLUSTER_ERR_CPG_DISPATCH));
          }
          if (rs == CS_ERR_LIBRARY) {
            LOG(ERROR)<< "corosync library error.";
            idgs_application()->shutdown();
            break;
          }
        }
      } catch (std::exception& e) {
        DVLOG(0) << "dispatch error, caused by " << e.what();
      } catch(...) {
        catchUnknownException();
      }
    } else {
      LOG_IF_EVERY_N(INFO, VLOG_IS_ON(3) ,50) << "no data to dispatch in " << timeout.tv_sec << " seconds";
    }
  }
  LOG(INFO)<< "corosync dispatch thread is terminated.";
}

} /// end namespace cluster
} /// end namespace idgs
#endif // defined(WITH_COROSYNC)
