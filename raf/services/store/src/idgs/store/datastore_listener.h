
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/cluster/partition_listener.h"
#include "idgs/cluster/member_event_listener.h"

namespace idgs {
namespace store {

// When partition changed and rebalance done, cluster get a event from cluster
class PartitionChangeListener: public idgs::cluster::PartitionListener {
public:

  PartitionChangeListener();
  virtual ~PartitionChangeListener();

  /// @brief  When partition changed and rebalance done, get the partition info from cluster and migrate data.
  /// @param  evt Event of partition changed.
  void partitionChanged(const idgs::pb::DeltaPartitionEvent& evt);
};

// When new member joined, cluster get a event from cluster
class MemberJoinedListener: public idgs::cluster::MemberEventListener {
public:
  MemberJoinedListener();
  virtual ~MemberJoinedListener();

  // When new member joined, sync data for replicated store.
  void statusChanged(const idgs::cluster::MemberWrapper& member);
};

} /* namespace store */
} /* namespace idgs */
