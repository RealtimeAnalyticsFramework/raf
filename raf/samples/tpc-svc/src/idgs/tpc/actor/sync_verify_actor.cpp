/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#include "sync_verify_actor.h"

#include "idgs/store/store_module.h"
#include "idgs/store/listener/listener_manager.h"


namespace idgs {
namespace tpc {

std::string SyncVerifyActor::actorName;
idgs::actor::ActorDescriptorPtr SyncVerifyActor::descriptor;

SyncVerifyActor::SyncVerifyActor() : memberSize(0) {
  SyncVerifyActor::actorName = "tpc.sync_verify";
  actorId = "tpc.sync_verify";
}

SyncVerifyActor::~SyncVerifyActor() {
}

const idgs::actor::ActorMessageHandlerMap& SyncVerifyActor::getMessageHandlerMap() const {
  static idgs::actor::ActorMessageHandlerMap handlerMap = {
      {"VERIFY_REQUEST", {
          static_cast<idgs::actor::ActorMessageHandler>(&SyncVerifyActor::handleVerifyRequest),
          &idgs::tpc::pb::SyncVerifyRequest::default_instance()
      }},
      {"LOCAL_VERIFY_REQUEST",  {
          static_cast<idgs::actor::ActorMessageHandler>(&SyncVerifyActor::handleLocalVerifyRequest),
          &idgs::tpc::pb::SyncVerifyRequest::default_instance()
      }},
      {"VERIFY_RESPONSE",  {
          static_cast<idgs::actor::ActorMessageHandler>(&SyncVerifyActor::handleVerifyResponse),
          &idgs::tpc::pb::SyncVerifyResponse::default_instance()
      }}
  };

  return handlerMap;
}

const idgs::actor::ActorDescriptorPtr& SyncVerifyActor::getDescriptor() const {
  return SyncVerifyActor::descriptor;
}

idgs::actor::ActorDescriptorPtr SyncVerifyActor::generateActorDescriptor() {
  static std::shared_ptr<idgs::actor::ActorDescriptorWrapper> descriptor;
  if (descriptor) {
    return descriptor;
  }

  descriptor = std::make_shared<idgs::actor::ActorDescriptorWrapper>();

  descriptor->setName(SyncVerifyActor::actorName);
  descriptor->setDescription("Verify sync");
  descriptor->setType(idgs::pb::AT_STATEFUL);

  // operation of VERIFY_REQUEST
  idgs::actor::ActorOperationDescriporWrapper request;
  request.setName("VERIFY_REQUEST");
  request.setDescription("Handle global verify request");
  request.setPayloadType("idgs.tpc.pb.SyncVerifyRequest");
  descriptor->setInOperation(request.getName(), request);

  // operation LOCAL_VERIFY_REQUEST
  idgs::actor::ActorOperationDescriporWrapper localRequest;
  localRequest.setName("LOCAL_VERIFY_REQUEST");
  localRequest.setDescription("Handle local verify request");
  localRequest.setPayloadType("idgs.tpc.pb.SyncVerifyRequest");
  descriptor->setInOperation(localRequest.getName(), localRequest);

  // operation VERIFY_RESPONSE
  idgs::actor::ActorOperationDescriporWrapper response;
  response.setName("VERIFY_RESPONSE");
  response.setDescription("Handle verify response");
  response.setPayloadType("idgs.tpc.pb.SyncVerifyResponse");
  descriptor->setInOperation(response.getName(), response);

  // out operation of VERIFY_REQUEST
  descriptor->setOutOperation(localRequest.getName(), localRequest);

  // out operation of LOCAL_VERIFY_REQUEST and VERIFY_RESPONSE
  descriptor->setOutOperation(response.getName(), response);

  // local access descriptor
  SyncVerifyActor::descriptor = descriptor;

  return descriptor;
}

void SyncVerifyActor::handleVerifyRequest(const idgs::actor::ActorMessagePtr& msg) {
  pb::SyncVerifyRequest* request = NULL;
  if (msg->getPayload()) {
    request = dynamic_cast<pb::SyncVerifyRequest*>(msg->getPayload().get());
  }

  if (request && request->has_schema_name() && request->has_store_name()) {
    auto datastore = idgs::store::idgs_store_module()->getDataStore();
    auto store = datastore->getStore(request->schema_name(), request->store_name());
    if (!store) {
      LOG(ERROR) << "store " << request->schema_name() << "." << request->store_name() << " is not found.";
      auto payload = std::make_shared<pb::SyncVerifyResponse>();
      payload->set_result_code(static_cast<int32_t>(RC_STORE_NOT_FOUND));
      auto respMsg = msg->createResponse();
      respMsg->setOperationName("VERIFY_RESPONSE");
      respMsg->setPayload(payload);
      idgs::actor::sendMessage(respMsg);
      return;
    }

    if (store->getStoreConfig()->getStoreConfig().partition_type() != idgs::store::pb::REPLICATED) {
      LOG(ERROR) << "store " << request->schema_name() << "." << request->store_name() << " is not a replicated store.";
      auto payload = std::make_shared<pb::SyncVerifyResponse>();
      payload->set_result_code(static_cast<int32_t>(RC_STORE_NOT_FOUND));
      auto respMsg = msg->createResponse();
      respMsg->setOperationName("VERIFY_RESPONSE");
      respMsg->setPayload(payload);
      idgs::actor::sendMessage(respMsg);
      return;
    }
  }

  clientMsg = msg;
  auto& members = idgs_application()->getMemberManager()->getMemberTable();
  auto it = members.begin();
  for (; it != members.end(); ++ it) {
    if (it->isLocalStore() && (it->getState() == idgs::pb::MS_PREPARED || it->getState() == idgs::pb::MS_ACTIVE)) {
      auto reqMsg = createActorMessage();
      reqMsg->setOperationName("LOCAL_VERIFY_REQUEST");
      reqMsg->setDestMemberId(it->getId());
      reqMsg->setDestActorId(actorId);
      reqMsg->setPayload(msg->getPayload());
      idgs::actor::sendMessage(reqMsg);
      ++ memberSize;
    }
  }
}

void SyncVerifyActor::handleLocalVerifyRequest(const idgs::actor::ActorMessagePtr& msg) {
  pb::SyncVerifyRequest* request = NULL;
  if (msg->getPayload()) {
    request = dynamic_cast<pb::SyncVerifyRequest*>(msg->getPayload().get());
  }

  auto datastore = idgs::store::idgs_store_module()->getDataStore();
  std::vector<idgs::store::StorePtr> stores;
  if (request && request->has_schema_name() && request->has_store_name()) {
    auto store = datastore->getStore(request->schema_name(), request->store_name());
    stores.push_back(store);
  } else {
    datastore->getStores(stores);
  }

  auto local = idgs_application()->getMemberManager()->getLocalMemberId();

  auto payload = std::make_shared<pb::SyncVerifyResponse>();
  payload->set_result_code(static_cast<int32_t>(RC_SUCCESS));

  auto memberData = payload->add_member_data();
  memberData->set_member_id(local);

  for (int32_t i = 0; i < stores.size(); ++ i) {
    auto& store = stores.at(i);
    auto& storeConfigWrapper = store->getStoreConfig();
    if (storeConfigWrapper->getStoreConfig().partition_type() == idgs::store::pb::REPLICATED) {
      auto& schemaName = store->getStoreConfig()->getSchema();
      auto& storeName = store->getStoreConfig()->getStoreConfig().name();
      auto storeData = memberData->add_store_data();
      storeData->set_schema_name(schemaName);
      storeData->set_store_name(storeName);
      storeData->set_data_size(store->size());
    }
  }

  auto respMsg = msg->createResponse();
  respMsg->setOperationName("VERIFY_RESPONSE");
  respMsg->setPayload(payload);
  idgs::actor::sendMessage(respMsg);
}

void SyncVerifyActor::handleVerifyResponse(const idgs::actor::ActorMessagePtr& msg) {
  auto response = dynamic_cast<pb::SyncVerifyResponse*>(msg->getPayload().get());
  memberData.insert(std::pair<int32_t, pb::MemberSyncData>(msg->getSourceMemberId(), response->member_data(0)));

  if (memberData.size() == memberSize) {
    auto payload = std::make_shared<pb::SyncVerifyResponse>();
    payload->set_result_code(static_cast<int32_t>(RC_SUCCESS));
    auto it = memberData.begin();
    for (; it != memberData.end(); ++ it) {
      payload->add_member_data()->CopyFrom(it->second);
    }

    auto respMsg = clientMsg->createResponse();
    respMsg->setOperationName("VERIFY_RESPONSE");
    respMsg->setPayload(payload);

    idgs::actor::sendMessage(respMsg);

    memberData.clear();
    memberSize = 0;
  }
}

} /* namespace tpc */
} /* namespace idgs */
