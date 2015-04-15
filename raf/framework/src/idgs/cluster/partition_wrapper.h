
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/pb/cluster_event.pb.h"
#include "idgs/idgslogging.h"

namespace idgs {
namespace cluster {

class PartitionWrapper {
public:
  PartitionWrapper();
  PartitionWrapper(int replicas);
  ~PartitionWrapper() {
  }

public:
  uint8_t getReplicaCount() const {
    return partition.cells_size();
  }

  void setMemberId(int32_t index, int32_t memberId) {
    partition.mutable_cells(index)->set_member_id(memberId);
  }

  void setState(int pos, const pb::PartitionState& state) {
    partition.mutable_cells(pos)->set_state(state);
  }

  pb::PartitionState getState(int pos) const {
    return partition.cells(pos).state();
  }

  pb::PartitionState getMemberState(int32_t memberId) const;

  const idgs::pb::Partition& getPartition() const {
    return partition;
  }

  int32_t getPrimaryMemberId() const;

  int32_t getMemberId(uint32_t index) const {
    return partition.cells(index).member_id();
  }

  void setPartition(const idgs::pb::Partition& partition) {
    this->partition = partition;
  }

  friend std::ostream& operator <<(std::ostream& os, const PartitionWrapper& p);
  std::string toString() const;

private:
  idgs::pb::Partition partition;
  void init(int replicas);
}; // end class PartitionWrapper
} // end namespace cluster
} // end namespace idgs
