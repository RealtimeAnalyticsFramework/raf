/*
 Copyright (c) <2012>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#pragma once

#include "idgs/actor/actor_message.h"
#include "idgs/cluster/member_wrapper.h"


namespace idgs {
namespace cluster {

class StandaloneClusterAdapter {
public:
  StandaloneClusterAdapter();
  virtual ~StandaloneClusterAdapter();
public:
  // ===================================================================
  // public interfaces
  // ===================================================================

  ///
  /// Initialize corosync cluster engine
  ///
  ResultCode init(idgs::pb::ClusterConfig* cfg);

  ///
  /// Start a member
  ///
  ResultCode start();

  ///
  /// Member quit from cluster engine
  ///
  void stop();


  ///
  /// Multicast actor message
  /// @param actor_msg_ptr sent actor message
  ///
  idgs::ResultCode multicastMessage(const std::shared_ptr<idgs::actor::ActorMessage>& actor_msg_ptr);
};

} // namespace cluster
} // namespace idgs
