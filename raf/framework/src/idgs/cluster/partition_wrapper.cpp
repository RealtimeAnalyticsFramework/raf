
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/cluster/partition_wrapper.h"

namespace idgs {
namespace cluster {

PartitionWrapper::PartitionWrapper() {
}

PartitionWrapper::PartitionWrapper(int replicas) {
  init(replicas);
}

void PartitionWrapper::init(int replicas) {
  for(int i = 0; i < replicas; ++i) {
    auto cell = partition.add_cells();
    cell->set_member_id(-1);
    cell->set_state(idgs::pb::PS_INIT);
  }
}

int32_t PartitionWrapper::getPrimaryMemberId() const {
  for (int32_t i = 0; i < partition.cells_size(); ++ i) {
    if (partition.cells(i).state() >= idgs::pb::PS_READY) {
      return partition.cells(i).member_id();
    }
  }

  return -1;
}

pb::PartitionState PartitionWrapper::getMemberState(int32_t memberId) const {
  for (int32_t i = 0; i < partition.cells_size(); ++ i) {
    if (partition.cells(i).member_id() == memberId) {
      return partition.cells(i).state();
    }
  }

  return pb::PS_INIT;
}

std::ostream& operator <<(std::ostream& os, const PartitionWrapper& pw) {
  return os << pw.toString();
}

std::string PartitionWrapper::toString() const {
  return partition.DebugString();
}

} // end namespace cluster
} // end namespace idgs

