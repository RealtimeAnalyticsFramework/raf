
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/application.h"
#include <gtest/gtest.h>
#include "idgs/signal_handler.h"
using namespace idgs;
using namespace idgs::cluster;
using namespace std;

static int32_t EMPTY_PARTITION_TABLE[][2] =
{
      { -1, -1},
      { -1, -1},
      { -1, -1},
      { -1, -1},
      { -1, -1},
      { -1, -1},
      { -1, -1},
      { -1, -1},
      { -1, -1},
      { -1, -1},
      { -1, -1},
      { -1, -1},
      { -1, -1},
      { -1, -1},
      { -1, -1},
      { -1, -1},
      { -1, -1}
};
static int32_t PARTITION_TABLE_WITH_0[][2] =
{
      { 0, -1},
      { 0, -1},
      { 0, -1},
      { 0, -1},
      { 0, -1},
      { 0, -1},
      { 0, -1},
      { 0, -1},
      { 0, -1},
      { 0, -1},
      { 0, -1},
      { 0, -1},
      { 0, -1},
      { 0, -1},
      { 0, -1},
      { 0, -1},
      { 0, -1}
};
static int32_t PARTITION_TABLE_WITH_1_0[][2] =
{
      { 1, 0},
      { 1, 0},
      { 1, 0},
      { 1, 0},
      { 1, 0},
      { 1, 0},
      { 1, 0},
      { 1, 0},
      { 0, 1},
      { 0, 1},
      { 0, 1},
      { 0, 1},
      { 0, 1},
      { 0, 1},
      { 0, 1},
      { 0, 1},
      { 0, 1}
};
static int32_t PARTITION_TABLE_WITH_0_1[][2] =
{
      { 0, 1},
      { 0, 1},
      { 0, 1},
      { 0, 1},
      { 0, 1},
      { 0, 1},
      { 0, 1},
      { 0, 1},
      { 0, 1},
      { 1, 0},
      { 1, 0},
      { 1, 0},
      { 1, 0},
      { 1, 0},
      { 1, 0},
      { 1, 0},
      { 1, 0}
};
static int32_t PARTITION_TABLE_WITH_1[][2] =
{
      { 1, -1},
      { 1, -1},
      { 1, -1},
      { 1, -1},
      { 1, -1},
      { 1, -1},
      { 1, -1},
      { 1, -1},
      { 1, -1},
      { 1, -1},
      { 1, -1},
      { 1, -1},
      { 1, -1},
      { 1, -1},
      { 1, -1},
      { 1, -1},
      { 1, -1}
};

static void check(int step);
static void checkStep1(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table);
static void checkStep2(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table);
static void checkStep3(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table);
static void checkStep4(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table);
static void checkStep5(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table);
static void checkStep6(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table);
static void checkStep7(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table);
static void checkStep8(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table);
static void checkStep9(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table);
static void checkStep10(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table);

static void checkMembershipTable(const std::vector<int32_t>& expect_table, const std::vector<MemberWrapper>& actual_table);
static void displayMembershipTable(const std::vector<int32_t>& expect_table, const std::vector<MemberWrapper>& actual_table);
static void toVector(std::vector<std::vector<int32_t>>& vct, int32_t array[][2], const int array_len);
static bool checkPartitionTable(const std::vector<std::vector<int32_t>>& expect_table, const std::vector<PartitionWrapper>& actual_table);
static void displayPartitionTable(const std::vector<std::vector<int32_t>>& expect_table, const std::vector<PartitionWrapper>& actual_table);

namespace {
  struct ApplicationSetting {
    ApplicationSetting():clusterConfig("") {}
    std::string clusterConfig;
  };
}

void startServer() {
  ApplicationSetting setting;
  setting.clusterConfig = "framework/conf/cluster.conf";

  Application& app = ::idgs::util::singleton<Application>::getInstance();
  ResultCode rc;
  rc = app.init(setting.clusterConfig);
  if (rc != RC_SUCCESS) {
    LOG(ERROR) << "Failed to initialize server: " << getErrorDescription(rc);
    exit(1);
  }
  rc = app.start();
  if (rc != RC_SUCCESS) {
    LOG(ERROR) << "Failed to start server: " << getErrorDescription(rc);
    exit(1);
  }
  SignalHandler sh;
  sh.setup();
}


TEST(cluster_it, cluster_it) {
	startServer();
	sleep(5);
	int step = -1;
	char *str_step = getenv("STEP");
	if(str_step) {
		step = atoi(str_step);
	}
	// begin to check
	check(step);
	// stop it
	::idgs::util::singleton<Application>::getInstance().stop();
}

