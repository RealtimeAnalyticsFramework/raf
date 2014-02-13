
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


using namespace idgs;
using namespace idgs::cluster;
using namespace idgs::pb;


TEST(cluster_cfg_parser_env_test, parse_env) {
  ClusterConfig config;
  ResultCode rs = ClusterCfgParser::parse(config, "framework/conf/cluster.conf");
  EXPECT_EQ(RC_SUCCESS, rs);
  EXPECT_EQ(5, config.thread_count());
  EXPECT_EQ(5, config.io_thread_count());
  EXPECT_EQ("202.106.46.151", config.member().publicaddress().host());
  EXPECT_EQ(10086, config.member().publicaddress().port());
  EXPECT_EQ("192.168.0.254", config.member().inneraddress().host());
  EXPECT_EQ(10010, config.member().inneraddress().port());
  EXPECT_EQ(32, config.member().weight());
  EXPECT_FALSE(config.member().service().local_store());
  EXPECT_FALSE(config.member().service().client_agent());
  EXPECT_FALSE(config.member().service().dist_computing());
  EXPECT_FALSE(config.member().service().admin_console());

  for(auto it = config.modules().begin(); it != config.modules().end(); ++it) {
    LOG(INFO) << it->DebugString();
    LOG(INFO) << "=================================";
  }

}
