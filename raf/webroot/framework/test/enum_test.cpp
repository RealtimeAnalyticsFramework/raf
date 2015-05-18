/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "idgs/util/enum_def.h"
#include "idgs/idgslogging.h"
#include "gtest/gtest.h"

DEF_ENUM(Number,
    ONE=1, // comment1
    /*comment2*/TWO,
    THREE,
    THREE2=3,
    FIVE=5
 );

TEST(ENUM, parse) {
  auto nvmap = idgs::util::parseEnumBody("ONE = -1, TWO = 2, THREE");
  for( auto& p: nvmap) {
    LOG(INFO) << p.first << "=" << p.second;
  }
}

TEST(ENUM, name2index) {
  Number v = ONE;
  LOG(INFO) << StringToNumber("ONE", &v);
  LOG(INFO) << v;
  LOG(INFO) << StringToNumber("TWO", &v);
  LOG(INFO) << v;
  LOG(INFO) << StringToNumber("THREE", &v);
  LOG(INFO) << v;
  LOG(INFO) << StringToNumber("THREE2", &v);
  LOG(INFO) << v;
  LOG(INFO) << StringToNumber("FOUR", &v);
  LOG(INFO) << StringToNumber("FIVE", &v);
  LOG(INFO) << v;
}

TEST(ENUM, index2name) {
  LOG(INFO) << NumberToString(ONE);
  LOG(INFO) << NumberToString(TWO);
  LOG(INFO) << NumberToString(THREE);
}

#define VALOG(...) \
  LOG(INFO) << #__VA_ARGS__;

TEST(ENUM, valog) {
  VALOG(ONE, // comment1
      /*comment2*/TWO,,
      THREE);
}