void check(int step) {
	ClusterFramework& cluster = ::idgs::util::singleton<ClusterFramework>::getInstance();
	const std::vector<MemberWrapper>& member_table = cluster.getMemberManager()->getMemberTable();
	const std::vector<PartitionWrapper>& partition_table = cluster.getPartitionManager()->getPartitionTable();

	typedef void (*CHECK_FUNC)(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table);

	CHECK_FUNC funcs[] = { checkStep1, checkStep2, checkStep3, checkStep4, checkStep5,
	    checkStep6, checkStep7, checkStep8, checkStep9, checkStep10
	};

	(*(funcs[step - 1]))(member_table, partition_table);

}

void displayMembershipTable(const std::vector<int32_t>& expect_table, const std::vector<MemberWrapper>& actual_table) {
  DVLOG(1) << "#################### Actual membership table #####################";
  for(auto it = actual_table.begin(); it != actual_table.end(); ++it) {
    DVLOG(1) << it->getId();
  }
  DVLOG(1) << "#################### Expect membership table #####################";
  for(auto it = expect_table.begin(); it != expect_table.end(); ++it) {
    DVLOG(1) << *it;
  }
}

void checkMembershipTable(const std::vector<int32_t>& expect_table, const std::vector<MemberWrapper>& actual_table) {
  bool flag = (expect_table.size() == actual_table.size());
  if(!flag) { // if check invalid, print expect table & actual table
    displayMembershipTable(expect_table, actual_table);
  }
  ASSERT_TRUE(flag);
  auto expect_it = expect_table.begin();
  for(auto it = actual_table.begin(); it != actual_table.end() && expect_it != expect_table.end(); ++it, ++expect_it) {
    bool flag = (it->getId() == *expect_it);
    if(!flag) { // if check invalid, print expect table & actual table
      displayMembershipTable(expect_table, actual_table);
    }
    ASSERT_TRUE(flag);
  }
}

void displayPartitionTable(const std::vector<std::vector<int32_t> >& expect_table, const std::vector<PartitionWrapper>& actual_table) {
  DVLOG(1) << "#################### Actual partition table #####################";
  for(auto it = actual_table.begin(); it != actual_table.end(); ++it) {
    stringstream str;
    str << it->getMemberId(0) << " | ";
    for(uint8_t i = 1, backups = it->getBackupNodes(); i <= backups; ++i) {
      str << it->getMemberId(i) << " | ";
    }
    DVLOG(1) << str.str();
  }
  DVLOG(1) << "#################### Expect partition table #####################";
  for(auto it = expect_table.begin(); it != expect_table.end(); ++it) {
    stringstream str;
    for(size_t j = 0; j < it->size(); ++j) {
      str << it->at(j) << " | ";
    }
    DVLOG(1) << str.str();
  }
}

void toVector(std::vector<std::vector<int32_t>>& vct, int array[][2], const int array_len) {
  for (int i = 0, size = array_len; i < size; ++i) {
    std::vector<int32_t> element;
    element.push_back(array[i][0]);
    element.push_back(array[i][1]);
    vct.push_back(element);
  }
}

bool checkPartitionTable(const std::vector<std::vector<int32_t>>& expect_table, const std::vector<PartitionWrapper>& actual_table) {
  auto expect_it = expect_table.begin();
  for(auto it = actual_table.begin(); it != actual_table.end() && expect_it!=expect_table.end(); ++it, ++expect_it) {
    bool flag = (it->getMemberId(0) == expect_it->at(0));
    if(!flag) {
      return false;
    }
    for(uint8_t i = 1, backups = it->getBackupNodes(); i <= backups; ++i) {
      flag = (it->getMemberId(i) == expect_it->at(i));
      if(!flag) {
        return false;
      }
    }
  }
  return true;
}

void checkStep1(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table) {
  function_footprint();
	VLOG(1) << "############################## step 1: start server 1(local store) ################################";
	// check leading
	const MemberWrapper& leading = member_table[0];
	ASSERT_TRUE(leading.isLeading());
	// initialize expect membership table & partition_table
	std::vector<int32_t> expect_member_table = {0, 1};
	// check membership table
	checkMembershipTable(expect_member_table, member_table);

	// check partition table
	std::vector<std::vector<int32_t>> expect_partition_table;
	toVector(expect_partition_table, PARTITION_TABLE_WITH_0, partition_table.size());
	bool flag = checkPartitionTable(expect_partition_table, partition_table);
	if(!flag) {
	  displayPartitionTable(expect_partition_table, partition_table);
	}
	ASSERT_TRUE(flag);
}

