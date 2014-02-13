
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/cluster/cluster_framework.h"
#include "idgs/cluster/partitiontable_balance_verifier.h"
#include <gtest/gtest.h>

#include "idgs/cluster/cluster_framework.h"

using namespace idgs::cluster;
using namespace idgs::pb;
using namespace std;

typedef std::chrono::high_resolution_clock Clock;
static bool isPrime(size_t number);
static void verifyBalance(unsigned int seed, size_t max_reserved_size, size_t partition_count,  size_t loop_count, size_t max_member_size, size_t min_member_size);

TEST(partitiontable_balance_verifier, verify) {
  ClusterFramework& cluster = ::idgs::util::singleton<ClusterFramework>::getInstance();
  cluster.loadCfgFile("framework/conf/cluster.conf");
  srand(clock());
  verifyBalance(clock(), 100, 1023, 800, 20, 0);
  for (size_t i = 17; i < 200; ++i) {
    if(isPrime(i)) {
      size_t min_member_size = rand() % i;
      if(min_member_size > 20) {
        min_member_size = rand() % 20;
      }
      size_t max_member_size = min_member_size + 5 + rand() % 20;
      if(max_member_size > i) {
        max_member_size = i;
      }
      clock_t seed = clock();
      verifyBalance(seed, 100, i , 800, max_member_size, min_member_size);
    }
  }
}

static bool isPrime(size_t number) {
  size_t root = sqrt(number);
  for (size_t i = 2; i <= root; ++i) {
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
  map<size_t, TotalResult> joinedTotalResult, leftTotalResult;


  ClusterConfig cfg;
  cfg.set_reserved_member_size(max_reserved_size);
  cfg.set_partition_count(partition_count);
  cfg.set_max_backup_count(1);

  MembershipTableMgr memberMgr;
  memberMgr.init(&cfg);

  PartitionTableMgr partitionMgr;
  partitionMgr.init(&cfg);
  partitionMgr.setMembershipTableManager(memberMgr);


  for (size_t i = 0; i < loop_count; ++i) {
    MemberWrapper member;
    member.setMember(::idgs::util::singleton<ClusterFramework>::getInstance().getMemberConfig());
    MemberWrapper* memberPtr;
    size_t balanceable_member_size = memberMgr.getBalanceableMemberSize();
    bool flag = rand() % 2;
    if(balanceable_member_size <= min_member_size) {
      flag = true;
    } else if( balanceable_member_size >= max_member_size){
      flag = false;
    }
    if(flag) {
      // join
      member.setIsleading(i == 0);
      memberPtr = memberMgr.addMember(member);
      memberPtr->setStatus(JOINED);
    } else {
      // leave
      const size_t whole_member_size = memberMgr.getMemberSize();
      int leaveMemberId = rand() % whole_member_size;
      memberPtr = memberMgr.getMember(leaveMemberId);
      while(memberPtr->isInitial() || memberPtr->isLeave()) {
        ++leaveMemberId;
        if(leaveMemberId > whole_member_size - 1) {
          leaveMemberId = 0;
        }
        memberPtr = memberMgr.getMember(leaveMemberId);
      }
      memberPtr->setStatus(INACTIVE);
    }
    std::shared_ptr<VerifyResult> prs(new VerifyResult);
    std::shared_ptr<DeltaPartitionEvent> evt_ptr(new DeltaPartitionEvent);
    VerifyResult& rs = *prs;
    rs.partitionTable = std::shared_ptr<PartitionTable>(new idgs::pb::PartitionTable);
		partitionMgr.genPartitionTable(*(rs.partitionTable.get()));
    auto start = Clock::now();
    std::vector<MemberWrapper>& membershipTable = const_cast<std::vector<MemberWrapper>&>(memberMgr.getMemberTable());
    std::vector<PartitionWrapper>& partitionTable = const_cast<std::vector<PartitionWrapper>&>(partitionMgr.getPartitionTable());
		partitionMgr.getPartitionBalancer()->balance(membershipTable,partitionTable, *memberPtr, *evt_ptr.get());
		partitionMgr.balanceOk(*evt_ptr.get());
//    LOG(INFO) << "after balance,  member " << memberPtr->getId() << " " << memberPtr->getStringStatus() << endl << memberMgr.toString() << partitionMgr.toString();
    std::chrono::nanoseconds duration = std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - start);
    rs.timeTaken = duration.count() / 1.000 / 1000000;
    size_t expectMigrateCount = 0;
    VerifyErrCode errCode = PartitiontableBalanceVerifier::verify(memberMgr, partitionMgr, rs, expectMigrateCount, *evt_ptr.get());
    if(errCode == RC_VEFIFY_ERROR_DUPLICATE_PRIMARY_BACKUP) {
    	LOG(ERROR) << "verify error, error code = " << errCode << ", caused by duplicate primary node and backup node, when member "
      		<< memberPtr->getId() << ", status: "<< MemberStatus_Name(memberPtr->getStatus());
    }
    else if(errCode == RC_VEFIFY_ERROR_DUPLICATE_PRIMARY_NOT_BALANCED){
      LOG(ERROR) << "verify error, error code = " << errCode << ", caused by primary node own max partition count - min partition count > 1 , when member "
      		<< memberPtr->getId() << ", status: "<< MemberStatus_Name(memberPtr->getStatus());
    }
    else if(errCode == RC_VEFIFY_ERROR_MIGRATE_COUNT) {
      LOG(ERROR) << "verify error, error code = " << errCode << ", caused by migrate count: " << rs.migrateCount << ", not equals to expect migrate: " << expectMigrateCount
      		<< ", when member " << memberPtr->getId() << ", status: "<< MemberStatus_Name(memberPtr->getStatus());
    }
    // count
    balanceable_member_size =  memberMgr.getBalanceableMemberSize();
    if(memberPtr->isJoined() || memberPtr->isPrepared()) {
      PartitiontableBalanceVerifier::total(prs, rs, balanceable_member_size, joinedTotalResult);
    }
    else if(memberPtr->isLeave()) {
      PartitiontableBalanceVerifier::total(prs, rs, balanceable_member_size, leftTotalResult);
    }
    // update member status
    if(memberPtr->isJoined()) {
      memberPtr->setStatus(PREPARED);
    }
  }
  // show total result
  PartitiontableBalanceVerifier::display(joinedTotalResult, leftTotalResult, loop_count, max_member_size, min_member_size, partition_count, 1);
}
