
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

/// cluster integration test ///

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
static void checkStep11(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table);

static void displayMembershipTable(const std::vector<MemberWrapper>& actual_table);

static void displayPartitionTable(const std::vector<PartitionWrapper>& actual_table);

namespace {
  struct ApplicationSetting {
    ApplicationSetting():clusterConfig("") {}
    std::string clusterConfig;
  };
}

void startServer() {
  ApplicationSetting setting;
  setting.clusterConfig = "conf/cluster.conf";

  Application& app = * idgs_application();
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

	// wait for member/partition event
	sleep(1);


	int step = -1;
	char *str_step = getenv("STEP");
	if(str_step) {
		step = atoi(str_step);
	}
	// begin to check
	check(step);
	// stop it
	idgs_application()->stop();
}

void check(int step) {
	auto cluster = idgs_application()->getClusterFramework();
	const std::vector<MemberWrapper>& member_table = cluster->getMemberManager()->getMemberTable();
	const std::vector<PartitionWrapper>& partition_table = cluster->getPartitionManager()->getPartitionTable();

	typedef void (*CHECK_FUNC)(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table);

	CHECK_FUNC funcs[] = { checkStep1, checkStep2, checkStep3, checkStep4, checkStep5,
	    checkStep6, checkStep7, checkStep8, checkStep9, checkStep10, checkStep11
	};

	displayMembershipTable(member_table);
  displayPartitionTable(partition_table);

	(*(funcs[step - 1]))(member_table, partition_table);

}

void displayMembershipTable(const std::vector<MemberWrapper>& actual_table) {
  DVLOG(1) << "#################### Actual membership table #####################";
  for(auto it = actual_table.begin(); it != actual_table.end(); ++it) {
    DVLOG(1) << *it;
  }
}

void displayPartitionTable(const std::vector<PartitionWrapper>& actual_table) {
  DVLOG(1) << "#################### Actual partition table #####################";
  for(auto it = actual_table.begin(); it != actual_table.end(); ++it) {
    stringstream str;
    str << it->getMemberId(0) << " | ";
    for(uint8_t i = 1, backups = it->getPartition().cells_size(); i < backups; ++i) {
      str << it->getMemberId(i) << " | ";
    }
    DVLOG(1) << str.str();
  }
}

void checkStep1(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table) {
  function_footprint();
	DVLOG(1) << "step 1: start server 1(local store), itself selected as leading";
}

void checkStep2(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table) {
  function_footprint();
  DVLOG(1) << "step 2: start server 2(local store), server 1 is leading";
}

void checkStep3(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table) {
  DVLOG(1) << "step 3: kill server 2(local store), server 1 is leading";
}

void checkStep4(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table) {
	DVLOG(1) << "step 4: start server 2(local store) again, server 1 is leading";
}

void checkStep5(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table) {
	DVLOG(1) << "step 5: kill server 1(local store, leading), server 2 selected as new leading";
}

void checkStep6(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table) {
	DVLOG(1) << "step 6: start server 1(local store) server 3(not local store) at the same time, server 2 is leading";
}

void checkStep7(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table) {
	DVLOG(1) << "step 7: kill server 1(local store), server 3(not local store) at the same time";
}

void checkStep8(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table) {
  DVLOG(1) << "step 8: start server 1(local store) server 3(not local store) at the same time again, server 2 is leading";
}

void checkStep9(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table) {
  DVLOG(1) << "step 9: kill server 1(local store) server 2(local store, leading) at the same time, server 3(not local store) selected as new leading";
}

void checkStep10(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table) {
  DVLOG(1) << "step 10: start server 1(local store) server 2(local store) at the same time again, server 3 is leading";
}

void checkStep11(const std::vector<MemberWrapper>& member_table, const std::vector<PartitionWrapper>& partition_table) {
  DVLOG(1) << "step 10: kill server 1,2,3 at the same time, only exist test client, itself is leading";
}
