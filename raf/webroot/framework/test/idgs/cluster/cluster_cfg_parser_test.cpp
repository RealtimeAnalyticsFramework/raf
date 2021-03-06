
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include <gtest/gtest.h>
#include "idgs/cluster/cluster_cfg_parser.h"
#include "protobuf/pb_serdes.h"

using namespace idgs;
using namespace idgs::cluster;
using namespace idgs::pb;
using namespace std;

TEST(ClusterCfg, defaultValue) {
  ClusterConfig config;
  DVLOG(1) << config.partition_count();
  DVLOG(1) << config.max_replica_count();
}

TEST(ClusterCfg, parse) {
  ClusterConfig config;
  ResultCode rs = ClusterCfgParser::parse(config, "conf/cluster.conf");
  DVLOG(1) << rs;

//  ASSERT_EQ(7, config.thread_count());
//  ASSERT_EQ(17, config.partition_count());
//  ASSERT_EQ("g1", config.member().group_name());

  string text = config.DebugString();
  DVLOG(1) << text;

  ClusterConfig config2;

  protobuf::ProtoSerdesHelper::deserialize(protobuf::PB_TEXT, text, &config2);
  protobuf::ProtoSerdesHelper::serialize(protobuf::PB_TEXT, &config2, &text);
  DVLOG(1) << text;
}
