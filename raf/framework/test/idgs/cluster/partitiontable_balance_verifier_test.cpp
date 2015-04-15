
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/cluster/cluster_framework.h"
#include "idgs/cluster/balancer/partition_balance_verifier.h"
#include "idgs/cluster/balancer/balancer_util.h"
#include <gtest/gtest.h>

#include "idgs/cluster/cluster_framework.h"

using namespace idgs::cluster;
using namespace idgs::pb;

typedef std::chrono::high_resolution_clock Clock;
static bool isPrime(size_t number);
static void verifyBalance(unsigned int seed, size_t max_reserved_size, size_t partition_count,  size_t loop_count, size_t max_member_size, size_t min_member_size);

TEST(partitiontable_balance_verifier, verify) {
  srand(time(NULL));
  verifyBalance(rand(), 100, 17, 20, 5, 1);

  // return;

  for (size_t i = 17; i < 50; ++i) {
    if(isPrime(i)) {
      size_t min_member_size = rand() % i;
      if(min_member_size > 20) {
        min_member_size = rand() % 20;
      }
      if (min_member_size <= 0) {
        min_member_size = 1;
      }
      size_t max_member_size = min_member_size + 5 + rand() % 20;
      if(max_member_size > i) {
        max_member_size = i;
      }
      unsigned int seed = rand();
      verifyBalance(seed, 100, i , 2014, max_member_size, min_member_size);
    }
  }
}

static bool isPrime(size_t number) {
  if (number == 2) {
    return true;
  }
  if ((number & 1) == 0) {
    return false;
  }
  size_t root = sqrt(number);
  for (size_t i = 3; i <= root; i += 2) {
    if((number % i) == 0) {
      return false;
    }
  }
  return true;
}

