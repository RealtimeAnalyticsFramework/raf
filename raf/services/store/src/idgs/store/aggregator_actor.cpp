
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "aggregator_actor.h"

#include "idgs/cluster/cluster_framework.h"

using namespace idgs::pb;
using namespace idgs::actor;
using namespace idgs::cluster;
using namespace google::protobuf;

namespace idgs {
namespace store {

ActorDescriptorPtr DataAggregatorActor::descriptor;
ActorDescriptorPtr DataSizeAggregatorActor::descriptor;

void AggregatorActor::sendResponse(const std::string& opName, const shared_ptr<Message>& response) {
  ActorMessagePtr am = clientMsg->createResponse();
  am->setOperationName(opName);
  am->setPayload(response);
  idgs::actor::sendMessage(am);
}


/// @todo use a loop of unicast instead of multicast.
void AggregatorActor::multicast(const ActorMessagePtr& msg, const string& actorId, const string& opName) {
  ActorMessagePtr multMsg = msg->createMulticastMessage();
  multMsg->setOperationName(opName);
  multMsg->setSourceActorId(getActorId());
  multMsg->setDestActorId(actorId);

  ::idgs::actor::postMessage(multMsg);
}

size_t DataAggregatorActor::getValidateMemberSize() {
  ClusterFramework& cluster = ::idgs::util::singleton<ClusterFramework>::getInstance();

  // get membership table
  const vector<MemberWrapper>& membershipTable = cluster.getMemberManager()->getMemberTable();
  size_t size = 0;
  for (int32_t i = 0; i < membershipTable.size(); ++i) {
    // get valid local store members
    if (membershipTable[i].isLocalStore() && (membershipTable[i].isPrepared() || membershipTable[i].isActive())) {
      ++size;
    }
  }

  return size;
}

const idgs::actor::ActorMessageHandlerMap& DataAggregatorActor::getMessageHandlerMap() const {
  static std::map<std::string, idgs::actor::ActorMessageHandler> handlerMap = {
      {DATA_STORE_INSERT_RESPONSE, static_cast<idgs::actor::ActorMessageHandler>(&DataAggregatorActor::handleInsertResponse)},
      {OP_INSERT, static_cast<idgs::actor::ActorMessageHandler>(&DataAggregatorActor::handleGlobeInsert)},
      {DATA_STORE_UPDATE_RESPONSE, static_cast<idgs::actor::ActorMessageHandler>(&DataAggregatorActor::handleUpdateResponse)},
      {OP_UPDATE, static_cast<idgs::actor::ActorMessageHandler>(&DataAggregatorActor::handleGlobeUpdate)},
      {DATA_STORE_REMOVE_RESPONSE, static_cast<idgs::actor::ActorMessageHandler>(&DataAggregatorActor::handleRemoveResponse)},
      {OP_DELETE, static_cast<idgs::actor::ActorMessageHandler>(&DataAggregatorActor::handleGlobeRemove)},
      {DATA_CLEAR_RESPONSE, static_cast<idgs::actor::ActorMessageHandler>(&DataAggregatorActor::handleDataClearResponse)},
      {OP_TRUNCATE, static_cast<idgs::actor::ActorMessageHandler>(&DataAggregatorActor::handleDataClear)}
  };

  return handlerMap;
}

ActorDescriptorPtr DataAggregatorActor::generateActorDescriptor() {
  static std::shared_ptr<ActorDescriptorWrapper> descriptor;
  if (descriptor)
    return descriptor;
  descriptor.reset(new ::ActorDescriptorWrapper);

  descriptor->setName(DATA_AGGREGATOR_ACTOR);
  descriptor->setDescription("Data CRUD aggregator for replicated store.");
  descriptor->setType(AT_STATEFUL);

  // insert
  ActorOperationDescriporWrapper dataInsertRequest;
  dataInsertRequest.setName(OP_INSERT);
  dataInsertRequest.setDescription("Receive the sync insert data request");
  dataInsertRequest.setPayloadType("idgs.store.pb.InsertRequest");
  descriptor->setInOperation(dataInsertRequest.getName(), dataInsertRequest);

  ActorOperationDescriporWrapper dataLocalInsertRequest;
  dataLocalInsertRequest.setName(DATA_STORE_LOCAL_INSERT);
  dataLocalInsertRequest.setDescription("Route the sync insert data request");
  dataLocalInsertRequest.setPayloadType("idgs.store.pb.InsertRequest");
  descriptor->setOutOperation(dataLocalInsertRequest.getName(), dataLocalInsertRequest);

  ActorOperationDescriporWrapper dataInsertResponse;
  dataInsertResponse.setName(DATA_STORE_INSERT_RESPONSE);
  dataInsertResponse.setDescription(
      "Receive the sync data response about status of data insert, when get all members response, response to client.");
  dataInsertResponse.setPayloadType("idgs.store.pb.InsertResponse");
  descriptor->setInOperation(dataInsertResponse.getName(), dataInsertResponse);
  descriptor->setOutOperation(dataInsertResponse.getName(), dataInsertResponse);

  // update
  ActorOperationDescriporWrapper dataUpdateRequest;
  dataUpdateRequest.setName(OP_UPDATE);
  dataUpdateRequest.setDescription("Receive the sync update data request");
  dataUpdateRequest.setPayloadType("idgs.store.pb.UpdateRequest");
  descriptor->setInOperation(dataUpdateRequest.getName(), dataUpdateRequest);

  ActorOperationDescriporWrapper dataLocalUpdateRequest;
  dataLocalUpdateRequest.setName(DATA_STORE_LOCAL_UPDATE);
  dataLocalUpdateRequest.setDescription("Route the sync update data request");
  dataLocalUpdateRequest.setPayloadType("idgs.store.pb.UpdateRequest");
  descriptor->setOutOperation(dataLocalUpdateRequest.getName(), dataLocalUpdateRequest);

  ActorOperationDescriporWrapper dataUpdateResponse;
  dataUpdateResponse.setName(DATA_STORE_UPDATE_RESPONSE);
  dataUpdateResponse.setDescription(
      "Receive the sync data response about status of data update, when get all members response, response to client.");
  dataUpdateResponse.setPayloadType("idgs.store.pb.UpdateResponse");
  descriptor->setInOperation(dataUpdateResponse.getName(), dataUpdateResponse);
  descriptor->setOutOperation(dataUpdateResponse.getName(), dataUpdateResponse);

  // remove
  ActorOperationDescriporWrapper dataRemoveRequest;
  dataRemoveRequest.setName(OP_DELETE);
  dataRemoveRequest.setDescription("Receive the sync remove data request");
  dataRemoveRequest.setPayloadType("idgs.store.pb.RemoveRequest");
  descriptor->setInOperation(dataRemoveRequest.getName(), dataRemoveRequest);

  ActorOperationDescriporWrapper dataRemoveLocalRequest;
  dataRemoveLocalRequest.setName(DATA_STORE_LOCAL_REMOVE);
  dataRemoveLocalRequest.setDescription("Route the sync remove data request");
  dataRemoveLocalRequest.setPayloadType("idgs.store.pb.RemoveRequest");
  descriptor->setOutOperation(dataRemoveLocalRequest.getName(), dataRemoveLocalRequest);

  ActorOperationDescriporWrapper dataRemoveResponse;
  dataRemoveResponse.setName(DATA_STORE_REMOVE_RESPONSE);
  dataRemoveResponse.setDescription(
      "Receive the sync data response about status of data remove, when get all members response, response to client.");
  dataRemoveResponse.setPayloadType("idgs.store.pb.RemoveResponse");
  descriptor->setInOperation(dataRemoveResponse.getName(), dataRemoveResponse);
  descriptor->setOutOperation(dataRemoveResponse.getName(), dataRemoveResponse);

  // operation name : DATA_CLEAR
  ActorOperationDescriporWrapper dataClearRequest;
  dataClearRequest.setName(OP_TRUNCATE);
  dataClearRequest.setDescription("Route data clear request message");
  dataClearRequest.setPayloadType("idgs.store.pb.DataClearRequest");
  descriptor->setInOperation(dataClearRequest.getName(), dataClearRequest);

  ActorOperationDescriporWrapper localDataClearRequest;
  localDataClearRequest.setName(LOCAL_DATA_CLEAR);
  localDataClearRequest.setDescription("Route data clear request message");
  localDataClearRequest.setPayloadType("idgs.store.pb.DataClearRequest");
  descriptor->setOutOperation(localDataClearRequest.getName(), localDataClearRequest);

  // operation name : DATA_STORE_LOCAL_REMOVE
  ActorOperationDescriporWrapper dataclearResponse;
  dataclearResponse.setName(DATA_CLEAR_RESPONSE);
  dataclearResponse.setDescription("Aggregate data clear response. and send result to client");
  dataclearResponse.setPayloadType("idgs.store.pb.DataClearResponse");
  descriptor->setInOperation(dataclearResponse.getName(), dataclearResponse);
  descriptor->setOutOperation(dataclearResponse.getName(), dataclearResponse);

  // local access descriptor
  DataAggregatorActor::descriptor = descriptor;

  return descriptor;
}

const ActorDescriptorPtr& DataAggregatorActor::getDescriptor() const {
  return DataAggregatorActor::descriptor;
}

void DataAggregatorActor::handleGlobeInsert(const ActorMessagePtr& msg) {
  DVLOG(2) << "multicast store insert operation with actor id : " << getActorId();
  clientMsg = msg;
  responseCount = 0;
  resultCode = idgs::store::pb::SRC_SUCCESS;
  multicast(msg, ACTORID_STORE_SERVCIE, DATA_STORE_LOCAL_INSERT);
}

void DataAggregatorActor::handleInsertResponse(const ActorMessagePtr& msg) {
  DVLOG(2) << "receive store insert response of member " << msg->getSourceMemberId();
  idgs::store::pb::InsertResponse* response = dynamic_cast<idgs::store::pb::InsertResponse*>(msg->getPayload().get());

  // error
  if (response->result_code() == idgs::store::pb::SRC_NOT_LOCAL_STORE) {
    DVLOG(2) << "response by a no local store member.";
  } else if (response->result_code() != idgs::store::pb::SRC_SUCCESS) {
    resultCode = response->result_code();
    LOG(ERROR)<< "error in insert data to member " << msg->getSourceActorId() << " error code : " << resultCode;
  }

    // collect response from all members and calculate size and then response to client.
  shared_ptr<idgs::store::pb::InsertResponse> globalResponse(new idgs::store::pb::InsertResponse);
  ++responseCount;
  size_t activeMemberSize = getValidateMemberSize();
  DVLOG(2) << "member " << msg->getSourceMemberId() << " responsed, left " << activeMemberSize - responseCount
              << " members.";
  if (responseCount == activeMemberSize) {
    DVLOG(2) << "all member responsed.";

    // response result
    DVLOG(2) << "response client.";
    globalResponse->set_result_code(resultCode);
    sendResponse(DATA_STORE_INSERT_RESPONSE, globalResponse);

    terminate();
  }
}

void DataAggregatorActor::handleGlobeUpdate(const ActorMessagePtr& msg) {
  DVLOG(2) << "multicast store update operation with actor id : " << getActorId();
  clientMsg = msg;
  responseCount = 0;
  resultCode = idgs::store::pb::SRC_SUCCESS;
  multicast(msg, ACTORID_STORE_SERVCIE, DATA_STORE_LOCAL_UPDATE);
}

void DataAggregatorActor::handleUpdateResponse(const ActorMessagePtr& msg) {
  DVLOG(2) << "receive store update response of member " << getActorId();
  idgs::store::pb::UpdateResponse* response = dynamic_cast<idgs::store::pb::UpdateResponse*>(msg->getPayload().get());

  // error
  if (response->result_code() == idgs::store::pb::SRC_NOT_LOCAL_STORE) {
    DVLOG(2) << "response by a no local store member.";
  } else if (response->result_code() != idgs::store::pb::SRC_SUCCESS) {
    LOG(ERROR)<< "error in update data to member " << msg->getSourceActorId();
    resultCode = response->result_code();
  }

    // collect response from all members and calculate size and then response to client.
  shared_ptr<idgs::store::pb::UpdateResponse> globalResponse(new idgs::store::pb::UpdateResponse);
  ++responseCount;
  size_t activeMemberSize = getValidateMemberSize();
  DVLOG(2) << "member " << msg->getSourceMemberId() << " responsed, left " << activeMemberSize - responseCount
              << " members.";
  if (responseCount == activeMemberSize) {
    DVLOG(2) << "all member responsed.";

    // response result
    DVLOG(2) << "response client.";
    globalResponse->set_result_code(resultCode);
    sendResponse(DATA_STORE_UPDATE_RESPONSE, globalResponse);

    terminate();
  }
}

void DataAggregatorActor::handleGlobeRemove(const ActorMessagePtr& msg) {
  DVLOG(2) << "multicast store remove operation with actor id : " << getActorId();
  clientMsg = msg;
  responseCount = 0;
  resultCode = idgs::store::pb::SRC_SUCCESS;
  multicast(msg, ACTORID_STORE_SERVCIE, DATA_STORE_LOCAL_REMOVE);
}

void DataAggregatorActor::handleRemoveResponse(const ActorMessagePtr& msg) {
  DVLOG(2) << "receive store remove response of member " << getActorId();
  idgs::store::pb::RemoveResponse* response = dynamic_cast<idgs::store::pb::RemoveResponse*>(msg->getPayload().get());

  // error
  if (response->result_code() == idgs::store::pb::SRC_NOT_LOCAL_STORE) {
    DVLOG(2) << "response by a no local store member.";
  } else if (response->result_code() != idgs::store::pb::SRC_SUCCESS) {
    LOG(ERROR)<< "error in update data to member " << msg->getSourceActorId();
    resultCode = response->result_code();
  }

    // collect response from all members and calculate size and then response to client.
  shared_ptr<idgs::store::pb::RemoveResponse> globalResponse(new idgs::store::pb::RemoveResponse);
  ++responseCount;
  size_t activeMemberSize = getValidateMemberSize();
  DVLOG(2) << "member " << msg->getSourceMemberId() << " responsed, left " << activeMemberSize - responseCount
              << " members.";
  if (responseCount == activeMemberSize) {
    DVLOG(2) << "all member responsed.";

    // response result
    DVLOG(2) << "response client.";
    globalResponse->set_result_code(resultCode);
    sendResponse(DATA_STORE_REMOVE_RESPONSE, globalResponse);

    terminate();
  }
}

void DataAggregatorActor::handleDataClear(const idgs::actor::ActorMessagePtr& msg) {
  DVLOG(2) << "aggregator handle data clear";
  idgs::store::pb::DataClearRequest* request = dynamic_cast<idgs::store::pb::DataClearRequest*>(msg->getPayload().get());
  clientMsg = msg;
  responseCount = 0;
  resultCode = idgs::store::pb::SRC_SUCCESS;

  ClusterFramework& cluster = ::idgs::util::singleton<ClusterFramework>::getInstance();
  size_t partSize = cluster.getPartitionCount();
  DVLOG(2) << "send message to each active actor with partition.";
  for (int32_t i = 0; i < partSize; ++i) {
    int32_t destMemberId = cluster.getPartitionManager()->getPartition(i)->getPrimaryMemberId();
    shared_ptr<idgs::store::pb::DataClearRequest> payload(new idgs::store::pb::DataClearRequest);
    payload->set_mode(request->mode());
    payload->set_partition(i);
    payload->set_store_name(request->store_name());

    ActorMessagePtr message(new ActorMessage);
    message->setOperationName(LOCAL_DATA_CLEAR);
    message->setSourceMemberId(cluster.getMemberManager()->getLocalMemberId());
    message->setSourceActorId(getActorId());
    message->setDestMemberId(destMemberId);
    message->setDestActorId(ACTORID_STORE_SERVCIE);
    message->setPayload(payload);
    ::idgs::actor::postMessage(message);
  }
}

void DataAggregatorActor::handleDataClearResponse(const idgs::actor::ActorMessagePtr& msg) {
  DVLOG(2) << "aggregator handle data clear response";
  idgs::store::pb::DataClearResponse* response =
      dynamic_cast<idgs::store::pb::DataClearResponse*>(msg->getPayload().get());

  size_t partCount = ::idgs::util::singleton<ClusterFramework>::getInstance().getPartitionCount();
  ++responseCount;
  DVLOG(2) << "receive " << responseCount << " partition response, left " << (partCount - responseCount)
              << ". current state is " << response->result_code();
  if (response->result_code() == idgs::store::pb::SRC_NOT_LOCAL_STORE) {
    DVLOG(2) << "response by a no local store member.";
  } else if (response->result_code() != idgs::store::pb::SRC_SUCCESS) {
    LOG(ERROR)<< "Data clear error at member " << msg->getSourceMemberId();
    resultCode = response->result_code();
    // collect response from all members and calculate size and then response to client.
  } else {
    shared_ptr<idgs::store::pb::DataClearResponse> globalResponse(new idgs::store::pb::DataClearResponse);
    if (responseCount == partCount) {
      globalResponse->set_result_code(resultCode);

      // response result
      DVLOG(2) << "response client.";
      sendResponse(DATA_CLEAR_RESPONSE, globalResponse);

      terminate();
    }
  }
}

const idgs::actor::ActorMessageHandlerMap& DataSizeAggregatorActor::getMessageHandlerMap() const {
  static std::map<std::string, idgs::actor::ActorMessageHandler> handlerMap = {
      {DATA_STORE_SIZE_RESPONSE, static_cast<idgs::actor::ActorMessageHandler>(&DataSizeAggregatorActor::handleSizeResponse)},
      {OP_COUNT, static_cast<idgs::actor::ActorMessageHandler>(&DataSizeAggregatorActor::handleGlobeSize)}
  };

  return handlerMap;
}

ActorDescriptorPtr DataSizeAggregatorActor::generateActorDescriptor() {
  static std::shared_ptr<ActorDescriptorWrapper> descriptor;
  if (descriptor)
    return descriptor;
  descriptor.reset(new ::ActorDescriptorWrapper);

  descriptor->setName(DATA_SIZE_AGGREGATOR_ACTOR);
  descriptor->setDescription("Aggregator of count store size with the given store name");
  descriptor->setType(AT_STATEFUL);

  // operation name : DATA_STORE_SIZE
  ActorOperationDescriporWrapper dataRequest;
  dataRequest.setName(OP_COUNT);
  dataRequest.setDescription("Receive count store size request");
  dataRequest.setPayloadType("idgs.store.pb.SizeRequest");
  descriptor->setInOperation(dataRequest.getName(), dataRequest);

  ActorOperationDescriporWrapper dataLocalRequest;
  dataLocalRequest.setName(DATA_STORE_LOCAL_SIZE);
  dataLocalRequest.setDescription("Route count store size request");
  dataLocalRequest.setPayloadType("idgs.store.pb.SizeRequest");
  descriptor->setOutOperation(dataLocalRequest.getName(), dataLocalRequest);

  // operation name : DATA_STORE_LOCAL_SIZE
  ActorOperationDescriporWrapper dataReponse;
  dataReponse.setName(DATA_STORE_SIZE_RESPONSE);
  dataReponse.setDescription("Collect count store size response, make the total size and response to client.");
  dataReponse.setPayloadType("idgs.store.pb.SizeResponse");
  descriptor->setInOperation(dataReponse.getName(), dataReponse);
  descriptor->setOutOperation(dataReponse.getName(), dataReponse);

  DataSizeAggregatorActor::descriptor = descriptor;

  return descriptor;
}

const ActorDescriptorPtr& DataSizeAggregatorActor::getDescriptor() const {
  return DataSizeAggregatorActor::descriptor;
}

void DataSizeAggregatorActor::handleSizeResponse(const idgs::actor::ActorMessagePtr& msg) {
  DVLOG(2) << "receive store size of member " << msg->getSourceActorId();
  idgs::store::pb::SizeResponse* response = dynamic_cast<idgs::store::pb::SizeResponse*>(msg->getPayload().get());

  // error
  if (response->result_code() != idgs::store::pb::SRC_SUCCESS) {
    DVLOG(2) << "error at member " << msg->getSourceActorId();
    resultCode = response->result_code();
    // collect response from all members and calculate size and then response to client.
  } else {
    shared_ptr<idgs::store::pb::SizeResponse> globalResponse(new idgs::store::pb::SizeResponse);
    dataSizeMap[response->partition()] = response->size();
    size_t partCount = ::idgs::util::singleton<ClusterFramework>::getInstance().getPartitionCount();
    if (dataSizeMap.size() == partCount) {
      DVLOG(2) << "aggregate done.";
      if (resultCode != idgs::store::pb::SRC_SUCCESS) {
        globalResponse->set_result_code(resultCode);
      } else {
        // calculate size
        size_t size = 0;
        map<int32_t, size_t>::iterator it = dataSizeMap.begin();
        for (; it != dataSizeMap.end(); ++it) {
          size += (*it).second;
        }

        globalResponse->set_result_code(idgs::store::pb::SRC_SUCCESS);
        globalResponse->set_size(size);
      }

      // response result
      DVLOG(2) << "response client.";
      sendResponse(DATA_STORE_SIZE_RESPONSE, globalResponse);

      terminate();
    }
  }
}

void DataSizeAggregatorActor::handleGlobeSize(const idgs::actor::ActorMessagePtr& msg) {
  DVLOG(2) << "starting aggregator of handle count data to store.";
  // remember client info
  idgs::store::pb::SizeRequest* request = dynamic_cast<idgs::store::pb::SizeRequest*>(msg->getPayload().get());
  clientMsg = msg;

  // send DATA_STORE_SIZE to each active member.
  ClusterFramework& cluster = ::idgs::util::singleton<ClusterFramework>::getInstance();
  size_t partSize = cluster.getPartitionCount();
  DVLOG(2) << "send message to each active actor.";
  for (size_t i = 0; i < partSize; ++i) {
    int32_t destMemberId = cluster.getPartitionManager()->getPartition(i)->getPrimaryMemberId();
    shared_ptr<idgs::store::pb::SizeRequest> payload(new idgs::store::pb::SizeRequest);
    payload->set_store_name(request->store_name());
    payload->set_options(request->options());
    payload->set_partition(i);

    ActorMessagePtr message(new ActorMessage);
    message->setOperationName(LOCAL_DATA_CLEAR);
    message->setSourceMemberId(cluster.getMemberManager()->getLocalMemberId());
    message->setSourceActorId(getActorId());
    message->setDestMemberId(destMemberId);
    message->setDestActorId(ACTORID_STORE_SERVCIE);
    message->setPayload(payload);
    ::idgs::actor::postMessage(message);
  }
}

} // namespace store
} // namespace idgs
