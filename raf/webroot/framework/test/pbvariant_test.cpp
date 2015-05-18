
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "gtest/gtest.h"
#include "idgs/idgslogging.h"

#include "protobuf/pbvariant.h"

using namespace protobuf;
using namespace std;

TEST(pbvariant, init) {
  PbVariant v;
  LOG(INFO) << sizeof(v);
  LOG(INFO) << sizeof(v.value);
  LOG(INFO) << sizeof(google::protobuf::FieldDescriptor::CppType);
  LOG(INFO) << sizeof(std::string);

  LOG(INFO) << v.value.v_int32;
  LOG(INFO) << v.value.v_int64;
  LOG(INFO) << v.value.v_string;

  v = string("hello");
  LOG(INFO) << v.value.v_int32;
  LOG(INFO) << v.value.v_int64;
  LOG(INFO) << v.value.v_string;

  PbVariant v1;
  v1 = 12345;
  LOG(INFO) << v1.value.v_int32;
}

TEST(pbvariant, cast) {
  PbVariant v = string("1234");
  LOG(INFO) << ((int32_t)v);
  LOG(INFO) << ((int32_t)v) + 4321;
}



