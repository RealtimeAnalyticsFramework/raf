
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
#include "idgs/client/command_parser.h"

using namespace std;
using namespace idgs;
using namespace idgs::client;

bool parsePbJsonPayload(std::string& outPut, ClientActorMessagePtr msg) {
  outPut = protobuf::JsonMessage::toIndentJsonString(msg->getRpcMessage()->payload());
  return true;
}

bool parsePaylaod(std::string& outPut, ClientActorMessagePtr msg) {
  if (msg->getSerdesType() == idgs::pb::PB_JSON) {
    return parsePbJsonPayload(outPut, msg);
  } else {
    outPut = msg->getRpcMessage()->payload();
    return true;
  }
}

bool execute(const string& cmd) {
  CommandParser parser;
  Command command;
  ResultCode code = parser.parse(cmd, &command);
  if (code != RC_SUCCESS) {
    cout << "Error : " << getErrorDescription(code);
    return false;
  }
  std::shared_ptr<TcpClientInterface> client = ::idgs::util::singleton<TcpClientPool>::getInstance().getTcpClient(code);
  if (code != RC_SUCCESS) {
    DLOG(ERROR) << "Get TcpSynchronousClient error: " << getErrorDescription(code);;
    exit(1);
  }
  ClientActorMessagePtr clientActorMsg = std::make_shared<ClientActorMessage>();
  code = command.toActorMsg(clientActorMsg);
  if (code != RC_SUCCESS) {
    DLOG(ERROR) << "Parse command error: " << getErrorDescription(code) << ", command is : " << cmd;
    exit(1);
  }

  // send message, wait and get response message
  ClientActorMessagePtr tcpResponse = client->sendRecv(clientActorMsg, &code);

  if (code != RC_SUCCESS) {
    cout << "execute the command error: " << getErrorDescription(code) << endl;
    return false;
  }

  string result;
  if (!parsePaylaod(result, tcpResponse)) {
    cout << "get result error!! >> " << tcpResponse->getPayload()->DebugString();
    return false;
  }

  cout << result;

  return true;
}

TEST(net_admin, teset) {
  ClientSetting setting;
  setting.clientConfig = "integration_test/store_it/client.conf";
  ResultCode code = ::idgs::util::singleton<TcpClientPool>::getInstance().loadConfig(setting);
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "initial TcpSynchronousClient error: " << getErrorDescription(code);
    exit(1);
  }
  //shared_ptr<TcpClientInterface> client = ::idgs::util::singleton<TcpClientPool>::getInstance().getTcpClient(code);

  bool result = execute("admin_service get {\"module_op_request\":[{\"module_name\":\"net\",\"attributes\":[{\"attribute\":\"net.network_statistics;member_id=0\"}]}]}");

  EXPECT_EQ(result, true);
}



