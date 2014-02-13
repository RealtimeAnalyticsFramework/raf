
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "naive_partition_balancer.h"

using namespace idgs::pb;

namespace idgs {

namespace cluster {

static int32_t select(std::map<size_t, size_t>& map);

static int32_t select(std::map<size_t, size_t>& map, const size_t except_member_id);

//		static void displayPartitionCount(std::map<size_t, size_t>& delta_partition_count_map, std::map<size_t, size_t> & partition_count_map) {
//			VLOG(1) << "######## Delta partition count #######";
//			for(auto it = delta_partition_count_map.begin(); it != delta_partition_count_map.end(); ++it) {
//				VLOG(3) << it->first << ", " << it->second;
//			}
//			VLOG(1) << "######## Partition count #######";
//			for(auto it = partition_count_map.begin(); it != partition_count_map.end(); ++it) {
//				VLOG(3) << it->first << ", " << it->second;
//			}
//		}

static size_t getBalanceableMemberSize(std::vector<MemberWrapper>& members) {
  const size_t memberSize = members.size();
  size_t sum = 0;
  for (int i = 0; i < memberSize; ++i) {
    if (members.at(i).canBalanced()) {
      ++sum;
    }
  }
  return sum;
}

void NaivePartitionBalancer::balance(MembershipTable& members, PartitionTable& partitions, MemberWrapper& member, pb::DeltaPartitionEvent& evt) {
  if (member.isJoined() || member.isPrepared()) {
    balanceWhenJoined(members, partitions, member, evt);
  } else if (member.isLeave()) {
    balanceWhenLeft(members, partitions, member, evt);
  }
}

void NaivePartitionBalancer::balanceWhenJoined(MembershipTable& members, PartitionTable& partitions, MemberWrapper& member, pb::DeltaPartitionEvent& evt) {
  const size_t member_size = members.size();
  const size_t partition_count = partitions.size();
  const size_t balanceable_member_size = member_size == 1 ? 1 : getBalanceableMemberSize(members);
  const size_t balance_count =
      (balanceable_member_size == 1) ? partition_count : partition_count / balanceable_member_size;
  std::map<size_t, size_t> delta_partition_count_map, partition_count_map;
  size_t balance_avg_count, balance_mod_count;
  balance_avg_count = balanceable_member_size == 1 ? partition_count : balance_count / (balanceable_member_size - 1);
  balance_mod_count = balanceable_member_size == 1 ? 0 : balance_count % (balanceable_member_size - 1);
  DVLOG(1) << partition_count  << ", " << balanceable_member_size << ", " << balance_count << ", " << balance_avg_count << ", " << balance_mod_count  ;
  calculatePartitionCountWhenJoined(members, member, member_size, balance_count, balance_avg_count,
      balance_mod_count, delta_partition_count_map, partition_count_map);
  const uint32_t joined_member_id = member.getId();
  if (balanceable_member_size == 1) {
    for (size_t i = 0; i < partition_count; ++i) {
      DeltaPartitionItemEvent* itemEvt = evt.add_items();
      itemEvt->set_needmigrate(false);
      itemEvt->set_partitionid(i);
      itemEvt->set_position(0);
      itemEvt->set_currmemberid(-1);
      itemEvt->set_newmemberid(joined_member_id);
    }
    return;
  }
  std::map<size_t, size_t> delta_partition_count_copy_map;
  delta_partition_count_copy_map.insert(delta_partition_count_map.begin(), delta_partition_count_map.end());
  std::map<size_t, size_t> primary_replaced_map;
  // balance primary node
  for (size_t i = 0; i < partition_count; ++i) {
    const int32_t primary_member_id = partitions.at(i).getMemberId(0);
    if (delta_partition_count_map[primary_member_id] > 0) {
      DeltaPartitionItemEvent* itemEvt = evt.add_items();
      itemEvt->set_needmigrate(true);
      itemEvt->set_partitionid(i);
      itemEvt->set_position(0);
      itemEvt->set_currmemberid(primary_member_id);
      itemEvt->set_newmemberid(joined_member_id);
      --delta_partition_count_map[primary_member_id];
      primary_replaced_map[i] = joined_member_id;
    }
  }
  // balance backup node
  if (balanceable_member_size == 2) { // all backup node 's member id  = -1
    for (size_t i = 0; i < partition_count; ++i) {
      const int32_t primary_member_id = partitions.at(i).getMemberId(0);
      const int32_t backup_member_id = partitions.at(i).getMemberId(1);
      DeltaPartitionItemEvent* itemEvt = evt.add_items();
      itemEvt->set_partitionid(i);
      itemEvt->set_position(1);
      itemEvt->set_currmemberid(backup_member_id);
      if (delta_partition_count_copy_map[primary_member_id] > 0) {
        itemEvt->set_needmigrate(false);
        itemEvt->set_newmemberid(primary_member_id); // backup replaced by primary node directly
        --delta_partition_count_copy_map[primary_member_id];
      } else {
        itemEvt->set_needmigrate(true);
        itemEvt->set_newmemberid(joined_member_id);
      }
    }
    return;
  }
  for (size_t i = 0; i < partition_count; ++i) {
    const int32_t primary_member_id = partitions.at(i).getMemberId(0);
    const int32_t backup_member_id = partitions.at(i).getMemberId(1);
    if (partition_count_map[backup_member_id] > 0) {
      --partition_count_map[backup_member_id];
      continue;
    }
    uint32_t memberId = joined_member_id;
    if (primary_replaced_map.count(i) > 0) {
      memberId = select(partition_count_map, joined_member_id);
    }
    DeltaPartitionItemEvent* itemEvt = evt.add_items();
    itemEvt->set_needmigrate(true);
    itemEvt->set_partitionid(i);
    itemEvt->set_position(1);
    itemEvt->set_currmemberid(backup_member_id);
    itemEvt->set_newmemberid(memberId);
  }
}

int32_t select(std::map<size_t, size_t>& map) {
  int32_t memberId = -1;
  for (std::map<size_t, size_t>::iterator it = map.begin(); it != map.end(); ++it) {
    if (it->second == 0) {
      continue;
    }
    memberId = it->first;
    --it->second;
    break;
  }
  return memberId;
}

int32_t select(std::map<size_t, size_t>& map, const size_t except_member_id) {
  int32_t memberId = -1;
  for (std::map<size_t, size_t>::iterator it = map.begin(); it != map.end(); ++it) {
    if (it->second == 0 || it->first == except_member_id) {
      continue;
    }
    memberId = it->first;
    --it->second;
    break;
  }
  return memberId;
}

void NaivePartitionBalancer::balanceWhenLeft(MembershipTable& members, PartitionTable& partitions, MemberWrapper& member, pb::DeltaPartitionEvent& evt) {
  const size_t member_size = members.size();
  const size_t partition_count = partitions.size();
  const size_t balanceable_member_size = getBalanceableMemberSize(members);
  const size_t balance_count =
      (balanceable_member_size == 0 || balanceable_member_size == 1) ?
          partition_count : partition_count / balanceable_member_size;
  size_t left_member_partition_count = member.getPartition(0) == 0 ? partition_count : member.getPartition(0);
  size_t balance_avg_count =
      balanceable_member_size == 0 ? partition_count : left_member_partition_count / (balanceable_member_size);
  size_t balance_mod_count = balanceable_member_size == 0 ? 0 : left_member_partition_count % (balanceable_member_size);
  std::map<size_t, size_t> delta_partition_count_map, partition_count_map;
  calculatePartitionCountWhenLeft(members, member, member_size, balance_count, balance_avg_count,
      balance_mod_count, delta_partition_count_map, partition_count_map);
//			displayPartitionCount(delta_partition_count_map, partition_count_map);
  if (balanceable_member_size == 0) {
    for (size_t i = 0; i < partition_count; ++i) {
      DeltaPartitionItemEvent* itemEvt = evt.add_items();
      itemEvt->set_partitionid(i);
      itemEvt->set_position(0);
      itemEvt->set_currmemberid(-1);
      itemEvt->set_newmemberid(-1);
      itemEvt->set_needmigrate(false);
    }
    return;
  }
  const uint32_t left_member_id = member.getId();
  // copy delta partition count map;
  std::map<size_t, size_t> delta_partition_count_copy_map;
  delta_partition_count_copy_map.insert(delta_partition_count_map.begin(), delta_partition_count_map.end());
  // balance primary node
  std::map<size_t, size_t> primary_replaced_map;
  for (size_t i = 0; i < partition_count; ++i) {
    const int32_t primary_member_id = partitions.at(i).getMemberId(0);
    const int32_t backup_member_id = partitions.at(i).getMemberId(1);
    if (primary_member_id != left_member_id) {
      continue;
    }
    int32_t memberId = -1;
    do {
      memberId = select(delta_partition_count_map);
    } while (memberId == -1);
    DeltaPartitionItemEvent* itemEvt = evt.add_items();
    itemEvt->set_partitionid(i);
    itemEvt->set_position(0);
    itemEvt->set_currmemberid(left_member_id);
    itemEvt->set_newmemberid(memberId);
    itemEvt->set_needmigrate(true);
    primary_replaced_map[i] = memberId;
  }
  // balance backup node
  if (balanceable_member_size == 1) { // only exists leading member
    for (size_t i = 0; i < partition_count; ++i) {
      const int32_t backup_member_id = partitions.at(i).getMemberId(1);
      DeltaPartitionItemEvent* itemEvt = evt.add_items();
      itemEvt->set_partitionid(i);
      itemEvt->set_position(1);
      itemEvt->set_currmemberid(backup_member_id);
      itemEvt->set_newmemberid(-1);
      itemEvt->set_needmigrate(false);
    }
    return;
  }
  for (size_t i = 0; i < partition_count; ++i) {
    const int32_t primary_member_id = partitions.at(i).getMemberId(0);
    const int32_t backup_member_id = partitions.at(i).getMemberId(1);
    if (backup_member_id == left_member_id) {
      int32_t member_id = -1;
      for (std::map<size_t, size_t>::iterator it = delta_partition_count_copy_map.begin();
          it != delta_partition_count_copy_map.end(); ++it) {
        if (it->second == 0 || it->first == primary_member_id) {
          continue;
        }
        member_id = it->first;
        --it->second;
        break;
      }
      if (member_id == -1) {
        for (std::map<size_t, size_t>::iterator it = partition_count_map.begin(); it != partition_count_map.end();
            ++it) {
          if (it->second == 0 || it->first == primary_member_id) {
            continue;
          }
          member_id = it->first;
          --it->second;
          break;
        }
      }
      DeltaPartitionItemEvent* itemEvt = evt.add_items();
      itemEvt->set_partitionid(i);
      itemEvt->set_position(1);
      itemEvt->set_currmemberid(backup_member_id);
      itemEvt->set_newmemberid(member_id);
      itemEvt->set_needmigrate(true);
      continue;
    }
    if (primary_replaced_map.count(i) > 0 && primary_replaced_map[i] == backup_member_id) {
      int32_t member_id = -1;
      for (std::map<size_t, size_t>::iterator it = partition_count_map.begin(); it != partition_count_map.end(); ++it) {
        if (it->second == 0 || it->first == primary_replaced_map[i]) {
          continue;
        }
        member_id = it->first;
        --it->second;
        break;
      }
      DeltaPartitionItemEvent* itemEvt = evt.add_items();
      itemEvt->set_partitionid(i);
      itemEvt->set_position(1);
      itemEvt->set_currmemberid(backup_member_id);
      itemEvt->set_newmemberid(member_id);
      itemEvt->set_needmigrate(true);
      continue;
    }
    if (partition_count_map[backup_member_id] > 0) {
      --partition_count_map[backup_member_id];
      continue;
    }
  }
}

void NaivePartitionBalancer::calculatePartitionCountWhenJoined(MembershipTable& members, MemberWrapper& joined_member, const size_t member_size, const size_t balance_count, const size_t balance_avg_count,
    const size_t balance_mod_count, std::map<size_t, size_t>& delta_partition_count_map,
    std::map<size_t, size_t>& partition_count_map) {
  const size_t max_backup_count = config->max_backup_count();
  for (size_t i = 0; i < member_size; ++i) {
    MemberWrapper& member = members.at(i);
    const int32_t member_id = member.getId();
    if (member_id == joined_member.getId()) {
      member.setPartitionCount(0, balance_count);
      member.setPartitionCount(1, member.getPartition(0) * max_backup_count);
      partition_count_map[member_id] = member.getPartition(0);
      continue;
    }
    if (!member.canBalanced()) {
      continue;
    }
    if (balance_avg_count > 0) {
      member.setPartitionCount(0, member.getPartition(0) - balance_avg_count);
      member.setPartitionCount(1, member.getPartition(0) * max_backup_count);
      delta_partition_count_map[member_id] = balance_avg_count;
    }
    partition_count_map[member_id] = member.getPartition(0);
  }
  // with mod count
  size_t count = 0;
  while (count < balance_mod_count) {
    // if possible, minus from member who own partition count > balance_count + 1
    for (size_t i = 0; count < balance_mod_count && i < member_size; ++i) {
      MemberWrapper& member = members.at(i);
      const int32_t member_id = member.getId();
      if (!member.canBalanced() || member.getId() == joined_member.getId()) {
        continue;
      }
      if (member.getPartition(0) > balance_count + 1) {
        member.setPartitionCount(0, member.getPartition(0) - 1);
        member.setPartitionCount(1, member.getPartition(0) * max_backup_count);
        ++count;
        ++delta_partition_count_map[member_id];
        --partition_count_map[member_id];
      }
    }
    // if possible, minus from member who own partition count == balance_count + 1
    for (size_t i = 0; count < balance_mod_count && i < member_size; ++i) {
      MemberWrapper& member = members.at(i);
      const int32_t member_id = member.getId();
      if (!member.canBalanced() || member.getId() == joined_member.getId()) {
        continue;
      }
      if (member.getPartition(0) == balance_count + 1) {
        member.setPartitionCount(0, member.getPartition(0) - 1);
        member.setPartitionCount(1, member.getPartition(0) * max_backup_count);
        ++count;
        ++delta_partition_count_map[member_id];
        --partition_count_map[member_id];
      }
    }
    // if possible, minus from member who own partition count > balance_count
    for (size_t i = 0; count < balance_mod_count && i < member_size; ++i) {
      MemberWrapper& member = members.at(i);
      const int32_t member_id = member.getId();
      if (!member.canBalanced() || member.getId() == joined_member.getId()) {
        continue;
      }
      if (member.getPartition(0) > balance_count) {
        member.setPartitionCount(0, member.getPartition(0) - 1);
        member.setPartitionCount(1, member.getPartition(0) * max_backup_count);
        ++count;
        ++delta_partition_count_map[member_id];
        --partition_count_map[member_id];
      }
    }
    // if possible, minus from member who own partition count == balance_count
    for (size_t i = 0; count < balance_mod_count && i < member_size; ++i) {
      MemberWrapper& member = members.at(i);
      const int32_t member_id = member.getId();
      if (!member.canBalanced() || member.getId() == joined_member.getId()) {
        continue;
      }
      if (member.getPartition(0) == balance_count) {
        member.setPartitionCount(0, member.getPartition(0) - 1);
        member.setPartitionCount(1, member.getPartition(0) * max_backup_count);
        ++count;
        ++delta_partition_count_map[member_id];
        --partition_count_map[member_id];
      }
    }
  }
}

void NaivePartitionBalancer::calculatePartitionCountWhenLeft(MembershipTable& members, MemberWrapper& left_member, const size_t member_size, const size_t balance_count, const size_t balance_avg_count,
    const size_t balance_mod_count, std::map<size_t, size_t>& delta_partition_count_map,
    std::map<size_t, size_t>& partition_count_map) {
  const size_t max_backup_count = config->max_backup_count();
  // active member append balance_count
  for (size_t i = 0; i < member_size; ++i) {
    MemberWrapper& member = members.at(i);
    const int32_t member_id = member.getId();
    if (member_id == left_member.getId()) {
      member.setPartitionCount(0, 0);
      member.setPartitionCount(1, 0);
      continue;
    }
    if (!member.canBalanced()) {
      continue;
    }
    if (balance_avg_count > 0) {
      member.setPartitionCount(0, member.getPartition(0) + balance_avg_count);
      member.setPartitionCount(1, member.getPartition(0) * max_backup_count);
      delta_partition_count_map[member_id] = balance_avg_count;
    }
    partition_count_map[member_id] = member.getPartition(0);
  }
  // with mode count
  size_t count = 0;
  while (count < balance_mod_count) {
    // if possible, plus from member who own partition count < balance_count
    for (size_t i = 0; count < balance_mod_count && i < member_size; ++i) {
      MemberWrapper& member = members.at(i);
      const int32_t member_id = member.getId();
      if (!member.canBalanced() || member.getId() == left_member.getId()) {
        continue;
      }
      if (member.getPartition(0) < balance_count) {
        member.setPartitionCount(0, member.getPartition(0) + 1);
        member.setPartitionCount(1, member.getPartition(0) * max_backup_count);
        ++count;
        ++delta_partition_count_map[member_id];
        ++partition_count_map[member_id];
      }
    }
    // if possible, plus from member who own partition count == balance_count + 1
    for (size_t i = 0; count < balance_mod_count && i < member_size; ++i) {
      MemberWrapper& member = members.at(i);
      const int32_t member_id = member.getId();
      if (!member.canBalanced() || member.getId() == left_member.getId()) {
        continue;
      }
      if (member.getPartition(0) == balance_count) {
        member.setPartitionCount(0, member.getPartition(0) + 1);
        member.setPartitionCount(1, member.getPartition(0) * max_backup_count);
        ++count;
        ++delta_partition_count_map[member_id];
        ++partition_count_map[member_id];
      }
    }
    // if possible, minus from member who own partition count > balance_count
    for (size_t i = 0; count < balance_mod_count && i < member_size; ++i) {
      MemberWrapper& member = members.at(i);
      const int32_t member_id = member.getId();
      if (!member.canBalanced() || member.getId() == left_member.getId()) {
        continue;
      }
      if (member.getPartition(0) == balance_count + 1) {
        member.setPartitionCount(0, member.getPartition(0) + 1);
        member.setPartitionCount(1, member.getPartition(0) * max_backup_count);
        ++count;
        ++delta_partition_count_map[member_id];
        ++partition_count_map[member_id];
      }
    }
    // if possible, minus from member who own partition count == balance_count
    for (size_t i = 0; count < balance_mod_count && i < member_size; ++i) {
      MemberWrapper& member = members.at(i);
      const int32_t member_id = member.getId();
      if (!member.canBalanced() || member.getId() == left_member.getId()) {
        continue;
      }
      if (member.getPartition(0) < balance_count + 1) {
        member.setPartitionCount(0, member.getPartition(0) + 1);
        member.setPartitionCount(1, member.getPartition(0) * max_backup_count);
        ++count;
        ++delta_partition_count_map[member_id];
        ++partition_count_map[member_id];
      }
    }
  }
}
}
}

