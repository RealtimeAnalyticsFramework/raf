
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__)
#include "idgs_gch.h"
#endif // GNUC_ $

#include "gtest/gtest.h"
#include "idgs/client/client_pool.h"
#include "idgs/tpc/pb/tpc_sync.pb.h"

using namespace std;
using namespace idgs;
using namespace idgs::client;
using namespace idgs::pb;
using namespace idgs::tpc::pb;

TEST(migration, verify) {
  ClientSetting setting;
  setting.clientConfig = "conf/client.conf";
  ResultCode code;

  auto& pool = getTcpClientPool();
  code = pool.loadConfig(setting);
  ASSERT_EQ(RC_SUCCESS, code);

  auto client = pool.getTcpClient(code);
  ASSERT_EQ(RC_SUCCESS, code);

  auto request = make_shared<MigrationVerifyRequest>();
  request->set_schema_name("tpch");
  request->set_store_name("LineItem");

  ClientActorMessagePtr clientActorMsg = std::make_shared<ClientActorMessage>();
  clientActorMsg->setOperationName("VERIFY_REQUEST");
  clientActorMsg->setChannel(TC_TCP);
  clientActorMsg->setDestActorId("tpc.migration_verify");
  clientActorMsg->setSourceActorId("client_actor_id");
  clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
  clientActorMsg->setDestMemberId(ANY_MEMBER);
  clientActorMsg->setPayload(request);

  // response
  ClientActorMessagePtr tcpResponse;
  code = client->sendRecv(clientActorMsg, tcpResponse);
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "Error sending message, cause by " << getErrorDescription(code);
  }
  ASSERT_EQ(RC_SUCCESS, code);

  if (tcpResponse.get() == NULL) {
    LOG(ERROR) << "No response found.";
  }
  ASSERT_NE((void*)NULL, (tcpResponse.get()));

  MigrationVerifyResponse response;
  ASSERT_TRUE(tcpResponse->parsePayload(&response));

  ASSERT_EQ(0, response.result_code());

  std::map<int32_t, std::map<std::string, std::vector<size_t>>> dataSizeMap;
  for (int32_t m = 0; m < response.member_data_size(); ++ m) {
    auto& memberData = response.member_data(m);
    VLOG(0) << "########################################";
    VLOG(0) << "  member : " << to_string(memberData.member_id());
    for (int32_t s = 0; s < memberData.store_data_size(); ++ s) {
      auto& storeData = memberData.store_data(s);
      bool printed = storeData.store_name() == "LineItem";
      VLOG_IF(0, printed) << "===================================";
      VLOG_IF(0, printed) << "  store : " << storeData.schema_name() << "." << storeData.store_name();
      for (int32_t p = 0; p < storeData.partition_data_size(); ++ p) {
        auto& partitionData = storeData.partition_data(p);
        VLOG_IF(0, printed) << "------------------------------";
        VLOG_IF(0, printed) << "  partition : " << to_string(partitionData.partition_id());
        VLOG_IF(0, printed) << "  position  : " << to_string(partitionData.position());
        VLOG_IF(0, printed) << "  data size : " << to_string(partitionData.size());

        ASSERT_EQ(partitionData.member_id(), memberData.member_id());
        for (int32_t i = 0; i < partitionData.key_partition_size(); ++ i) {
          auto& keyPartition = partitionData.key_partition(i);

          ASSERT_EQ(keyPartition.key_partition_id(), partitionData.partition_id());
          ASSERT_EQ(keyPartition.key_member_id(), partitionData.member_id());
        }

        dataSizeMap[partitionData.partition_id()][storeData.schema_name() + "." + storeData.store_name()].push_back(partitionData.size());
      }
      VLOG_IF(0, printed) << "------------------------------";
    }
    VLOG(0) << "===================================";
  }

  auto it = dataSizeMap.begin();
  for (; it != dataSizeMap.end(); ++ it) {
    auto sit = it->second.begin();
    for (; sit != it->second.end(); ++ sit) {
      size_t size = 0;
      for (int32_t i = 0; i < sit->second.size(); ++ i) {
        if (i == 0) {
          size = sit->second.at(i);
        } else {
          if (size != sit->second.at(i)) {
            LOG(ERROR) << "partition " << it->first << " store " << sit->first << " migration error, " << sit->second.at(i) << "/" << size;
          }
          ASSERT_EQ(size, sit->second.at(i));
        }
      }
    }
  }

  VLOG(0) << "########################################";
}
