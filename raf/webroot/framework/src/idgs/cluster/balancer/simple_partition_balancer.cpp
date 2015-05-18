/*
 Copyright (c) <2012>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#include "simple_partition_balancer.h"
#include "balancer_util.h"

#ifndef REBALANCE_DEBUG
#define REBALANCE_DEBUG 0
#endif

namespace idgs {
namespace cluster {

SimplePartitionBalancer::SimplePartitionBalancer() : old_partitions(NULL), members(NULL), member(NULL), total_avaialbe_member_count(0) {
}

SimplePartitionBalancer::~SimplePartitionBalancer() {
}



/**
 * Balance partition table when member join or leave
 * @param members membership table
 * @param partitions partition table
 * @param mid join/leave member
 * @param evt created delta partition event
 */
int SimplePartitionBalancer::balance(MembershipTable& members, PartitionTable& partitions, int mid, pb::DeltaPartitionEvent& evt) {
  this->members = &members;
  this->old_partitions = &partitions;
  this->new_partitions = partitions;
  this->member = &members[mid];

  int replica_count = config->max_replica_count();
  int part_count    = config->partition_count();

  /// prepare
  total_avaialbe_member_count = BalancerUtil::calcualtePartitionCount(part_count, replica_count, new_partitions, members);

  int ret;
  /// rebalance
  if (total_avaialbe_member_count <= replica_count) {
//    fullPermutation();
    ret = simple_balance();
  } else if (member->getState() == idgs::pb::MS_INACTIVE) {
    ret = balanceLeave();
  } else {
    ret = balanceJoin();
  }
#if REBALANCE_DEBUG != 0
  /// don't adjust partition table for debug
  if (ret == 0) {
    adjust();
  }
#else
  adjust();
#endif

  /// generate partition delta list
  BalancerUtil::generatePartitionDeltaList(*old_partitions, new_partitions, evt);
  return ret;
}


#define MAX_PRIORITY std::max(10, total_part_count / 3)

///
/// a very simple rebalancer, which just re-construct the whole partition table.
///
int SimplePartitionBalancer::simple_balance() {
  VLOG(1) << __FUNCTION__ << " enter";
  int replica_count = config->max_replica_count();
  int total_part_count = config->partition_count();

  int member_size = members->size();
  int max_replicas = std::min(total_avaialbe_member_count, replica_count);

  int leave_mid = -1;
  if (!member->isAvailable()) {
    leave_mid = member->getId();
  }

  if (total_avaialbe_member_count < replica_count) {
    for (int p = 0; p < total_part_count; ++p) { // partition
      auto& part = new_partitions[p];
      for (int r = 0; r < max_replicas; ++r) { // replica
        auto mid = part.getMemberId(r);
        if (mid < 0 || mid == leave_mid) {
          for (int r2 = max_replicas; r2 < replica_count; ++r2) {
            auto mid2 = part.getMemberId(r2);
            if (mid2 >= 0 && mid != leave_mid) {
              swapMember(p, r, r2);
              break;
            }
          }
        }
      } // replica
      for (int r2 = max_replicas; r2 < replica_count; ++r2) {
        setMember(p, r2, -1);
      }
    } // partition
  }

  int member_index = 0;
  for (int r = 0; r < max_replicas; ++r) { // replica
    for (int p = 0; p < total_part_count; ++p) { // partition
      auto mid = new_partitions[p].getMemberId(r);
      auto& current_member = (*members)[mid];
      if (mid >=0 && mid != leave_mid && current_member.getPartitionCount(r) <= current_member.getExpectPartCount()) {
        continue;
      }
      bool success = false;
      // member_index = (std::abs(rand())) % member_size;
      for (int priority = -4; priority <= MAX_PRIORITY; ++priority) { // priority
        for (int k = 0; k < member_size; ++k) { // member
          auto& m = (*members)[member_index];
          if ((++member_index) >= member_size) {
            member_index = 0;
          }
          auto new_mid = m.getId();
//          LOG(INFO) << "new_mid: " << new_mid;
          if (m.isAvailable()
              && (((int)m.getPartitionCount(r))) < (((int)m.getExpectPartCount()) + priority)
              && (((int)m.getTotalPartitionCount())) < (int)((m.getExpectPartCount() * max_replicas + priority * (1 + (max_replicas - 1) / 2)))
              && checkMember(p, r, new_mid)) {
            setMember(p, r, new_mid);
            success = true;
            break;
          }
        } // loop member
        if (success) {
          break;
        }
      } // loop priority
      if (!success && (mid < 0 || mid == leave_mid)) {
        LOG(ERROR) << "Failed to find a valid member, partition: " << p << ", replica: " << r << ", leave_mid: " << leave_mid << ", available members: " << total_avaialbe_member_count << ", total partition: " << total_part_count;
        if (mid == leave_mid) {
          setMember(p, r, -1);
        }
#if REBALANCE_DEBUG != 0
        return 1;
#endif
      }
    } // loop partition
  } // loop replica
  return 0;
}

