
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "idgs/cluster/corosync_adapter.h"


#include "idgs/application.h"
#include "idgs/net/network_statistics.h"

using namespace std;
using namespace idgs::pb;
using namespace idgs::pb;
using namespace idgs::actor;

namespace idgs {

namespace cluster {

CorosyncClusterAdapter::CorosyncClusterAdapter() :
    cpg(*this), config(NULL), initialized(false), dispatching(true), corosyncThread(NULL) {

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
  cs_error_t rs = cpg.join(member->group_name());
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
    cpg.leave();
    cpg.shutdown();
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
  }
}

void CorosyncClusterAdapter::processMessage(std::shared_ptr<ActorMessage>& actorMsg) {
  ActorFramework* af = ::idgs::util::singleton<RpcFramework>::getInstance().getActorFramework();
  Actor* actor = af->getActor(actorMsg->getDestActorId());
  if (actor) {
    actor->process(actorMsg);
  } else {
    LOG(ERROR)<< "Actor " << actorMsg->getDestActorId() << " is not found ";
  }
}

ResultCode CorosyncClusterAdapter::multicastMessage(const std::shared_ptr<ActorMessage>& msg) {
  idgs::ResultCode code = RC_SUCCESS;

  if (!msg->getRpcBuffer()) {
    code = msg->toRpcBuffer();
  }

  if (code != RC_SUCCESS) {
    LOG(ERROR)<< "send message error with code " << code;
    return code;
  }
  struct iovec iov;
  iov.iov_base = (void*) msg->getRpcBuffer()->getBody();
  iov.iov_len = msg->getRpcBuffer()->getBodyLength();

  static ::idgs::net::NetworkStatistics* stats =
      ::idgs::util::singleton<RpcFramework>::getInstance().getNetwork()->getNetworkStatistics();
  stats->multicastPacketSent.fetch_add(1);
  stats->multicastBytesSent.fetch_add(iov.iov_len);

  {
    // cppcheck-suppress unreadVariable
    std::lock_guard < std::mutex > lockGuard(mcastLock);

    if (CS_OK != cpg.multicast(&iov, 1)) {
      return RC_CLUSTER_ERR_MULTICAST;
    }
  }
  return RC_SUCCESS;
}

void CorosyncClusterAdapter::deliver(cpg_handle_t handle, const struct cpg_name* group_name, uint32_t nodeid,
    uint32_t pid, void* msg, int msg_len) {
  if (dispatching) {
    std::shared_ptr<RpcMessage> rpcMsg(new RpcMessage);
    bool flag = protobuf::ProtoSerdes<DEFAULT_PB_SERDES>::deserializeFromArray(static_cast<const char*>(msg), msg_len,
        rpcMsg.get());

    if (!flag) {
      LOG(ERROR)<< getErrorDescription(RC_CLUSTER_ERR_DESERIALIZE_MSG) << " from nodeId:" << nodeid << " pid:" << pid;
      LOG(ERROR) << rpcMsg->DebugString();
      return;
    }

    static ::idgs::net::NetworkStatistics* stats =
        ::idgs::util::singleton<RpcFramework>::getInstance().getNetwork()->getNetworkStatistics();
    stats->multicastPacketRecv.fetch_add(1);
    stats->multicastBytesRecv.fetch_add(msg_len);

    if (!::idgs::util::singleton<idgs::Application>::getInstance().isRunning()) {
      LOG(INFO)<< "Ignore multicast messages during shutting down.";
      return;
    }

    std::shared_ptr<ActorMessage> actor_msg_ptr(new ActorMessage(rpcMsg));
    processMessage(actor_msg_ptr);
  } else {
    LOG(INFO)<< "closed, don't accept multi-cast any more.";
  }
}

void CorosyncClusterAdapter::configChange(cpg_handle_t handle, const struct cpg_name* group_name,
    const struct cpg_address* member_list, int member_list_entries, const struct cpg_address* left_list,
    int left_list_entries, const struct cpg_address* joined_list, int joined_list_entries) {
  if (!::idgs::util::singleton<idgs::Application>::getInstance().isRunning()) {
    LOG(INFO)<< "Ignore config change during shutting down.";
    return;
  }
  DVLOG(3) << "Joined entries: " << joined_list_entries << ", Current entries: " << member_list_entries << ", Left entries: " << left_list_entries;
  if(left_list_entries > 0) { // handle member left
    const MemberWrapper* local_member = ::idgs::util::singleton<ClusterFramework>::getInstance().getMemberManager()->getLocalMember();
    if(local_member && local_member->getId() > -1) { // find out left members
      std::vector<MemberWrapper*> leftMembers;
      for (int i = 0, index = 0; i < left_list_entries; ++i) {
        auto member = ::idgs::util::singleton<ClusterFramework>::getInstance().getMemberManager()->findMember(left_list[i].nodeid, left_list[i].pid);
        if(!member || member->isInactive()) {
          continue;
        }
        leftMembers.push_back(member);
      }
      ::idgs::util::singleton<ClusterFramework>::getInstance().getMemberManager()->memberLeft(leftMembers);
    }
  }
  if(joined_list_entries > 0) { // handle new member joined
    for (int j = 0; j < joined_list_entries; ++j) {
      auto cfg_member = config->mutable_member();
      cfg_member->mutable_service()->set_leading(joined_list_entries >= member_list_entries ? j == 0 : false);
      if(cfg_member->node_id() == joined_list[j].nodeid && cfg_member->pid() == joined_list[j].pid) { // send itself to leading
        DVLOG(1) << "Joined member send itself to leading";
        MemberWrapper joined_member;
        joined_member.setMember(*cfg_member);
        ::idgs::util::singleton<ClusterFramework>::getInstance().getMemberManager()->memberJoined(joined_member);
      }
    }
  }
}

/// @todo specified exception should be caught.
void CorosyncClusterAdapter::dispatch() {
  set_thread_name("idgs_corosync");

  cs_error_t rs;
  CorosyncClusterAdapter* corosync = this; // (CorosyncClusterAdapter*)args;
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
    if (select(cpgfd + 1, &readfds, NULL, NULL, &timeout) > 0) {
      try {
        if (FD_ISSET(cpgfd, &readfds)) {
          DVLOG(2) << "enter Corosync select";
          cpgfd = corosync->getCpg().getFd();
          if (cpgfd < 0) {
            break;
          }
          rs = corosync->getCpg().dispatchAll();
          DVLOG(2) << "Corosync dispatch messages done.";
          if (rs != CS_OK) {
            LOG(ERROR)<< CorosyncCpg::errorStr(rs, getErrorDescription(RC_CLUSTER_ERR_CPG_DISPATCH));
          }
          if (rs == CS_ERR_LIBRARY) {
            LOG(ERROR)<< "Corosync library error.";
            idgs::util::singleton<idgs::Application>::getInstance().shutdown();
            break;
          }
        }
      } catch (exception& e) {
        DVLOG(1) << e.what();
        catchUnknownException();
      }
    } else {
      DVLOG(5) << "Corosync select 0";
    }
  }
  LOG(INFO)<< "Corosync dispatch thread is terminated.";
//      return (void*) ((((0))));
}}
}
