
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "idgs/cluster/cluster_framework.h"
#include <gtest/gtest.h>


using namespace idgs::cluster;
using namespace idgs::pb;


// init
TEST(membershiptable_mgr_test, init) {
  const int max_reserved_size = 10;
  MembershipTableMgr member_manager;
  ClusterConfig cfg;
  cfg.set_reserved_member_size(max_reserved_size);
  member_manager.init(&cfg);
}
// addMember
TEST(membershiptable_mgr_test, addMember) {
  const int max_reserved_size = 10;
  MembershipTableMgr member_manager;
  ClusterConfig cfg;
  cfg.set_reserved_member_size(max_reserved_size);
  member_manager.init(&cfg);
  for (int i = 0; i < max_reserved_size; ++i) {
    MemberWrapper member;
    Member info;
    member.setMember(info);
    member.setId(i);
    member.setNodeId(i);
    member.setPid(1000 + i);
    member.setIsleading(i == 0);
    member_manager.addMember(member);
  }
}

// get member
TEST(membershiptable_mgr_test, getMember) {
  const int max_reserved_size = 10;
  MembershipTableMgr member_manager;
  ClusterConfig cfg;
  cfg.set_reserved_member_size(max_reserved_size);
  member_manager.init(&cfg);
  for (int i = 0; i < max_reserved_size; ++i) {
    MemberWrapper member;
    Member info;
    member.setMember(info);
    member.setId(i);
    member.setNodeId(i);
    member.setPid(1000 + i);
    member.setIsleading(i == 0);
    member_manager.addMember(member);
  }
  ASSERT_TRUE(member_manager.getMember(0));
}

// findMember
TEST(membershiptable_mgr_test, findMember) {
  const int max_reserved_size = 10;
  MembershipTableMgr member_manager;
  ClusterConfig cfg;
  cfg.set_reserved_member_size(max_reserved_size);
  member_manager.init(&cfg);

  for (int i = 0; i < max_reserved_size; ++i) {
    MemberWrapper member;
    Member info;
    member.setMember(info);
    member.setId(i);
    member.setNodeId(i);
    member.setPid(1000 + i);
    member.setIsleading(i == 0);
    member_manager.addMember(member);
  }
  ASSERT_TRUE(member_manager.findMember(3, 1003));
}

// genMembershipTable
TEST(membershiptable_mgr_test, genMembershipTable) {
  const int max_reserved_size = 10;
  MembershipTableMgr member_manager;
  ClusterConfig cfg;
  cfg.set_reserved_member_size(max_reserved_size);
  member_manager.init(&cfg);
  for (int i = 0; i < max_reserved_size; ++i) {
    MemberWrapper member;
    Member info;
    member.setMember(info);
    member.setId(i);
    member.setNodeId(i);
    member.setPid(1000 + i);
    member.setIsleading(i == 0);
    member_manager.addMember(member);
  }
  MembershipTable mt;
  member_manager.genMembershipTable(mt);

  unsigned int seed = clock();
  DVLOG(1) << "seed = " << seed;
  srand(seed);
  int random = rand() % max_reserved_size;
  ASSERT_EQ(member_manager.getMember(random)->getId(), mt.member(random).id());
  DVLOG(1) << mt.DebugString();
}

// genMembershipTable
TEST(membershiptable_mgr_test, toString) {
  const int max_reserved_size = 10;
  MembershipTableMgr member_manager;
  ClusterConfig cfg;
  cfg.set_reserved_member_size(max_reserved_size);
  member_manager.init(&cfg);
  for (int i = 0; i < max_reserved_size; ++i) {
    MemberWrapper member;
    Member info;
    member.setMember(info);
    member.setId(i);
    member.setNodeId(i);
    member.setPid(1000 + i);
    member.setIsleading(i == 0);
    member_manager.addMember(member);
  }
  DVLOG(1) << std::endl << member_manager.toString();
}



