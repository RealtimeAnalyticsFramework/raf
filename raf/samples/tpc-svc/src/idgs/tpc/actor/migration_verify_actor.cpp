/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#include "migration_verify_actor.h"

#include "idgs/store/store_module.h"
#include "idgs/store/listener/listener_manager.h"

#include "idgs/sync/store_migration_source_actor.h"

namespace idgs {
namespace tpc {

std::string MigrationVerifyActor::actorName;
idgs::actor::ActorDescriptorPtr MigrationVerifyActor::descriptor;

MigrationVerifyActor::MigrationVerifyActor() : memberSize(0) {
  MigrationVerifyActor::actorName = "tpc.migration_verify";
  actorId = "tpc.migration_verify";
}

MigrationVerifyActor::~MigrationVerifyActor() {
}

const idgs::actor::ActorMessageHandlerMap& MigrationVerifyActor::getMessageHandlerMap() const {
  static idgs::actor::ActorMessageHandlerMap handlerMap = {
      {"VERIFY_REQUEST",   {
          static_cast<idgs::actor::ActorMessageHandler>(&MigrationVerifyActor::handleVerifyRequest),
          &idgs::tpc::pb::MigrationVerifyRequest::default_instance()
      }},
      {"LOCAL_VERIFY_REQUEST",  {
          static_cast<idgs::actor::ActorMessageHandler>(&MigrationVerifyActor::handleLocalVerifyRequest),
          &idgs::tpc::pb::MigrationVerifyRequest::default_instance()
      }},
      {"VERIFY_RESPONSE",  {
          static_cast<idgs::actor::ActorMessageHandler>(&MigrationVerifyActor::handleVerifyResponse),
          &idgs::tpc::pb::MigrationVerifyResponse::default_instance()
      }}
  };

  return handlerMap;
}

const idgs::actor::ActorDescriptorPtr& MigrationVerifyActor::getDescriptor() const {
  return MigrationVerifyActor::descriptor;
}

idgs::actor::ActorDescriptorPtr MigrationVerifyActor::generateActorDescriptor() {
  static std::shared_ptr<idgs::actor::ActorDescriptorWrapper> descriptor;
  if (descriptor) {
    return descriptor;
  }

  descriptor = std::make_shared<idgs::actor::ActorDescriptorWrapper>();

  descriptor->setName(MigrationVerifyActor::actorName);
  descriptor->setDescription("Verify migration");
  descriptor->setType(idgs::pb::AT_STATEFUL);

  // operation of VERIFY_REQUEST
  idgs::actor::ActorOperationDescriporWrapper request;
  request.setName("VERIFY_REQUEST");
  request.setDescription("Handle global verify request");
  request.setPayloadType("idgs.tpc.pb.MigrationVerifyRequest");
  descriptor->setInOperation(request.getName(), request);

  // operation LOCAL_VERIFY_REQUEST
  idgs::actor::ActorOperationDescriporWrapper localRequest;
  localRequest.setName("LOCAL_VERIFY_REQUEST");
  localRequest.setDescription("Handle local verify request");
  localRequest.setPayloadType("idgs.tpc.pb.MigrationVerifyRequest");
  descriptor->setInOperation(localRequest.getName(), localRequest);

  // operation VERIFY_RESPONSE
  idgs::actor::ActorOperationDescriporWrapper response;
  response.setName("VERIFY_RESPONSE");
  response.setDescription("Handle verify response");
  response.setPayloadType("idgs.tpc.pb.MigrationVerifyResponse");
  descriptor->setInOperation(response.getName(), response);

  // out operation of VERIFY_REQUEST
  descriptor->setOutOperation(localRequest.getName(), localRequest);

  // out operation of LOCAL_VERIFY_REQUEST and VERIFY_RESPONSE
  descriptor->setOutOperation(response.getName(), response);

  // local access descriptor
  MigrationVerifyActor::descriptor = descriptor;

  return descriptor;
}

void MigrationVerifyActor::handleVerifyRequest(const idgs::actor::ActorMessagePtr& msg) {
  pb::MigrationVerifyRequest* request = NULL;
  if (msg->getPayload()) {
    request = dynamic_cast<pb::MigrationVerifyRequest*>(msg->getPayload().get());
  }

  if (request && request->has_schema_name() && request->has_store_name()) {
    auto datastore = idgs::store::idgs_store_module()->getDataStore();
    auto store = datastore->getStore(request->schema_name(), request->store_name());
    if (!store) {
      LOG(ERROR) << "store " << request->schema_name() << "." << request->store_name() << " is not found.";
      auto payload = std::make_shared<pb::MigrationVerifyResponse>();
      payload->set_result_code(static_cast<int32_t>(RC_STORE_NOT_FOUND));
      auto respMsg = msg->createResponse();
      respMsg->setOperationName("VERIFY_RESPONSE");
      respMsg->setPayload(payload);
      idgs::actor::sendMessage(respMsg);
      return;
    }

    if (store->getStoreConfig()->getStoreConfig().partition_type() != idgs::store::pb::PARTITION_TABLE) {
      LOG(ERROR) << "store " << request->schema_name() << "." << request->store_name() << " is not a partition store.";
      auto payload = std::make_shared<pb::MigrationVerifyResponse>();
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

void MigrationVerifyActor::handleLocalVerifyRequest(const idgs::actor::ActorMessagePtr& msg) {
  pb::MigrationVerifyRequest* request = NULL;
  if (msg->getPayload()) {
    request = dynamic_cast<pb::MigrationVerifyRequest*>(msg->getPayload().get());
  }

  auto datastore = idgs::store::idgs_store_module()->getDataStore();
  std::vector<idgs::store::StorePtr> stores;
  if (request && request->has_schema_name() && request->has_store_name()) {
    auto store = datastore->getStore(request->schema_name(), request->store_name());
    stores.push_back(store);
  } else {
    datastore->getStores(stores);
  }

  auto app = idgs_application();
  auto cluster = app->getClusterFramework();
  auto pcnt = cluster->getPartitionCount();
  auto bkcnt = cluster->getClusterConfig()->max_replica_count() - 1;
  auto local = cluster->getLocalMember()->getId();
  auto partitionMgr = app->getPartitionManager();

  auto payload = std::make_shared<pb::MigrationVerifyResponse>();
  payload->set_result_code(static_cast<int32_t>(RC_SUCCESS));

  auto memberData = payload->add_member_data();
  memberData->set_member_id(local);

  for (int32_t i = 0; i < stores.size(); ++ i) {
    auto& store = stores.at(i);
    auto& storeConfigWrapper = store->getStoreConfig();
    if (storeConfigWrapper->getStoreConfig().partition_type() == idgs::store::pb::PARTITION_TABLE) {
      auto pstore = dynamic_cast<idgs::store::PartitionedStore*>(store.get());

      auto& schemaName = store->getStoreConfig()->getSchema();
      auto& storeName = store->getStoreConfig()->getStoreConfig().name();
      auto storeData = memberData->add_store_data();
      storeData->set_schema_name(schemaName);
      storeData->set_store_name(storeName);

      for (int32_t p = 0; p < pcnt; ++ p) {
        auto partition = partitionMgr->getPartition(p);
        for (int32_t pos = 0; pos < bkcnt + 1; ++ pos) {
          if (partition->getMemberId(pos) == local) {
            auto partitionData = storeData->add_partition_data();
            partitionData->set_partition_id(p);
            partitionData->set_position(pos);
            partitionData->set_member_id(local);
            partitionData->set_size(pstore->dataSize(p));
            VLOG(0) << schemaName << "." << storeName << " partition " << p << "(" << pos << ") data size " << partitionData->size() << " on member " << local;

            std::shared_ptr<idgs::store::StoreMap> map;
            pstore->snapshotStore(p, map);
            auto it = map->iterator();
            while (it->hasNext()) {
              idgs::store::StoreOption ps;
              storeConfigWrapper->calculatePartitionInfo(it->key(), &ps);
              ps.memberId = partitionMgr->getPartition(ps.partitionId)->getMemberId(pos);

              auto keyPartition = partitionData->add_key_partition();
              keyPartition->set_key_partition_id(ps.partitionId);
              keyPartition->set_key_member_id(ps.memberId);

              it->next();
            }
          }
        }
      }
    }
  }

  auto respMsg = msg->createResponse();
  respMsg->setOperationName("VERIFY_RESPONSE");
  respMsg->setPayload(payload);
  idgs::actor::sendMessage(respMsg);
}

void MigrationVerifyActor::handleVerifyResponse(const idgs::actor::ActorMessagePtr& msg) {
  auto response = dynamic_cast<pb::MigrationVerifyResponse*>(msg->getPayload().get());
  memberData.insert(std::pair<int32_t, pb::MemberMigrationData>(msg->getSourceMemberId(), response->member_data(0)));

  if (memberData.size() == memberSize) {
    auto payload = std::make_shared<pb::MigrationVerifyResponse>();
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
