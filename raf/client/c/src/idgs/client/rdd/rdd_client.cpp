/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#include "idgs/client/rdd/rdd_client.h"
#include "idgs/client/client_pool.h"
#include "idgs/rdd/rdd_const.h"

using namespace idgs::pb;
using namespace idgs::rdd;

namespace idgs {
namespace client {
namespace rdd {

ResultCode RddClient::createStoreDelegateRDD(const DelegateRddRequestPtr& request, DelegateRddResponsePtr& response,
    int time_out) {
  ResultCode code = RC_SUCCESS;
  auto client = getTcpClientPool().getTcpClient(code);
  if (code != RC_SUCCESS) {
    LOG(ERROR)<< "Error in get client, cause by " << getErrorDescription(code);
    return code;
  }

  if (!request->has_rdd_name()) {
    request->set_rdd_name(request->store_name() + "-DelegateRDD");
  }

  ClientActorMessagePtr clientActorMsg = std::make_shared<ClientActorMessage>();
  clientActorMsg->setOperationName(CREATE_STORE_DELEGATE_RDD);
  clientActorMsg->setChannel(TC_TCP);
  clientActorMsg->setDestActorId(RDD_SERVICE_ACTOR);
  clientActorMsg->setDestMemberId(ANY_MEMBER);
  clientActorMsg->setSourceActorId("client_actor_id");
  clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
  clientActorMsg->setPayload(request);

  ClientActorMessagePtr resp;
  code = client->sendRecv(clientActorMsg, resp, time_out);
  if (code != RC_SUCCESS) {
    LOG(ERROR)<< "Error in create store delegate RDD, cause by " << getErrorDescription(code);
  } else if (!resp->parsePayload(response.get())) {
    LOG(ERROR) << "Cannot parse response for store delegate.";
    code = RC_ERROR;
  }

  client->close();

  return code;
}

ResultCode RddClient::createRdd(const RddRequestPtr& request, RddResponsePtr& response, const AttachMessage& attach,
    int time_out) {
  ResultCode code = RC_SUCCESS;
  auto client = getTcpClientPool().getTcpClient(code);
  if (code != RC_SUCCESS) {
    LOG(ERROR)<< "Error in get client, cause by " << getErrorDescription(code);
    return code;
  }

  ClientActorMessagePtr clientActorMsg = std::make_shared<ClientActorMessage>();
  clientActorMsg->setOperationName(CREATE_RDD);
  clientActorMsg->setChannel(TC_TCP);
  clientActorMsg->setDestActorId(RDD_SERVICE_ACTOR);
  clientActorMsg->setDestMemberId(ANY_MEMBER);
  clientActorMsg->setSourceActorId("client_actor_id");
  clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
  clientActorMsg->setPayload(request);

  if (!attach.empty()) {
    auto it = attach.begin();
    for (; it != attach.end(); ++it) {
      clientActorMsg->setAttachment(it->first, it->second);
    }
  }

  ClientActorMessagePtr resp;
  code = client->sendRecv(clientActorMsg, resp, time_out);
  if (code != RC_SUCCESS) {
    LOG(ERROR)<< "Error in sending message to server, cause by " << getErrorDescription(code);
  } else if (!resp->parsePayload(response.get())) {
    LOG(ERROR) << "Cannot parse response for store delegate.";
    code = RC_ERROR;
  }

  client->close();

  return code;
}

ResultCode RddClient::sendAction(const ActionRequestPtr& request, ActionResponsePtr& response, ActionResultPtr& result,
    const AttachMessage& attach, int time_out) {
  if (!request->has_rdd_name()) {
    LOG(ERROR)<< "Request must has rdd name.";
    return RC_RDD_IS_NOT_READY;
  }

  ResultCode code = RC_SUCCESS;
  auto client = getTcpClientPool().getTcpClient(code);
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "Error in get client, cause by " << getErrorDescription(code);
    return code;
  }

  char* sTimeout = getenv("ACTION_TIMEOUT");
  if (sTimeout) {
    time_out = atoi(sTimeout);
    if (time_out < 0) {
      time_out = 30;
    }
  }

  ClientActorMessagePtr clientActorMsg = std::make_shared<ClientActorMessage>();
  clientActorMsg->setOperationName(RDD_ACTION_REQUEST);
  clientActorMsg->setChannel(TC_TCP);
  clientActorMsg->setDestActorId(RDD_SERVICE_ACTOR);
  clientActorMsg->setDestMemberId(ANY_MEMBER);
  clientActorMsg->setSourceActorId("client_actor_id");
  clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
  clientActorMsg->setPayload(request);