void checkStep2(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table) {
	VLOG(1) << "############################## step 2: start server 2(local store) ################################";
	// check leading
  const MemberWrapper& leading = member_table[0];
  ASSERT_TRUE(leading.isLeading());
  // initialize expect membership table & partition_table
  std::vector<int32_t> expect_member_table = {0, 1, 2};
  // check membership table
  checkMembershipTable(expect_member_table, member_table);

  std::vector<std::vector<int32_t>> expect_partition_table;
  toVector(expect_partition_table, PARTITION_TABLE_WITH_1_0, partition_table.size());
  // check partition table
  bool flag = checkPartitionTable(expect_partition_table, partition_table);
  if(!flag) {
    displayPartitionTable(expect_partition_table, partition_table);
  }
  ASSERT_TRUE(flag);
}

void checkStep3(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table) {
	VLOG(1) << "############################## step 3: start server 3(local store) ################################";
	// check leading
  const MemberWrapper& leading = member_table[0];
  ASSERT_TRUE(leading.isLeading());
  // initialize expect membership table & partition_table
  std::vector<int32_t> expect_member_table = {0, 1, 2, 3};
  // check membership table
  checkMembershipTable(expect_member_table, member_table);

  std::vector<std::vector<int32_t>> expect_partition_table;
  toVector(expect_partition_table, PARTITION_TABLE_WITH_1_0, partition_table.size());
  // check partition table
  bool flag = checkPartitionTable(expect_partition_table, partition_table);
  if(!flag) {
    displayPartitionTable(expect_partition_table, partition_table);
  }
  ASSERT_TRUE(flag);
}

void checkStep4(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table) {
	VLOG(1) << "############################## step 4: kill server 1(local store, leading) ################################";
	// check leading
  const MemberWrapper& leading = member_table[1];
  ASSERT_TRUE(leading.isLeading());
  // initialize expect membership table & partition_table
  std::vector<int32_t> expect_member_table = {0, 1, 2, 3};
  // check membership table
  checkMembershipTable(expect_member_table, member_table);

  std::vector<std::vector<int32_t>> expect_partition_table;
  toVector(expect_partition_table, PARTITION_TABLE_WITH_1, partition_table.size());
  // check partition table
  bool flag = checkPartitionTable(expect_partition_table, partition_table);
  if(!flag) {
    displayPartitionTable(expect_partition_table, partition_table);
  }
  ASSERT_TRUE(flag);
}

void checkStep5(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table) {
	VLOG(1) << "############################## step 5: kill server 2(local store, leading) ################################";
	// check leading
  const MemberWrapper& leading = member_table[2];
  ASSERT_TRUE(leading.isLeading());
  // initialize expect membership table & partition_table
  std::vector<int32_t> expect_member_table = {0, 1, 2, 3};
  // check membership table
  checkMembershipTable(expect_member_table, member_table);

  std::vector<std::vector<int32_t>> expect_partition_table;
  toVector(expect_partition_table, EMPTY_PARTITION_TABLE, partition_table.size());
  // check partition table
  bool flag = checkPartitionTable(expect_partition_table, partition_table);
  if(!flag) {
    displayPartitionTable(expect_partition_table, partition_table);
  }
  ASSERT_TRUE(flag);
}

void checkStep6(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table) {
	VLOG(1) << "############################## step 6: start server 1, 2(local store) at the same time ################################";
	// check leading
  const MemberWrapper& leading = member_table[2];
  ASSERT_TRUE(leading.isLeading());
  // initialize expect membership table & partition_table
  std::vector<int32_t> expect_member_table = {0, 1, 2, 3};
  // check membership table
  checkMembershipTable(expect_member_table, member_table);

  std::vector<std::vector<int32_t>> expect_partition_table1;
  toVector(expect_partition_table1, PARTITION_TABLE_WITH_1_0, partition_table.size());
  // check partition table
  bool flag = checkPartitionTable(expect_partition_table1, partition_table);
  if(flag) {
    return;
  }
  // possible result 2
  std::vector<std::vector<int32_t>> expect_partition_table2;
  toVector(expect_partition_table1, PARTITION_TABLE_WITH_0_1, partition_table.size());
  flag = checkPartitionTable(expect_partition_table2, partition_table);
  if(!flag) {
    displayPartitionTable(expect_partition_table1, partition_table);
    DVLOG(1) << "####################### Another possible result ###########################";
    displayPartitionTable(expect_partition_table2, partition_table);
  }
  ASSERT_TRUE(flag);
}

