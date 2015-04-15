/*
 Copyright (c) <2012>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#include <math.h>
#include "balancer_util.h"
#include "simple_partition_balancer.h"

namespace idgs {
namespace cluster {

BalancerUtil::BalancerUtil() {
}

BalancerUtil::~BalancerUtil() {
}

/// factory method to create partition balancer.
std::shared_ptr<PartitionBalancer> BalancerUtil::createBalancer() {
  return std::make_shared<SimplePartitionBalancer>();
}


/// @param partitions [in] new partition table
/// @param evt [in] result of rebalance
/// @param actions [out] migration actions
void BalancerUtil::generateMigrateActionList(
    const std::vector<PartitionWrapper>& partitions,
    const idgs::pb::DeltaPartitionEvent& evt,
    std::vector<MigrateAction>& actions) {
  int pos1 = 0;
  int pos2 = 0;
  while (pos1 < evt.items_size()) {
    int pid = evt.items(pos1).part_id();
    auto& part = partitions[pid];

    for (pos2 = pos1; pos2 < evt.items_size(); ++pos2) {
      // find the first item with different partition id
      if (evt.items(pos2).part_id() != pid) {
        break;
      }

      // find deleted members
      // old member id isn't in partition table any more.
      auto& item = evt.items(pos2);
      auto mid = item.old_mid();
      if (mid < 0) {
        continue;
      }
      bool found = false;
      for (int i = 0; i < part.getReplicaCount(); ++i) {
        if (part.getMemberId(i) == mid) {
          found = true;
          break;
        }
      }
      if (!found) {
        MigrateAction ma;
        ma.partition_id = pid;
        ma.member_id = mid;
        ma.command = MigrateAction::DEL;
        actions.push_back(ma);
        VLOG(2) << ma << std::endl;
      }
    }

    // find new members
    // member is in partition table, and in new_member, and not old_member
    for (int i = 0; i < part.getReplicaCount(); ++i) {
      bool in_new = false;
      bool in_old = false;
      int mid = part.getMemberId(i);
      if (mid < 0) {
        continue;
      }
      for (int j = pos1; j < pos2; ++j) {
        if (evt.items(j).old_mid() == mid) {
          in_old = true;
        }
        if (evt.items(j).new_mid() == mid) {
          in_new = true;
        }
      }
      if (in_new && !in_old) {
        MigrateAction ma;
        ma.partition_id = pid;
        ma.member_id = mid;
        ma.command = MigrateAction::NEW;
        actions.push_back(ma);
        VLOG(2) << ma << std::endl;
      }
    }


    // next partition
    pos1 = pos2;
  }
}

/// calculate actual partition count and expected partition count for each member.
/// @param part_count [in] cluster partition count, should be a prime number.
/// @param replica_count [in] cluster max replica count, usually 3.
/// @param partitions [in] the partition table.
/// @param members [out] member table.
/// @return total available member count
int BalancerUtil::calcualtePartitionCount (int part_count, int replica_count, const std::vector<PartitionWrapper>& partitions, std::vector<MemberWrapper>& members) {
  float total_weight = 0;
  int total_avaialbe_member_count = 0;
  for (auto& m : members) {
    if (m.isAvailable()) {
      total_weight += m.getMember().weight();
      ++total_avaialbe_member_count;
    }
  }

  // weight per partition
  float weight_per_part = total_weight / part_count;
  VLOG(4) << "Total weight: " << total_weight << ", weight_per_part: " << weight_per_part;

  // calculate expected part count for each member
  for (auto& m : members) {
    if (m.isAvailable()) {
      m.setExpectPartCount(ceilf(m.getMember().weight() / weight_per_part));
      VLOG(4) << "Member: " << m.getId() << ", expect part count: " << ceilf(m.getMember().weight() / weight_per_part) << ", weight:" << m.getMember().weight();
    } else {
      m.setExpectPartCount(0);
    }
    // reset actual partition count
    m.resetPartitionCount();
  }

  // calculate actual part count for each member
  for (int i = 0; i < part_count; ++i) {
    auto & p = partitions[i];
    for (int r = 0; r < replica_count; ++ r) {
      int m = p.getMemberId(r);
      if (m >= 0) {
        members[m].increasePartitionCount(r); //
      }
    }
  }
  return total_avaialbe_member_count;
}

/// generate delta partition event list from old and new partition table
/// @param old_partitions [in] old partition table
/// @param new_partitions [in] new partition table
/// @param evt [out] output delta partition list
void BalancerUtil::generatePartitionDeltaList (const std::vector<PartitionWrapper>& old_partitions, const std::vector<PartitionWrapper>& new_partitions, idgs::pb::DeltaPartitionEvent& evt) {
  int replica_count = old_partitions[0].getReplicaCount();
  int total_part_count = old_partitions.size();
  for (int i = 0; i < total_part_count; ++i) {
    // loop all partition
    auto& old_part = old_partitions[i];
    auto& new_part = new_partitions[i];
    for (int j = 0; j < replica_count; ++j) {
      if (old_part.getMemberId(j) != new_part.getMemberId(j)) {
        auto item = evt.add_items();
        item->set_part_id(i);
        item->set_position(j);
        item->set_old_mid(old_part.getMemberId(j));
        item->set_new_mid(new_part.getMemberId(j));
        if(new_part.getState(j) != old_part.getState(j)) {
          item->set_state(new_part.getState(j));
        }
      }
    } // loop replica
  } // loop partition
}


} // namespace cluster
} // namespace idgs
