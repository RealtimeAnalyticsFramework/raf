
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/pb/cluster_config.pb.h"
#include "idgs/idgslogging.h"

namespace idgs {

namespace cluster {

class PartitionWrapper {

private:

  idgs::pb::Partition partition;

  uint8_t backup_nodes;

  void init();

public:

  PartitionWrapper();

  PartitionWrapper(uint8_t bakupNodes);

  ~PartitionWrapper() {

  }
  uint8_t getMemberCount() const {
    return partition.cells_size();
  }

  uint8_t getBackupNodes() const {
    return backup_nodes;
  }

  /// copy constructor, called by containers.
  PartitionWrapper(const PartitionWrapper& other) = default;

  /// copy assignment, called by containers.
  PartitionWrapper& operator =(const PartitionWrapper& other) = default;

  void setMemberId(int32_t index, int32_t memberId) {
    partition.mutable_cells(index)->set_memberid(memberId);
  }

  void setState(int pos, bool state) {
    partition.mutable_cells(pos)->set_state(state);
  }

  bool getState(int pos) const {
    return partition.cells(pos).state();
  }

  const idgs::pb::Partition& getPartition() const {
    return partition;
  }

  bool isPrimaryReady() const {
    return getState(0);
  }

  uint32_t getPrimaryMemberId() const {
    return getMemberId(0);
  }

  int32_t getMemberId(uint32_t index) const {
    return partition.cells(index).memberid();
  }

  void setPartition(const idgs::pb::Partition& partition) {
    this->partition = partition;
  }

  friend std::ostream& operator <<(std::ostream& os, const PartitionWrapper& p);

  std::string toString() const;

};
// end class PartitionWrapper
}// end namespace cluster
} // end namespace idgs
