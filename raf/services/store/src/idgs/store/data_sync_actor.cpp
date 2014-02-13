
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "data_sync_actor.h"

#include "idgs/cluster/cluster_framework.h"
#include "data_store.h"
#include "protobuf/message_helper.h"

using namespace idgs::cluster;
using namespace idgs::pb;
using namespace idgs::actor;
using namespace google::protobuf;
using namespace protobuf;

namespace idgs {
namespace store {

ActorDescriptorPtr ReplicatedStoreSyncStatefulActor::descriptor;

StoreSyncStatelessActor::StoreSyncStatelessActor(const std::string& actorId) {
  setActorId(actorId);

  descriptor = StoreSyncStatelessActor::generateActorDescriptor();
}

StoreSyncStatelessActor::~StoreSyncStatelessActor() {
  DLOG(INFO)<< "Deconstruct";

}

const idgs::actor::ActorMessageHandlerMap& StoreSyncStatelessActor::getMessageHandlerMap() const {
  static std::map<std::string, idgs::actor::ActorMessageHandler> handlerMap = {
      {DATA_STORE_SYNC, static_cast<idgs::actor::ActorMessageHandler>(&StoreSyncStatelessActor::handleDataSync)}
  };

  return handlerMap;
}

ActorDescriptorPtr StoreSyncStatelessActor::generateActorDescriptor() {
  static std::shared_ptr<ActorDescriptorWrapper> descriptor;
  if (descriptor)
    return descriptor;
  descriptor.reset(new ::ActorDescriptorWrapper);

  descriptor->setName(DATA_STORE_SYNC_ACTOR);
  descriptor->setDescription("Syncronize data of store.");
  descriptor->setType(AT_STATELESS);

  // operation name : DATA_STORE_SYNC
  ActorOperationDescriporWrapper dataSync;
  dataSync.setName(DATA_STORE_SYNC);
  dataSync.setDescription("Receive sync data request, get data with the given store name.");
  dataSync.setPayloadType("idgs.store.pb.SyncStoreRequest");
  descriptor->setInOperation(dataSync.getName(), dataSync);

  ActorOperationDescriporWrapper dataSyncResponse;
  dataSyncResponse.setName(DATA_STORE_SYNC_RESPONSE);
  dataSyncResponse.setDescription("Send sync data response");
  dataSyncResponse.setPayloadType("idgs.store.pb.SyncStore");
  descriptor->setOutOperation(dataSyncResponse.getName(), dataSyncResponse);

  return descriptor;
}

void StoreSyncStatelessActor::handleDataSync(const ActorMessagePtr& msg) {
  // get sync data for replicated store
  DVLOG(2) << "Replicated store sync data, get data from other member.";
  idgs::store::pb::SyncStoreRequest* request = dynamic_cast<idgs::store::pb::SyncStoreRequest*>(msg->getPayload().get());
  shared_ptr<idgs::store::pb::SyncStore> data(new idgs::store::pb::SyncStore);

  DataStore& dataStore = ::idgs::util::singleton<DataStore>::getInstance();

  shared_ptr<StoreConfigWrapper> storeConfigWrapper;
  string storeName = request->store_name();
  if (dataStore.loadStoreConfig(storeName, storeConfigWrapper) == RC_SUCCESS) {
    switch (storeConfigWrapper->getStoreConfig().partition_type()) {
      case pb::REPLICATED: {
        // get sync data
        data->set_store_name(storeName);
        dataStore.syncStore(storeName, data);
        if (data->data_size() == 0) {
          DVLOG(1) << "Store " << storeName << " in member " << msg->getDestMemberId() << " has no data to sync";
        }

        // send response back
        DVLOG(2) << "Send response";
        ActorMessagePtr msgResponse = msg->createResponse();
        msgResponse->setOperationName(DATA_STORE_SYNC_RESPONSE);
        msgResponse->setPayload(data);
        idgs::actor::sendMessage(msgResponse);

        break;
      }
      default: {
        break;
      }
    }
  } else {
    LOG(ERROR)<< "Store named " << storeName << " is not configed.";
  }
}

ReplicatedStoreSyncStatefulActor::ReplicatedStoreSyncStatefulActor() : storeCnt(0) {
  getReplicatedStores();
}

ReplicatedStoreSyncStatefulActor::~ReplicatedStoreSyncStatefulActor() {
}

void ReplicatedStoreSyncStatefulActor::innerProcess(const idgs::actor::ActorMessagePtr& msg) {
  if (::idgs::util::singleton<ClusterFramework>::getInstance().getLocalMember()->isLocalStore()) {
    super::innerProcess(msg);
  } else {
    DVLOG(2) << "This member is not local store";
  }
}

const idgs::actor::ActorMessageHandlerMap& ReplicatedStoreSyncStatefulActor::getMessageHandlerMap() const {
  static std::map<std::string, idgs::actor::ActorMessageHandler> handlerMap = {
      {DATA_STORE_SYNC_RESPONSE, static_cast<idgs::actor::ActorMessageHandler>(&ReplicatedStoreSyncStatefulActor::handleDataSyncResponse)}
  };

  return handlerMap;
}

ActorDescriptorPtr ReplicatedStoreSyncStatefulActor::generateActorDescriptor() {
  static std::shared_ptr<ActorDescriptorWrapper> descriptor;
  if (descriptor)
    return descriptor;
  descriptor.reset(new ::ActorDescriptorWrapper);

  descriptor->setName(DATA_REPLICATED_STORE_SYNC_ACTOR);
  descriptor->setDescription("Syncronize data of replicated store.");
  descriptor->setType(AT_STATEFUL);

  // operation name : DATA_STORE_SYNC_RESPONSE
  ActorOperationDescriporWrapper dataSyncResponse;
  dataSyncResponse.setName(DATA_STORE_SYNC_RESPONSE);
  dataSyncResponse.setDescription("Get the sync data response, put data to local store.");
  dataSyncResponse.setPayloadType("idgs.store.pb.SyncStore");
  descriptor->setInOperation(dataSyncResponse.getName(), dataSyncResponse);

  ReplicatedStoreSyncStatefulActor::descriptor = descriptor;

  return descriptor;
}

const ActorDescriptorPtr& ReplicatedStoreSyncStatefulActor::getDescriptor() const {
  return ReplicatedStoreSyncStatefulActor::descriptor;
}

void ReplicatedStoreSyncStatefulActor::handleDataSyncRequest() {
  MembershipTableMgr* memberManager = ::idgs::util::singleton<ClusterFramework>::getInstance().getMemberManager();

  int32_t localMemberId = memberManager->getLocalMemberId();
  const vector<MemberWrapper>& mamberTable = memberManager->getMemberTable();
  vector<int32_t> storeMembers;

  // get prepared or active other members.
  for (int32_t i = 0; i < mamberTable.size(); ++i) {
    if ((mamberTable[i].getStatus() == idgs::pb::PREPARED || mamberTable[i].getStatus() == idgs::pb::ACTIVE)
        && mamberTable[i].isLocalStore() && i != localMemberId) {
      storeMembers.push_back(i);
      break;
    }
  }

  if (storeMembers.empty()) {
    return;
  }

  if (replicatedStores.empty()) {
    LOG(INFO)<< "Cannot find any store.";
    DVLOG(2) << "Unregister actor ReplicatedStoreSyncStatefulActor";
    return;
  }

  ::idgs::util::singleton<RpcFramework>::getInstance().getActorFramework()->Register(getActorId(), this);
  // each store sync data from one member.
  DVLOG(2) << "Starting sync data with replicated store";
  int32_t memberIndex = 0;
  for (int32_t i = 0; i < replicatedStores.size(); ++i) {
    string storeName = replicatedStores[i];

    DVLOG(2) << "send sync request of store named " << storeName << " to member " << storeMembers[memberIndex] << ".";
    shared_ptr<idgs::store::pb::SyncStoreRequest> request(new idgs::store::pb::SyncStoreRequest);
    request->set_store_name(storeName);

    ActorMessagePtr reqmsg(new ActorMessage);
    reqmsg->setDestMemberId(storeMembers[memberIndex]);
    reqmsg->setSourceMemberId(localMemberId);
    reqmsg->setOperationName(DATA_STORE_SYNC);
    reqmsg->setSourceActorId(getActorId());
    reqmsg->setDestActorId(DATA_STORE_SYNC_ACTOR);
    reqmsg->setChannel(TC_AUTO);
    reqmsg->setPayload(request);

    // send request to dest member with channel auto.
    ::idgs::actor::postMessage(reqmsg);

    // if the dest member is the last member, then next store sync from the first member.
    if (++memberIndex >= storeMembers.size()) {
      memberIndex = 0;
    }
  }
}

void ReplicatedStoreSyncStatefulActor::handleDataSyncResponse(const ActorMessagePtr& msg) {
  // store sync response, receive sync data.
  DVLOG(2) << "Replicated store sync data, receive data from other member.";
  idgs::store::pb::SyncStore* data = dynamic_cast<idgs::store::pb::SyncStore*>(msg->getPayload().get());

  string storeName = data->store_name();

  ++storeCnt;
  if (replicatedStores.size() == storeCnt) {
    DVLOG(2) << "Unregister actor ReplicatedStoreSyncStatefulActor.";
    terminate();
  }

  // whether the store is configed.
  shared_ptr<StoreConfigWrapper> storeConfigWrapper;
  if (::idgs::util::singleton<DataStore>::getInstance().loadStoreConfig(storeName, storeConfigWrapper) != RC_SUCCESS) {
    LOG(ERROR)<< "Store " << storeName << " is not in config file.";
    return;
  }

  // whether the protobuf of key is registered
  string keyName = storeConfigWrapper->getStoreConfig().key_type();
  if (!::idgs::util::singleton<MessageHelper>::getInstance().isMessageRegistered(keyName)) {
    LOG(ERROR)<< "Protobuf " << keyName << " with key type is not registered to system.";
    return;
  }

  // whether the protobuf of value is registered
  string valueName = storeConfigWrapper->getStoreConfig().value_type();
  if (!::idgs::util::singleton<MessageHelper>::getInstance().isMessageRegistered(valueName)) {
    LOG(ERROR)<< "Protobuf " << valueName << " with value type is not registered to system.";
    return;
  }

  if (data->data_size() == 0) {
    DVLOG(1) << "Replicated store " << storeName << " has not data to sync.";
  } else {
    // insert sync data
    for (int32_t i = 0; i < data->data_size(); ++i) {
      auto key = ::idgs::util::singleton<MessageHelper>::getInstance().createMessage(keyName);
      string valueName = storeConfigWrapper->getStoreConfig().value_type();
      auto value = ::idgs::util::singleton<MessageHelper>::getInstance().createMessage(valueName);
      protobuf::SerdesMode mode = (protobuf::SerdesMode) ((int32_t) msg->getSerdesType());
      protobuf::ProtoSerdesHelper::deserialize(mode, data->data(i).key(), key.get());
      protobuf::ProtoSerdesHelper::deserialize(mode, data->data(i).value(), value.get());

      StoreKey<Message> storeKey(key);
      StoreValue<Message> storeValue(value);

      ResultCode resultCode = ::idgs::util::singleton<DataStore>::getInstance().insertData(storeName, storeKey,
          storeValue);
      if (resultCode != RC_SUCCESS) {
        LOG(ERROR)<< "Insert data error: " << getErrorDescription(resultCode);
        continue;
      }
    }

    DVLOG(1) << "Replicated store " << storeName << " sync done.";
  }
}

void ReplicatedStoreSyncStatefulActor::getReplicatedStores() {
  DataStore& dataStore = ::idgs::util::singleton<DataStore>::getInstance();
  vector<string> storeNames = dataStore.getAllStoreNames();
  for (int32_t i = 0; i < storeNames.size(); ++i) {
    string storeName = storeNames[i];
    shared_ptr<StoreConfigWrapper> storeConfigWrapper;
    if (dataStore.loadStoreConfig(storeName, storeConfigWrapper) != RC_SUCCESS) {
      continue;
    }

    switch (storeConfigWrapper->getStoreConfig().partition_type()) {
    // replicated store send sync request.
    case pb::REPLICATED: {
      replicatedStores.push_back(storeName);
      break;
    }
    default: {
      break;
    }
    }
  }
}

} // namespace store
} /* namespace idgs */