///
/// rebalance when member leave
///
int SimplePartitionBalancer::balanceLeave() {
  VLOG(1) << __FUNCTION__ << " enter";
  int total_part_count = config->partition_count();

  int member_size = members->size();
  int leave_mid = member->getId();

  int max_replicas = std::min(total_avaialbe_member_count, (int)config->max_replica_count());
  int member_index = 0;
  for (int r = 0; r < max_replicas; ++r) { // replica
    for (int p = 0; p < total_part_count; ++p) { // partition
      // auto& part = new_partitions[p];
      int mid = new_partitions[p].getMemberId(r);
      if (mid >=0 && mid != leave_mid) {
        continue;
      }
      bool success = false;
      member_index = (std::abs(rand())) % member_size;
      for (int priority = -4; priority <= MAX_PRIORITY; ++priority) { // priority
        for (int k = 0; k < member_size; ++k) { // member
          auto& m = (*members)[(member_index++) % member_size];
          if (m.isAvailable()
              && (((int)m.getPartitionCount(r))) < (((int)m.getExpectPartCount()) + priority)
              && (((int)m.getTotalPartitionCount())) < (int)((m.getExpectPartCount() * max_replicas + priority * (1 + (max_replicas - 1) / 2)))
              && checkMember(p, r, m.getId())) {
            setMember(p, r, m.getId());
            success = true;
            break;
          }
        } // loop member
        if (success) {
          break;
        }
      } // loop priority
      if (!success && (mid < 0 || mid == leave_mid)) {
        LOG(ERROR) << "Failed to find a valid member, partition: " << p << ", replica: " << r << ", leave_mid: " << leave_mid << ", available members: " << total_avaialbe_member_count << ", total partition: " << total_part_count;
        if (mid == leave_mid) {
          setMember(p, r, -1);
        }
#if REBALANCE_DEBUG != 0
        return 1;
#endif
      }
    } // loop partition
  } // loop replica
  return 0;
}

///
/// rebalance when member leave
///
int SimplePartitionBalancer::balanceJoin() {
  VLOG(1) << __FUNCTION__ << " enter";
  int total_part_count = config->partition_count();

  int join_mid = member->getId();

  int max_replicas = std::min(total_avaialbe_member_count, (int)config->max_replica_count());
  int part_index = 0;
  for (int r = 0; r < max_replicas; ++r) { // replica
    bool replica_ok = false;
    for (int state = idgs::pb::PS_INIT; state <= idgs::pb::PS_SOURCE; ++state) { // state
      for (int priority = 4; priority >= -3; --priority) { // priority
        for (int p = 0; p < total_part_count; ++p) { // partition
          auto current_part_index = part_index;
          if ((++part_index) >= total_part_count) {
            part_index = 0;;
          }
  //        VLOG(1) << "replica: " << r << ", part: " << current_part_index << ", priority: " << priority;
          auto mid = new_partitions[current_part_index].getMemberId(r);
          if (mid == join_mid) {
            continue;
          }
          auto& m = (*members)[mid];
          if ((mid < 0 || (
                (int)(new_partitions[current_part_index].getState(r)) <= state
                && (int)m.getPartitionCount(r) > (((int)m.getExpectPartCount()) + priority)
                && (int)m.getTotalPartitionCount() > (int)(m.getExpectPartCount() * max_replicas + priority * (1 + (max_replicas - 1) / 2)))
                )
              && checkMember(current_part_index, r, join_mid)) {
            setMember(current_part_index, r, join_mid);
            if (member->getPartitionCount(r) >= (member->getExpectPartCount() - 1)) {
              // next replica
              replica_ok = true;
              break;
            }
          }
        } // loop partition
        if (replica_ok) {
          break;
        }
      } // loop priority
      if (replica_ok) {
        break;
      }
    } // state
  } // loop replica
  return 0;
}

