
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include <gtest/gtest.h>
#include "idgs/client/client_pool.h"

using namespace idgs::client;
using namespace idgs;

TEST(client_pool_it, test_four_client) {

  LOG(INFO) << "Test Get 5 Clients from client pool, available server count 3";

  ClientSetting setting;
  setting.clientConfig = "integration_test/client_it/test_client.conf";

  auto& pool = getTcpClientPool();

  ResultCode code = pool.loadConfig(setting);
  EXPECT_EQ(RC_SUCCESS, code);

  auto client1 = pool.getTcpClient(code);
  EXPECT_EQ(RC_SUCCESS, code);
//  EXPECT_EQ("7700", client1->getServerAddress().port());

  auto client2 = pool.getTcpClient(code);
  EXPECT_EQ(RC_SUCCESS, code);
//  EXPECT_EQ("7701", client2->getServerAddress().port());

  auto client3 = pool.getTcpClient(code);
  EXPECT_EQ(RC_SUCCESS, code);
//  EXPECT_EQ("7702", client3->getServerAddress().port());

  auto client4 = pool.getTcpClient(code);
  EXPECT_EQ(RC_SUCCESS, code);
//  EXPECT_EQ("7700", client4->getServerAddress().port());

  auto client5 = pool.getTcpClient(code);
  EXPECT_EQ(RC_SUCCESS, code);
//  EXPECT_EQ("7701", client5->getServerAddress().port());

  /// pool has been empty, size = 0
  EXPECT_EQ(0, pool.size());

  auto client6 = pool.getTcpClient(code);
  EXPECT_EQ(RC_ERROR, code);

  /// push back client1 to the pool, size = 1
  client1->close();
  EXPECT_EQ(1, pool.size());

  /// get recycled client again.
  client6 = pool.getTcpClient(code);
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("7700", client6->getServerEndpoint().port());
  EXPECT_EQ(0, pool.size());

  client2->close();
  EXPECT_EQ(1, pool.size());
  client3->close();
  EXPECT_EQ(2, pool.size());
  client4->close();
  EXPECT_EQ(3, pool.size());
  client5->close();
  EXPECT_EQ(4, pool.size());
  client6->close();
  EXPECT_EQ(5, pool.size());

  /// display
  pool.toString();

  /// close pool
  pool.close();
}

