
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <limits>
#include <array>

#include "idgs/cluster/partition_manager_actor.h"

namespace idgs {
namespace cluster {

const size_t REPLICAS = 3;

///
/// verify rebalance result
///
struct BalanceVerifyResult {
  // in ms
  double duration;

  // balance for each replica: Max(abs(expected_partition_count - actual_partition_count))
  std::array<size_t, REPLICAS> balance;

  // migration action count
  size_t migrateCount;

  // change count
  size_t changeCount;

  BalanceVerifyResult() {
    duration = 0;
    for (auto& v : balance) {
      v = std::numeric_limits<size_t>::min();
    }
    migrateCount = 0;
    changeCount = 0;
  }

  std::ostream& print(std::ostream& os) {
    bool first = true;
    os << "[";
    for (auto v: balance) {
      if (first) {
        first = false;
      } else {
        os << ", ";
      }
      os << v;
    }
    os << "]";
    os << ", " << migrateCount;
    os << ", " << changeCount;
    return os;
  }
};

///
/// verify rebalance result summary
/// same partition count
/// same member count
/// same trigger: join or leave
///
struct BalanceVerifySummay {
  /// max members
  size_t memberSize;

  /// loop count
  size_t count;

  // duration
  double maxDuration;
  double minDuration;
  double sumDuration;

  /// balance for each replica
  /// map: balance ==> occur count
  std::array<std::map<size_t, size_t>, REPLICAS> balance;

  // migration action count
  size_t maxMigrateCount;
  size_t minMigrateCount;
  size_t sumMigrateCount;

  // migration action count
  size_t maxChangeCount;
  size_t minChangeCount;
  size_t sumChangeCount;

  BalanceVerifySummay() {
    memberSize = 0;
    count = 0;

    maxDuration = std::numeric_limits<double>::min();
    minDuration = std::numeric_limits<double>::max();
    sumDuration = 0;

    maxMigrateCount = std::numeric_limits<size_t>::min();
    minMigrateCount = std::numeric_limits<size_t>::max();
    sumMigrateCount = 0;

    maxChangeCount = std::numeric_limits<size_t>::min();
    minChangeCount = std::numeric_limits<size_t>::max();
    sumChangeCount = 0;

  }

  void accumulate (const BalanceVerifyResult& r) {
    ++count;

    /// duration
    maxDuration = std::max(maxDuration, r.duration);
    minDuration = std::min(minDuration, r.duration);
    sumDuration += r.duration;

    /// migration count
    maxMigrateCount = std::max(maxMigrateCount, r.migrateCount);
    minMigrateCount = std::min(minMigrateCount, r.migrateCount);
    sumMigrateCount += r.migrateCount;

    /// migration count
    maxChangeCount = std::max(maxChangeCount, r.changeCount);
    minChangeCount = std::min(minChangeCount, r.changeCount);
    sumChangeCount += r.changeCount;

    /// max balance statistics
    for (int i = 0; i < REPLICAS; ++i) {
      auto b = r.balance[i];
      auto & map = balance[i];
      ++map[b];
    }
  }

  std::ostream & print(std::ostream& os);
};

///
/// verify rebalance result report
/// same partition count
/// same member count
/// same trigger: join or leave
///
struct BalanceVerifyReport {
  size_t partitionCount;
  size_t replicaCount = REPLICAS;

  /// member count ==> summary
  std::map<size_t, BalanceVerifySummay> joinSummary;

  /// member count ==> summary
  std::map<size_t, BalanceVerifySummay> leaveSummary;

  size_t LOOP_COUNT;
  size_t MAX_MEMBER_SIZE;
  size_t MIN_MEMBER_SIZE;

  int count(std::ostream& os, std::map<size_t, BalanceVerifySummay>& summary);
  std::ostream & print(std::ostream& os);
  void accumulate (const BalanceVerifyResult& r, bool join, int memberCount);
};


///
/// Partition balance verifier.
///
class PartitionBalanceVerifier {
public:
  /**
   * Balance partition table when member join or leave
   * @param members [in, out] membership table, partition count array will be update
   * @param partitions [in] partition table
   * @param mid [in] join/leave member
   * @param delta [in] delta partition event
   * @param expectMigrateCount [in] expected migrate partition count
   * @param bvr [out] the verify result
   */
  static void verifyBalance(std::vector<MemberWrapper>& members, const std::vector<PartitionWrapper>& partitions,
      int mid, const pb::DeltaPartitionEvent& delta, const size_t expectMigrateCount,
      BalanceVerifyResult& bvr);
};
} // namespace cluster
} // namespace idgs