///
/// adjust partition table: swap replicas if necessary
///
void SimplePartitionBalancer::adjust() {
  int replica_count = config->max_replica_count();
  int total_part_count = config->partition_count();

  int max_replicas = std::min(total_avaialbe_member_count, replica_count);

  for (int r = 0; r < max_replicas; ++r) { // replica
    for (int p = 0; p < total_part_count; ++p) { // partition
      auto& part = new_partitions[p];
      int mid = part.getMemberId(r);
      if (mid < 0) {
        for (int r2 = r + 1; r2 < max_replicas; ++r2) {
          if (part.getMemberId(r2) < 0) {
            continue;
          }
          swapMember(p, r, r2);
          break;
        } // swap
      } else {
        // ==================================================================================
        // try to swap position of replica
        auto& member1 = (*members)[mid];
        for (int r2 = r + 1; r2 < max_replicas; ++r2) {
          if (part.getMemberId(r2) < 0) {
            continue;
          }
          auto& member2 = (*members)[part.getMemberId(r2)];
          int score = (member1.getPartitionCount(r) - member1.getPartitionCount(r2)) + (member2.getPartitionCount(r2) - member2.getPartitionCount(r));
          if (score >= 2) {
            VLOG(3) << "swap score: " << score << ", part: " << p << ", replica: " << r << "," << r2 << ", member: " << member1.getId() << "," << member2.getId();
            // LOG(INFO) << "swapMember(" << p << ", " << r << ", " << r2 << ")";
            swapMember(p, r, r2);
            break;
          }
        } // swap
      }
    } // loop partition
  } // loop replica
}


/// check whether partitions[part][pos] can be set to mid
bool SimplePartitionBalancer::checkMember (int part, int pos, int mid) {
  if (mid < 0) {
    return true;
  }
  auto & p = this->new_partitions[part];
  int replica_count = config->max_replica_count();
  int max_replicas = std::min(total_avaialbe_member_count, replica_count);

  // check whether the mid exist in the partition
  for (int i = 0; i < max_replicas; ++i) {
    if (p.getMemberId(i) == mid) {
      return false;
    }
  }
  return true;
}

/// set partitions[part][pos] to mid, move the original mid if possible
/// update actual part count in member table
void SimplePartitionBalancer::setMember (int part, int pos, int mid) {
  auto & p = this->new_partitions[part];
  int replica_count = config->max_replica_count();
  int max_replicas = std::min(total_avaialbe_member_count, replica_count);

  int old_mid = p.getMemberId(pos);
  auto old_ps = p.getState(pos);

  p.setMemberId(pos, mid);
  p.setState(pos, idgs::pb::PS_INIT);
  // update member table
  if (mid >= 0) {
    (*members)[mid].increasePartitionCount(pos);
  }

  if (old_mid >= 0) {
    // update member table
    (*members)[old_mid].decreasePartitionCount(pos);

    // move old mid to another low-priority replica
    if ((*members)[old_mid].isAvailable()) {
      for (int i = pos + 1; i < max_replicas; ++i) {
        if (p.getMemberId(i) < 0) {
          p.setMemberId(i, old_mid);
          p.setState(i, old_ps);
          // update member table
          (*members)[old_mid].increasePartitionCount(i);
          break;
        }
      }
    }
  }

}

/// swap mid between pos1 and pos2
/// update actual part count in member table
void SimplePartitionBalancer::swapMember (int part, int pos1, int pos2) {
  auto & p = this->new_partitions[part];
  int mid1 = p.getMemberId(pos1);
  auto ps1 = p.getState(pos1);
  int mid2 = p.getMemberId(pos2);
  auto ps2 = p.getState(pos2);
  p.setMemberId(pos2, mid1);
  p.setMemberId(pos1, mid2);
  p.setState(pos1, ps2);
  p.setState(pos2, ps1);

  // update member table
  if (mid1 >= 0) {
    (*members)[mid1].decreasePartitionCount(pos1);
    (*members)[mid1].increasePartitionCount(pos2);
  }
  if (mid2 >= 0) {
    (*members)[mid2].decreasePartitionCount(pos2);
    (*members)[mid2].increasePartitionCount(pos1);
  }
}


} // namespace cluster
} // namespace idgs
