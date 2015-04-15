
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/cluster/balancer/partition_balance_verifier.h"
#include "idgs/cluster/balancer/balancer_util.h"
using namespace std;

namespace idgs {

namespace cluster {

std::ostream & BalanceVerifySummay::print(std::ostream& os) {
  stringstream ss;
  bool firstReplica = true;
  for (auto& map : balance) {
    if (firstReplica) {
      firstReplica = false;
    } else {
      ss <<"; ";
    }
    bool firstBalance = true;
    ss << "[";
    for (auto& p : map) {
      if (firstBalance) {
        firstBalance = false;
      } else {
        ss << ", ";
      }
      ss << p.first << ":" << p.second;
    }
    ss << "]";
  }
  char buff[2048];
  snprintf(buff, 2048, "%d\t%d\t%.4f,%.4f,%.4f\t%d,%d,%d\t%d,%d,%d\t%s",
      (int) memberSize,
      (int) count,
      maxDuration,
      minDuration,
      (sumDuration / count),
      (int) maxMigrateCount,
      (int) minMigrateCount,
      (int) (sumMigrateCount / count),
      (int) maxChangeCount,
      (int) minChangeCount,
      (int) (sumChangeCount / count),
      ss.str().c_str());
  os << buff;

  return os;
}

int BalanceVerifyReport::count(std::ostream& os, std::map<size_t, BalanceVerifySummay>& summary) {
  char buff[2048];
  snprintf(buff, 2048, "%s\t%s\t%s\t%s\t%s\t%s",
      "Member",
      "Loop",
      "Duration(max,min,avg)",
      "Migration",
      "Changes",
      "replica => [balance =>count]}");
  os << buff << std::endl;
  size_t sumCount = 0;
  for (auto& pair : summary) {
    pair.second.memberSize = pair.first;
    pair.second.print(os) << std::endl;
    sumCount += pair.second.count;
  }
  return sumCount;
}


std::ostream & BalanceVerifyReport::print(std::ostream& os) {
  os << "######################################################################################################" << std::endl;
  os << "input argument: " << std::endl
      << "loop count = " << LOOP_COUNT << std::endl
      << "partition count = " << partitionCount << std::endl
      << "replica count = " << replicaCount << std::endl
      << "active member range = [" << MIN_MEMBER_SIZE << ", " << MAX_MEMBER_SIZE << "]" << std::endl;

  size_t sumCount, joinedCount, leaveCount;

  os << "-----------------------------------------------------------------------------------------------------" << std::endl;
  joinedCount = count(os, joinSummary);
  os << "-----------------------------------------------------------------------------------------------------" << std::endl;
  os << "Total join: " << joinedCount << std::endl;

  os << "-----------------------------------------------------------------------------------------------------" << std::endl;
  leaveCount = count(os, leaveSummary);
  os << "-----------------------------------------------------------------------------------------------------" << std::endl;
  os << "Total leave: " << leaveCount << std::endl;

  sumCount = joinedCount + leaveCount;
  if (sumCount != LOOP_COUNT) {
    LOG(ERROR)<< "Total result count is "<< sumCount <<", not equal loop count " << LOOP_COUNT;
  }

  return os;
}

void BalanceVerifyReport::accumulate (const BalanceVerifyResult& r, bool join, int memberCount) {
  if (join) {
    auto& summary = joinSummary[memberCount];
    summary.accumulate(r);
  } else {
    auto& summary = leaveSummary[memberCount];
    summary.accumulate(r);
  }
}


/**
 * Balance partition table when member join or leave
 * @param members [in, out] membership table, partition count array will be update
 * @param partitions [in] new partition table
 * @param mid [in] join/leave member
 * @param delta [in] delta partition event
 * @param expectMigrateCount [in] expected migrate partition count
 * @param bvr [out] the verify result
 */
void PartitionBalanceVerifier::verifyBalance(std::vector<MemberWrapper>& members, const std::vector<PartitionWrapper>& partitions,
    int mid, const pb::DeltaPartitionEvent& delta, const size_t expectMigrateCount,
    BalanceVerifyResult& bvr) {
  int partition_count = partitions.size();
  int replica_count = partitions[0].getReplicaCount();

  BalancerUtil::calcualtePartitionCount(partition_count, replica_count, partitions, members);
  for (auto& m: members) {
    if (m.isAvailable()) {
      for (int i = 0; i < replica_count; ++i) {
        int balance = std::abs((int)m.getPartitionCount(i) - m.getExpectPartCount());
        bvr.balance[i] = std::max((size_t)balance, bvr.balance[i]);
      }
    }
  }

  std::vector<MigrateAction> actions;
  BalancerUtil::generateMigrateActionList(partitions, delta, actions);
  int migrateCount = 0;
  for (auto& action: actions) {
    if (action.command == MigrateAction::NEW) {
      ++migrateCount;
    }
  }
  bvr.migrateCount = migrateCount;

  bvr.changeCount = delta.items_size();
}


} // namespace cluster
} // namespace idgs
