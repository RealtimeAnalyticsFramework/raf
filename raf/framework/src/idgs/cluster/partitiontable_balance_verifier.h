
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "partitiontable_mgr.h"

namespace idgs {

namespace cluster {

/*
 * This class is designed to verify whether partition table is balanced when member join or leave
 */

enum VerifyErrCode {

  RC_VEFIFY_SUCCESS,

  RC_VEFIFY_ERROR_DUPLICATE_PRIMARY_BACKUP,

  RC_VEFIFY_ERROR_DUPLICATE_PRIMARY_NOT_BALANCED,

  RC_VEFIFY_ERROR_MIGRATE_COUNT,

};

struct VerifyResult {
  double timeTaken;
  size_t migrateCount;
  size_t backupMaxMinDeltaCount;
  std::shared_ptr<pb::PartitionTable> partitionTable;
  std::shared_ptr<pb::DeltaPartitionEvent> event;
};

struct TotalResult {
  size_t memberSize;
  size_t count;

  double maxTimeTaken;
  double minTimeTaken;
  double sumTimeTaken;
  double avgTimeTaken;

  size_t maxMigrateCount;
  size_t minMigrateCount;
  size_t sumMigrateCount;
  size_t avgMigrateCount;

  std::shared_ptr<VerifyResult> minResult;
  std::shared_ptr<VerifyResult> maxResult;

  ::std::map<size_t, size_t> deltaBackupCount;
};

class PartitiontableBalanceVerifier {

public:

  static VerifyErrCode verify(MembershipTableMgr& memberMgr, PartitionTableMgr& partitionMgr, VerifyResult& rs,
      const size_t expectMigrateCount, pb::DeltaPartitionEvent& evt);

  static void total(std::shared_ptr<VerifyResult>& prs, VerifyResult& rs, size_t activeMemberSize,
      std::map<size_t, TotalResult>& joinedTotalResult);

  static size_t count(std::map<size_t, TotalResult>& totalResult);

  static void display(std::map<size_t, TotalResult>& joinedTotalResult, std::map<size_t, TotalResult>& leftTotalResult,
      size_t LOOP_COUNT, size_t MAX_MEMBER_SIZE, size_t MIN_MEMBER_SIZE, size_t partitionCount, size_t maxBackupCount);

};
}
}

