/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#include "tcp_loader.h"

namespace idgs {
namespace client {

typedef Loader super;

idgs::ResultCode TcpLoader::init(LoaderSettings* settings) {
  super::init(settings); /// call super's init
  ClientSetting client_setting;
  client_setting.clientConfig = settings->client_cfg_file;
  client_setting.storeConfig = "";
  client_setting.scriptFile = "";
  idgs::ResultCode rc = ::idgs::util::singleton<TcpClientPool>::getInstance().loadConfig(client_setting);
  if (rc != RC_SUCCESS) {
    LOG(ERROR)<< "load client config error, " << getErrorDescription(rc) << ", config file: " << client_setting.clientConfig;
    return rc;
  }
  return RC_OK;
}

void TcpLoader::sendMessage(idgs::client::ClientActorMessagePtr& clientActorMsg, ResultCode* rc, int time_out) {
  std::shared_ptr < TcpClientInterface > client = ::idgs::util::singleton<TcpClientPool>::getInstance().getTcpClient(
      *rc);
  sendMessage(client, clientActorMsg, rc, time_out);
  client->close();
}

void TcpLoader::sendMessage(const std::shared_ptr<idgs::client::TcpClientInterface>& client,
    idgs::client::ClientActorMessagePtr& clientActorMsg, ResultCode* rc, int time_out) {
  DVLOG(2) << clientActorMsg->toString();
  if (*rc != RC_OK) {
    LOG_FIRST_N(ERROR, 20) << "No client available";
    return;
  }
  client->send(clientActorMsg, rc, time_out);
  if (*rc != RC_SUCCESS) {
    LOG(ERROR)<< "Send client actor message error, " << getErrorDescription(*rc);
    client->close();
    return;
  }
}

ClientActorMessagePtr TcpLoader::sendRecvMessage(idgs::client::ClientActorMessagePtr& clientActorMsg, ResultCode* rc,
    int time_out) {
  std::shared_ptr < TcpClientInterface > client = ::idgs::util::singleton<TcpClientPool>::getInstance().getTcpClient(
      *rc);
  ClientActorMessagePtr response = sendRecvMessage(client, clientActorMsg, rc, time_out);
  client->close();
  return response;
}

ClientActorMessagePtr TcpLoader::sendRecvMessage(const std::shared_ptr<idgs::client::TcpClientInterface>& client,
    idgs::client::ClientActorMessagePtr& clientActorMsg, ResultCode* rc, int time_out) {
  DVLOG(2) << clientActorMsg->toString();
  if (*rc != RC_OK) {
    LOG_FIRST_N(ERROR, 10) << "No client available";
    return ClientActorMessagePtr(NULL);
  }
  ClientActorMessagePtr response = client->sendRecv(clientActorMsg, rc, time_out);
  if (*rc != RC_SUCCESS) {
    LOG_FIRST_N(ERROR, 10) << "Send client actor message error, " << getErrorDescription(*rc);
    return ClientActorMessagePtr(NULL);
  }
  client->close();
  return response;
}
}
}