  if (!attach.empty()) {
    auto it = attach.begin();
    for (; it != attach.end(); ++ it) {
      clientActorMsg->setAttachment(it->first, it->second);
    }
//    clientActorMsg->toBuffer();
  }

  ClientActorMessagePtr resp;
  code = client->sendRecv(clientActorMsg, resp, time_out);
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "Error in sending message to server, cause by " + getErrorDescription(code);
    return code;
  } else if (!resp->parsePayload(response.get())) {
    LOG(ERROR) << "Cannot parse action response";
    return RC_ERROR;
  } else {
    if (resp->getRawAttachments().find(ACTION_RESULT) != resp->getRawAttachments().end()) {
      if (!resp->parseAttachment(ACTION_RESULT, result.get())) {
        LOG(ERROR) << "parse action result error, message: " << resp->toString();
        return RC_ERROR;
      }
    }
  }

  client->close();

  return RC_SUCCESS;
}

ResultCode RddClient::sendAction(const ActionRequestPtr& request, ActionResponsePtr& response, ActionResultPtr& result,
    const ActorId& rddId, const AttachMessage& attach, int time_out) {
  ResultCode code;
  auto client = getTcpClientPool().getTcpClient(code);
  if (code != RC_SUCCESS) {
    LOG(ERROR)<< "Error in get client, cause by " << getErrorDescription(code);
    return code;
  }

  char* sTimeout = getenv("ACTION_TIMEOUT");
  if (sTimeout) {
    time_out = atoi(sTimeout);
    if (time_out < 0) {
      time_out = 30;
    }
  }

  ClientActorMessagePtr clientActorMsg = std::make_shared<ClientActorMessage>();
  clientActorMsg->setOperationName(RDD_ACTION_REQUEST);
  clientActorMsg->setChannel(TC_TCP);
  clientActorMsg->setDestActorId(rddId.actor_id());
  clientActorMsg->setDestMemberId(rddId.member_id());
  clientActorMsg->setSourceActorId("client_actor_id");
  clientActorMsg->setSourceMemberId(CLIENT_MEMBER);
  clientActorMsg->setPayload(request);

  if (!attach.empty()) {
    auto it = attach.begin();
    for (; it != attach.end(); ++it) {
      clientActorMsg->setAttachment(it->first, it->second);
    }
    clientActorMsg->toBuffer();
  }

  ClientActorMessagePtr resp;
  code = client->sendRecv(clientActorMsg, resp, time_out);

  if (code != RC_SUCCESS) {
    LOG(ERROR)<< "Error in sending message to server, cause by " + getErrorDescription(code);
    return code;
  } else if (!resp->parsePayload(response.get())) {
    LOG(ERROR) << "Cannot parse action response";
    return RC_ERROR;
  } else {
    if (resp->getRawAttachments().find(ACTION_RESULT) != resp->getRawAttachments().end()) {
      if (!resp->parseAttachment(ACTION_RESULT, result.get())) {
        LOG(ERROR) << "Cannot find of parse action result";
        return RC_ERROR;
      }
    }
  }

  client->close();

  return RC_SUCCESS;
}

ResultCode RddClient::init(const std::string& clientConfig) {
  idgs::client::ClientSetting setting;
  setting.clientConfig = clientConfig;

  ///replace by env.
  char* env_client_cfg_file = getenv("CLIENT_CONFIG");
  if (env_client_cfg_file) {
    setting.clientConfig = env_client_cfg_file;
  }

  if (setting.clientConfig == "") {
    LOG(ERROR)<< "Error in load client setting, cause by client config file is not found";
    return RC_ERROR;
  }

  ResultCode code = getTcpClientPool().loadConfig(setting);
  if (code != idgs::RC_SUCCESS) {
    LOG(ERROR)<< "Error in load client setting, cause by " << idgs::getErrorDescription(code);
  }
  return code;
}

const idgs::store::StoreConfigWrapperPtr& RddClient::getStoreConfigWrapper(const std::string& storeName) const {
  auto store = getTcpClientPool().getDataStore()->getStore(storeName);
  if (!store) {
    static idgs::store::StoreConfigWrapperPtr null;
    return null;
  }

  return store->getStoreConfigWrapper();
}

}
}
}
