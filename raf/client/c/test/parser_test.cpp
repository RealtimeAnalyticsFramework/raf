
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include <gtest/gtest.h>
#include "idgs/client/command_parser.h"
#include "idgs/store/data_store.h"

using namespace idgs::client;
using namespace idgs;
using namespace idgs::store;
using namespace std;

TEST(parser, parseToCmd) {

  std::string storeConfig = "services/store/test/data_store.conf";
  ResultCode code = ::idgs::util::singleton<DataStore>::getInstance().initialize(storeConfig);

  //string insert_cmd = "store.service insert {\"store_name\":\"Customer\"} key={\"c_custkey\":\"234000\"} value={\"c_name\":\"Tom0\",\"c_nationkey\":\"10\",\"c_phone\":\"13500000000\"}";
  string insert_cmd = "store.service insert {\"store_name\":\"Orders\"} key={\"o_orderkey\":\"100000\"} value={\"o_custkey\":\"234000\",\"o_orderstatus\":\"2\",\"o_totalprice\":\"200.55\",\"o_orderdate\":\"2013-02-01\"}";

  CommandParser parser;
  Command command;
  code = parser.parse(insert_cmd, &command);
  ASSERT_EQ(RC_SUCCESS, code);
  ASSERT_EQ("store.service", command.actorId);
  ASSERT_EQ("insert", command.opName);
  ASSERT_EQ("{\"store_name\":\"Orders\"}", command.payload);
/*  for(auto it = command.attachments.begin(); it!=command.attachments.end(); ++it) {
    LOG(INFO) << "get Attachment " << it->first << " | " << it->second;
  }*/
}