static void verifyBalance(unsigned int seed, size_t max_reserved_size, size_t partition_count,  size_t loop_count, size_t max_member_size, size_t min_member_size) {
  srand(seed);
  LOG(INFO) << "verifyBalance(" << seed << ", " << max_reserved_size <<  ", " << partition_count << ", " << loop_count << ", " << max_member_size << ", " << min_member_size << ");";
  assert(min_member_size <= max_member_size && max_member_size <= partition_count);

  BalanceVerifyReport report;
  report.LOOP_COUNT = loop_count;
  report.MAX_MEMBER_SIZE = max_member_size;
  report.MIN_MEMBER_SIZE = min_member_size;
  report.partitionCount = partition_count;

  ClusterConfig cfg;
  cfg.set_reserved_member_size(max_reserved_size);
  cfg.set_partition_count(partition_count);
  cfg.set_max_replica_count(REPLICAS);

  MemberManagerActor memberMgr;
  memberMgr.init(&cfg);

  PartitionManagerActor partitionMgr;
  partitionMgr.init(&cfg);
  partitionMgr.setMembershipTableManager(memberMgr);


  for (size_t i = 0; i < loop_count; ++i) {
    MemberWrapper* memberPtr;
    size_t available_member_count = memberMgr.getBalanceableMemberSize();

    ///
    /// decide whether join or leave
    ///
    bool join = rand() % 2;
    if(available_member_count <= min_member_size) {
      join = true;
    } else if( available_member_count >= max_member_size){
      join = false;
    }

    ///
    /// join or leave
    ///
    if(join) {
      // create a new member and join into the cluster
      MemberWrapper member;
      member.setMember(cfg.member());
      member.setPid(i);
      member.setNodeId(1234);
      member.setLeading(i == 0);
      memberPtr = memberMgr.addMember(member);
      memberPtr->setState(MS_JOINED);
    } else {
      // random find an available member, and set its state to INACTIVE
      const size_t whole_member_size = memberMgr.getMemberSize();
      int leaveMemberId = rand() % whole_member_size;
      memberPtr = memberMgr.getMember(leaveMemberId);
      while(memberPtr->getState() == idgs::pb::MS_INITIAL || memberPtr->getState() == idgs::pb::MS_INACTIVE) {
        ++leaveMemberId;
        if(leaveMemberId >= whole_member_size) {
          leaveMemberId = 0;
        }
        memberPtr = memberMgr.getMember(leaveMemberId);
      }
      memberPtr->setState(MS_INACTIVE);
    }

    std::vector<MemberWrapper> old_members = (memberMgr.getMemberTable());
    std::vector<PartitionWrapper> old_partitions = (partitionMgr.getPartitionTable());

    ///
    /// rebalance
    ///
    DeltaPartitionEvent partDelta;
    BalanceVerifyResult verifyResult;

    auto start = Clock::now();
    std::vector<MemberWrapper>& membershipTable = const_cast<std::vector<MemberWrapper>&>(memberMgr.getMemberTable());
    std::vector<PartitionWrapper>& partitionTable = const_cast<std::vector<PartitionWrapper>&>(partitionMgr.getPartitionTable());
    auto balancer = BalancerUtil::createBalancer();
    balancer->setConfig(&cfg);
    int ret = balancer->balance(membershipTable,partitionTable, (*memberPtr).getId(), partDelta);
    std::chrono::nanoseconds duration = std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - start);
    verifyResult.duration = duration.count() / 1.000 / 1000000;

    ///
    /// update partition table, and migrate data (set to ready directly)
    ///
		partitionMgr.balanceOk(partDelta);
    for (auto& p: partitionTable) {
      for (int i = 0; i < cfg.max_replica_count(); ++i) {
        if (p.getMemberId(i) >= 0) {
          p.setState(i, idgs::pb::PS_READY);
        }
      }
    }

    //
    // Verify the rebalance result
    //
    size_t expectMigrateCount = 0;
    auto& members = memberMgr.getMemberTable();
    auto& partitions = partitionMgr.getPartitionTable();
    PartitionBalanceVerifier::verifyBalance(const_cast<std::vector<MemberWrapper>&>(members), partitions, -1, partDelta, expectMigrateCount, verifyResult);
    available_member_count =  memberMgr.getBalanceableMemberSize();

    {
      bool dump = false;
      int invalid_replica;
      for (invalid_replica = 0; invalid_replica < std::min(available_member_count, REPLICAS); ++invalid_replica) {
        if (true
            && verifyResult.balance[invalid_replica] > (invalid_replica + 2)
            && verifyResult.balance[invalid_replica] > (partition_count / available_member_count / 5)) {
          dump = true;
          break;
        }
      }
      if (ret || dump) {
        LOG(ERROR) << "partition count: " << partition_count << ", member: " << available_member_count << ", replica: " << invalid_replica << ", value: " << verifyResult.balance[invalid_replica];
        verifyResult.print(std::cerr) << std::endl;

        LOG(ERROR) << "old member table";
        for (auto& m : old_members) {
          std::cerr << m.toShortString() << std::endl;
        }

        LOG(ERROR) << "new member table" << memberMgr.toSimpleString();

        LOG(ERROR) << "old partition table";
        for (int i = 0, size = partitions.size(); i < size; ++i) {
          std::cerr << std::setw(4) << i << " | ";
          auto& p = old_partitions.at(i);
          auto nodes = p.getPartition().cells_size();
          for(auto j = 0; j < nodes; ++j) {
            std::cerr << std::setw(3) << p.getMemberId(j) << "(" << p.getState(j) << ")" << " | ";
          }
          std::cerr << std::endl;
        }

        LOG(ERROR) << "new partition table" << partitionMgr.toString();
        if (ret) {
          exit(1);
        }
      }
    }

    //
    // statistics
    //
    report.accumulate(verifyResult, join, available_member_count);


    //
    // update member status
    //
    if(memberPtr->getState() == idgs::pb::MS_JOINED) {
      memberPtr->setState(MS_PREPARED);
    }
  } // loop

  //
  // print report
  //
  report.print(std::cerr) << std::endl;
}
