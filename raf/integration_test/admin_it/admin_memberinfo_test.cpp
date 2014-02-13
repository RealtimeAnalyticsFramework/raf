
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__)
#include "idgs_gch.h"
#endif // GNUC_ $

#include "gtest/gtest.h"
#include "idgs/client/admin/admin_client.h"

using namespace std;
using namespace idgs;
using namespace idgs::admin::pb;
using namespace idgs::client;
using namespace idgs::client::admin;


TEST(cluster_admin, all_membershipinfo_test) {

  ClientSetting setting;
  setting.clientConfig = "integration_test/store_it/client.conf";
  ResultCode code = ::idgs::util::singleton<TcpClientPool>::getInstance().loadConfig(setting);
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "initial TcpSynchronousClient error: " << getErrorDescription(code);
    exit(1);
  }
  shared_ptr<TcpClientInterface> client = ::idgs::util::singleton<TcpClientPool>::getInstance().getTcpClient(code);

  //-------------------- send get ALL_MEMBERS_ID request ---------------------------------------------
  std::shared_ptr<ClusterModuleRequest> request(new ClusterModuleRequest);
  request->addAllMembersInfoAttr();
  idgs::client::ClientActorMessagePtr getActorIdReq = AdminUtil::createAdminRequest(request, GET);

  idgs::client::ClientActorMessagePtr tcpResponse = client->sendRecv(getActorIdReq, &code);
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "Error in get data to partition store, cause by " << getErrorDescription(code);
    exit(1);
  }

  idgs::admin::pb::AdminResponse response;
  tcpResponse->parsePayload(&response);

  AttributeResponse attrResp;
  EXPECT_EQ(true, AdminUtil::getAttributeResponse(response, Cluster, idgs::admin::cluster::ALL_MEMBERS_ID, attrResp));

  LOG(INFO) << "get response: " << response.DebugString();
  LOG(INFO) << "get attribute: " << attrResp.attribute();

  EXPECT_EQ(idgs::admin::pb::ResponseStatus::Success, attrResp.status());
  EXPECT_EQ("membership_table.all_members_info", attrResp.attribute());

  //-------------------- send get get one member info request ---------------------------------------------
  std::shared_ptr<ClusterModuleRequest> request2(new ClusterModuleRequest);
  request2->addOneMemberInfoAttr("0");
  idgs::client::ClientActorMessagePtr getActorIdReq2 = AdminUtil::createAdminRequest(request2, GET);

  idgs::client::ClientActorMessagePtr tcpResponse2 = client->sendRecv(getActorIdReq2, &code);
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "Error in get data to partition store, cause by " << getErrorDescription(code);
    exit(1);
  }

  idgs::admin::pb::AdminResponse response2;
  tcpResponse2->parsePayload(&response2);

  AttributeResponse attrResp2;
  EXPECT_EQ(true, AdminUtil::getAttributeResponse(response2, Cluster,
      idgs::admin::cluster::MEMBER_INFO + ";" + idgs::admin::cluster::MEMBER_ID_PARAM + "=0", attrResp2));

  LOG(INFO) << "get response2: " << response2.DebugString();
  LOG(INFO) << "get attribute2: " << attrResp2.attribute();

  EXPECT_EQ(idgs::admin::pb::ResponseStatus::Success, attrResp2.status());
  EXPECT_EQ("membership_table.member_info;member_id=0", attrResp2.attribute());

  client->close();

  ::idgs::util::singleton<TcpClientPool>::getInstance().close();
}

