
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include <gtest/gtest.h>
#include "idgs/cluster/cluster_framework.h"

using namespace idgs::cluster;
using namespace idgs::pb;

TEST(member_wrapper, copyConstruction) {
	MemberWrapper member;
	Member info;
	member.setMember(info);
  member.setPartitionCount(0, 17);
  member.setPartitionCount(1, 2 * member.getPartition(0));

  //copy construction
  MemberWrapper m1 = member;
  ASSERT_EQ(17, m1.getPartition(0));
  ASSERT_EQ(34, m1.getPartition(1));

  //operator=
  MemberWrapper m2;
  m2 = member;

  ASSERT_EQ(17, m2.getPartition(0));
  ASSERT_EQ(34, m2.getPartition(1));

}

TEST(member_wrapper, toString) {
  MemberWrapper member;
  Member info;
  member.setMember(info);
  member.setPartitionCount(0, 17);
  member.setPartitionCount(1, 2 * member.getPartition(0));
  std::cerr << member.toString();
}


