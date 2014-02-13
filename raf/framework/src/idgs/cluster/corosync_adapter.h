
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "member_wrapper.h"
#include "idgs/actor/actor_message.h"
#include "corosync_cpg.h"
#include <thread>
#include <mutex>

namespace idgs {

namespace cluster {

class CorosyncClusterAdapter: private CorosyncCpg::CorosyncCpgHandler {

public:

  CorosyncClusterAdapter();

  ~CorosyncClusterAdapter();

  /**
   *  Initialize corosync cluster engine
   */
  ResultCode init(idgs::pb::ClusterConfig* cfg);

  /**
   * Start a member
   */
  ResultCode start();

  /**
   *  Member quit from cluster engine
   */
  void stop();

  CorosyncCpg& getCpg() {
    return cpg;
  }

  /**
   *  Multicast actor message
   *  @param actor_msg_ptr sent actor message
   */
  idgs::ResultCode multicastMessage(const std::shared_ptr<idgs::actor::ActorMessage>& actor_msg_ptr);

  /**
   * Thread call back function where cpg_disptch is always called by this thread
   */
  void dispatch();

private:
  std::mutex mcastLock;

  /**
   * corosync cpg service
   */
  CorosyncCpg cpg;

  /**
   * Cluster config pointer
   */
  idgs::pb::ClusterConfig* config;

  /**
   * initialized flag
   */
  bool initialized;

  /**
   *  dispatch thread flag
   */
  bool dispatching;

  ::std::thread *corosyncThread;

  /**
   *  Corosync rpc message call back function, occur when message is delivered and function cpg_dispatch is called
   */
  void deliver(cpg_handle_t handle, const struct cpg_name *group, uint32_t nodeid, uint32_t pid, void* msg,
      int msg_len);

  /**
   *  Corosync rpc message call back function, occur when function cpg_join or cpg_leave is called,
   *  this function only handle
   */
  void configChange(cpg_handle_t handle, const struct cpg_name *group, const struct cpg_address *members, int nMembers,
      const struct cpg_address *left, int nLeft, const struct cpg_address *joined, int nJoined);

  /**
   *  Process actor message
   *  @param actor_msg_ptr received actor message
   */
  void processMessage(std::shared_ptr<idgs::actor::ActorMessage>& actor_msg_ptr);
};
}
}