void checkStep7(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table) {
	VLOG(1) << "############################## step 7: kill server 1, 2(local store, without leading) ################################";
	// check leading
  const MemberWrapper& leading = member_table[2];
  ASSERT_TRUE(leading.isLeading());
  // initialize expect membership table & partition_table
  std::vector<int32_t> expect_member_table = {0, 1, 2, 3};
  // check membership table
  checkMembershipTable(expect_member_table, member_table);

  std::vector<std::vector<int32_t>> expect_partition_table;
  toVector(expect_partition_table, EMPTY_PARTITION_TABLE, partition_table.size());
  // check partition table
  bool flag = checkPartitionTable(expect_partition_table, partition_table);
  if(!flag) {
    displayPartitionTable(expect_partition_table, partition_table);
  }
  ASSERT_TRUE(flag);
}

void checkStep8(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table) {
  DVLOG(1) << "############################## step 8: start server 1, 2(local store) at the same time ################################";
  // check leading
  const MemberWrapper& leading = member_table[2];
  ASSERT_TRUE(leading.isLeading());
  // initialize expect membership table & partition_table
  std::vector<int32_t> expect_member_table = {0, 1, 2, 3};
  // check membership table
  checkMembershipTable(expect_member_table, member_table);

  std::vector<std::vector<int32_t>> expect_partition_table1;
  toVector(expect_partition_table1, PARTITION_TABLE_WITH_1_0, partition_table.size());
  // check partition table
  bool flag = checkPartitionTable(expect_partition_table1, partition_table);
  if(flag) {
    return;
  }
  // possible result 2
  std::vector<std::vector<int32_t>> expect_partition_table2;
  toVector(expect_partition_table1, PARTITION_TABLE_WITH_0_1, partition_table.size());
  flag = checkPartitionTable(expect_partition_table2, partition_table);
  if(!flag) {
    displayPartitionTable(expect_partition_table1, partition_table);
    DVLOG(1) << "####################### Another possible result ###########################";
    displayPartitionTable(expect_partition_table2, partition_table);
  }
  ASSERT_TRUE(flag);
}

void checkStep9(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table) {
	VLOG(1) << "############################## step 9: kill server 3(not local store, leading), 1(local store, possible selected leading) at the same time ################################";
	// check leading
  const MemberWrapper& leading = member_table[0].isLeading() ? member_table[0] : member_table[1];
  ASSERT_TRUE(leading.isLeading());
  // initialize expect membership table & partition_table
  std::vector<int32_t> expect_member_table = {0, 1, 2, 3};
  // check membership table
  checkMembershipTable(expect_member_table, member_table);

  std::vector<std::vector<int32_t>> expect_partition_table1;
  toVector(expect_partition_table1, PARTITION_TABLE_WITH_1, partition_table.size());
  // check partition table
  bool flag = checkPartitionTable(expect_partition_table1, partition_table);
  if(flag) {
    return;
  }
  // possible result 2
  std::vector<std::vector<int32_t>> expect_partition_table2;
  toVector(expect_partition_table1, PARTITION_TABLE_WITH_0, partition_table.size());
  flag = checkPartitionTable(expect_partition_table2, partition_table);
  if(!flag) {
    displayPartitionTable(expect_partition_table1, partition_table);
    DVLOG(1) << "####################### Another possible result ###########################";
    displayPartitionTable(expect_partition_table2, partition_table);
  }
  ASSERT_TRUE(flag);
}

void checkStep10(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table) {
	VLOG(1) << "############################## step 10: kill server 2(local store, leading) ################################";
	// check leading
  const MemberWrapper& leading = member_table[0];
  ASSERT_TRUE(leading.isLeading());
  // initialize expect membership table & partition_table
  std::vector<int32_t> expect_member_table = {0};
  // check membership table
  checkMembershipTable(expect_member_table, member_table);

  std::vector<std::vector<int32_t>> expect_partition_table;
  toVector(expect_partition_table, EMPTY_PARTITION_TABLE, partition_table.size());
  // check partition table
  bool flag = checkPartitionTable(expect_partition_table, partition_table);
  if(!flag) {
    displayPartitionTable(expect_partition_table, partition_table);
  }
  ASSERT_TRUE(flag);
}
