
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/cluster/partitiontable_balance_verifier.h"
using namespace idgs::pb;
using namespace std;

namespace idgs {

namespace cluster {

void PartitiontableBalanceVerifier::total(std::shared_ptr<VerifyResult>& prs, VerifyResult& rs, size_t activeMemberSize,
    std::map<size_t, TotalResult>& totalResult) {
  if (totalResult.count(activeMemberSize) > 0) {
    // calculate max, min time taken
    if (rs.timeTaken > totalResult[activeMemberSize].maxTimeTaken) {
      totalResult[activeMemberSize].maxTimeTaken = rs.timeTaken;
    }
    if (rs.timeTaken < totalResult[activeMemberSize].minTimeTaken) {
      totalResult[activeMemberSize].minTimeTaken = rs.timeTaken;
    }
    // calculate max, min migrate count
    if (rs.migrateCount > totalResult[activeMemberSize].maxMigrateCount) {
      totalResult[activeMemberSize].maxMigrateCount = rs.migrateCount;
      totalResult[activeMemberSize].maxResult = prs;
    }
    if (rs.migrateCount < totalResult[activeMemberSize].minMigrateCount) {
      totalResult[activeMemberSize].minResult = prs;
      totalResult[activeMemberSize].minMigrateCount = rs.migrateCount;
    }
    ++totalResult[activeMemberSize].count;
    totalResult[activeMemberSize].sumTimeTaken += rs.timeTaken;
    totalResult[activeMemberSize].avgTimeTaken = totalResult[activeMemberSize].sumTimeTaken
        / totalResult[activeMemberSize].count;
    totalResult[activeMemberSize].sumMigrateCount += rs.migrateCount;
    totalResult[activeMemberSize].avgMigrateCount = totalResult[activeMemberSize].sumMigrateCount
        / totalResult[activeMemberSize].count;
    if (totalResult[activeMemberSize].deltaBackupCount.count(rs.backupMaxMinDeltaCount) > 0) {
      ++totalResult[activeMemberSize].deltaBackupCount[rs.backupMaxMinDeltaCount];
    } else {
      totalResult[activeMemberSize].deltaBackupCount[rs.backupMaxMinDeltaCount] = 1;
    }
  } else {

    totalResult[activeMemberSize].count = 1;

    totalResult[activeMemberSize].sumTimeTaken = totalResult[activeMemberSize].avgTimeTaken = rs.timeTaken;
    totalResult[activeMemberSize].maxTimeTaken = totalResult[activeMemberSize].minTimeTaken = rs.timeTaken;

    totalResult[activeMemberSize].sumMigrateCount = totalResult[activeMemberSize].avgMigrateCount = rs.migrateCount;
    totalResult[activeMemberSize].maxMigrateCount = totalResult[activeMemberSize].minMigrateCount = rs.migrateCount;

    totalResult[activeMemberSize].minResult = totalResult[activeMemberSize].maxResult = prs;

    totalResult[activeMemberSize].deltaBackupCount[rs.backupMaxMinDeltaCount] = 1;
  }
}

size_t PartitiontableBalanceVerifier::count(std::map<size_t, TotalResult>& totalResult) {
  fprintf(stderr, "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t\n", "size", "count", "max-time(ms)", "min-time(ms)", "avg-time(ms)",
      "max-migrate", "min-migrate", "avg-migrate", "backup(max-min), count(s)");
  size_t sumCount = 0;
  for (std::map<size_t, TotalResult>::iterator it = totalResult.begin(); it != totalResult.end(); ++it) {
    sumCount += it->second.count;
    stringstream str;
    int i = 0;
    for (std::map<size_t, size_t>::iterator subIt = it->second.deltaBackupCount.begin();
        subIt != it->second.deltaBackupCount.end(); ++subIt, ++i) {
      if (i > 0) {
        str << " ;";
      }
      str << subIt->first << ", " << subIt->second;
    }
    fprintf(stderr, "%2d\t%4d\t%8.4f\t%8.4f\t%8.4f\t%8d\t%8d\t%8d\t%s\n", (int) it->first, (int) it->second.count,
        it->second.maxTimeTaken, it->second.minTimeTaken, it->second.avgTimeTaken, (int) it->second.maxMigrateCount,
        (int) it->second.minMigrateCount, (int) it->second.avgMigrateCount, str.str().c_str());

  }
  return sumCount;
}

/// @todo never dump string to stdout directly.
void PartitiontableBalanceVerifier::display(std::map<size_t, TotalResult>& joinedTotalResult,
    std::map<size_t, TotalResult>& leftTotalResult, size_t LOOP_COUNT, size_t MAX_MEMBER_SIZE, size_t MIN_MEMBER_SIZE,
    size_t partitionCount, size_t maxBackupCount) {
  // show total result
  cerr
      << "###################################################summary result###################################################"
      << endl;
  cerr << "input argument: " << endl << "loop count = " << LOOP_COUNT << endl << "partition count = " << partitionCount
      << endl << "max backup count = " << maxBackupCount << endl << "max active member size = " << MAX_MEMBER_SIZE
      << endl << "min active member size = " << MIN_MEMBER_SIZE << endl;
  cerr
      << "---------------------------------------------------------when join---------------------------------------------------------"
      << endl;
  size_t sumCount, joinedCount, leaveCount;
  joinedCount = count(joinedTotalResult);
  cerr << "join count: " << joinedCount << endl;
  cerr
      << "---------------------------------------------------------when leave--------------------------------------------------------"
      << endl;
  leaveCount = count(leftTotalResult);
  cerr << "leave count: " << leaveCount << endl;
  sumCount = joinedCount + leaveCount;
  if (sumCount != LOOP_COUNT) {
    LOG(ERROR)<< "error, total result count is "<< sumCount <<", not equal loop count " << LOOP_COUNT;
  }
  cerr
      << "###################################################summary result###################################################"
      << endl;
}

VerifyErrCode PartitiontableBalanceVerifier::verify(MembershipTableMgr& memberMgr, PartitionTableMgr& partitionMgr,
    VerifyResult& rs, const size_t expectMigrateCount, DeltaPartitionEvent& evt) {
  const size_t memberSize = memberMgr.getMemberTable().size();
  const size_t partitionCount = partitionMgr.getPartitionTable().size();
  size_t balanceable_member_size = memberMgr.getBalanceableMemberSize();
  if (balanceable_member_size == 0) {
    rs.migrateCount = 0;
    rs.backupMaxMinDeltaCount = 0;
    return RC_VEFIFY_SUCCESS;
  }
  for (int i = 0; i < partitionCount; ++i) {
    int32_t primaryNode = partitionMgr.getPartition(i)->getMemberId(0);
    int32_t backupNode = partitionMgr.getPartition(i)->getMemberId(1);
    if (primaryNode == backupNode) {
      return RC_VEFIFY_ERROR_DUPLICATE_PRIMARY_BACKUP;
    }
  }
  size_t maxPrimaryPartitionCount, minPrimaryPartitionCount, maxBackupPartitionCount, minBackupPartitionCount;
  int max_min_index = -1;
  for (int i = 0; i < memberSize; ++i) {
    MemberWrapper* member = memberMgr.getMember(i);
    if (member->isInitial() || member->isLeave()) {
      continue;
    }
    if (max_min_index == -1) {
      maxPrimaryPartitionCount = minPrimaryPartitionCount = memberMgr.getMember(i)->getPartition(0);
      maxBackupPartitionCount = minBackupPartitionCount = memberMgr.getMember(i)->getPartition(1);
      max_min_index = i;
      continue;
    }
    size_t primaryCount = member->getPartition(0);
    size_t backupCount = member->getPartition(1);
    // primary
    if (primaryCount > maxPrimaryPartitionCount) {
      maxPrimaryPartitionCount = primaryCount;
    }
    if (primaryCount < minPrimaryPartitionCount) {
      minPrimaryPartitionCount = primaryCount;
    }
    // backup
    if (backupCount > maxBackupPartitionCount) {
      maxBackupPartitionCount = backupCount;
    }
    if (backupCount < minBackupPartitionCount) {
      minBackupPartitionCount = backupCount;
    }
  }
  if (maxPrimaryPartitionCount - minPrimaryPartitionCount > 1) {
    return RC_VEFIFY_ERROR_DUPLICATE_PRIMARY_NOT_BALANCED;
  }
  rs.backupMaxMinDeltaCount = maxBackupPartitionCount - minBackupPartitionCount;
  const size_t size = evt.items_size();
  size_t sum = 0;
  for (int i = 0; i < size; ++i) {
    if (!evt.mutable_items(i)->needmigrate()) {
      continue;
    }
    ++sum;
  }
  rs.migrateCount = sum;
  if (expectMigrateCount != rs.migrateCount) {
//			  return RC_VEFIFY_ERROR_MIGRATE_COUNT;
  }
  return RC_VEFIFY_SUCCESS;
}
}
}

