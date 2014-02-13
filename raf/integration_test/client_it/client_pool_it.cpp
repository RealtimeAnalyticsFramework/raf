
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

TEST(client, test_four_client) {

  LOG(INFO) << "Test Get 5 Clients from client pool, available server count 3";

  ClientSetting setting;
  setting.clientConfig = "integration_test/client_it/ut_test_four_clients.conf";

  ResultCode code = ::idgs::util::singleton<TcpClientPool>::getInstance().loadConfig(setting);
  EXPECT_EQ(RC_SUCCESS, code);

  std::shared_ptr<TcpClientInterface> client1;
  client1 = ::idgs::util::singleton<TcpClientPool>::getInstance().getTcpClient(code);
  EXPECT_EQ(RC_SUCCESS, code);
//  EXPECT_EQ("7700", client1->getServerAddress().port());

  std::shared_ptr<TcpClientInterface> client2;
  client2 = ::idgs::util::singleton<TcpClientPool>::getInstance().getTcpClient(code);
  EXPECT_EQ(RC_SUCCESS, code);
//  EXPECT_EQ("7701", client2->getServerAddress().port());

  std::shared_ptr<TcpClientInterface> client3;
  client3 = ::idgs::util::singleton<TcpClientPool>::getInstance().getTcpClient(code);
  EXPECT_EQ(RC_SUCCESS, code);
//  EXPECT_EQ("7702", client3->getServerAddress().port());

  std::shared_ptr<TcpClientInterface> client4;
  client4 = ::idgs::util::singleton<TcpClientPool>::getInstance().getTcpClient(code);
  EXPECT_EQ(RC_SUCCESS, code);
//  EXPECT_EQ("7700", client4->getServerAddress().port());

  std::shared_ptr<TcpClientInterface> client5;
  client5 = ::idgs::util::singleton<TcpClientPool>::getInstance().getTcpClient(code);
  EXPECT_EQ(RC_SUCCESS, code);
//  EXPECT_EQ("7701", client5->getServerAddress().port());

  /// pool has been empty, size = 0
  EXPECT_EQ(0, ::idgs::util::singleton<TcpClientPool>::getInstance().size());

  std::shared_ptr<TcpClientInterface> client6;
  client6 = ::idgs::util::singleton<TcpClientPool>::getInstance().getTcpClient(code);
  EXPECT_EQ(RC_ERROR, code);

  /// push back client1 to the pool, size = 1
  client1->close();
  EXPECT_EQ(1, ::idgs::util::singleton<TcpClientPool>::getInstance().size());

  /// get recycled client again.
  client6 = ::idgs::util::singleton<TcpClientPool>::getInstance().getTcpClient(code);
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("7700", client6->getServerAddress().port());
  EXPECT_EQ(0, ::idgs::util::singleton<TcpClientPool>::getInstance().size());

  client2->close();
  EXPECT_EQ(1, ::idgs::util::singleton<TcpClientPool>::getInstance().size());
  client3->close();
  EXPECT_EQ(2, ::idgs::util::singleton<TcpClientPool>::getInstance().size());
  client4->close();
  EXPECT_EQ(3, ::idgs::util::singleton<TcpClientPool>::getInstance().size());
  client5->close();
  EXPECT_EQ(4, ::idgs::util::singleton<TcpClientPool>::getInstance().size());
  client6->close();
  EXPECT_EQ(5, ::idgs::util::singleton<TcpClientPool>::getInstance().size());

  /// display
  ::idgs::util::singleton<TcpClientPool>::getInstance().toString();

  /// close pool
  ::idgs::util::singleton<TcpClientPool>::getInstance().close();
}

