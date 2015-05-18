/*
 Copyright (c) <2012>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#pragma once
#include <memory>
#include "partition_balancer.h"

namespace idgs {
namespace cluster {

struct MigrateAction {
  int partition_id;
  int member_id;
  enum {
    NEW,
    DEL
  } command;
};
inline std::ostream& operator << (std::ostream& os, const MigrateAction& ma) {
  os << "part: " << ma.partition_id << ", mid:" << ma.member_id << ", cmd: " << (ma.command == MigrateAction::NEW? "NEW" : "DEL");
  return os;
}

class BalancerUtil {
public:
  BalancerUtil();
  ~BalancerUtil();

public:
  /// factory method to create partition balancer.
  static std::shared_ptr<PartitionBalancer> createBalancer();


  /// @param partitions [in] new partition table
  /// @param evt [in] result of rebalance
  /// @param actions [out] migration actions
  static void generateMigrateActionList(const std::vector<PartitionWrapper>& partitions, const idgs::pb::DeltaPartitionEvent& evt, std::vector<MigrateAction>& actions);

  /// calculate actual partition count and expected partition count for each member.
  /// @param part_count [in] cluster partition count, should be a prime number.
  /// @param replica_count [in] cluster max replica count, usually 3.
  /// @param partitions [in] the partition table.
  /// @param members [out] member table.
  /// @return total available member count
  static int calcualtePartitionCount (int part_count, int replica_count, const std::vector<PartitionWrapper>& partitions, std::vector<MemberWrapper>& members);

  /// generate delta partition event list from old and new partition table
  /// @param old_partitions [in] old partition table
  /// @param new_partitions [in] new partition table
  /// @param evt [out] output delta partition list
  static void generatePartitionDeltaList (const std::vector<PartitionWrapper>& old_partitions, const std::vector<PartitionWrapper>& new_partitions, idgs::pb::DeltaPartitionEvent& evt);
};

} // namespace cluster
} // namespace idgs
