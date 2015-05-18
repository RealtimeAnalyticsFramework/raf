
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "aggregator_actor.h"

#include "idgs/application.h"


using namespace idgs::pb;
using namespace idgs::actor;
using namespace google::protobuf;

namespace idgs {
namespace store {

void AggregatorActor::sendResponse(const std::string& opName, const shared_ptr<Message>& response) {
  ActorMessagePtr am = clientMsg->createResponse();
  am->setOperationName(opName);
  am->setPayload(response);
  idgs::actor::sendMessage(am);
}

void AggregatorActor::multicast(const ActorMessagePtr& msg, const string& actorId, const std::string& opName) {
  requestCount = 0;
  responseCount = 0;

  auto memberMgr = idgs_application()->getMemberManager();
  auto& members = memberMgr->getMemberTable();
  auto it = members.begin();
  for (; it != members.end(); ++ it) {
    if (it->isLocalStore() && (it->getState() == idgs::pb::MS_ACTIVE || it->getState() == idgs::pb::MS_PREPARED)) {
      ActorMessagePtr multMsg = msg->createRouteMessage(it->getId(), actorId);
      multMsg->setOperationName(opName);
      multMsg->setSourceActorId(this->getActorId());
      multMsg->setSourceMemberId(memberMgr->getLocalMemberId());
      ::idgs::actor::postMessage(multMsg);
      ++ requestCount;
    }
  }
}

ActorDescriptorPtr DataAggregatorActor::descriptor;

const idgs::actor::ActorMessageHandlerMap& DataAggregatorActor::getMessageHandlerMap() const {
  static idgs::actor::ActorMessageHandlerMap handlerMap = {
      {OP_INSERT_RESPONSE,       static_cast<idgs::actor::ActorMessageHandler>(&DataAggregatorActor::handleInsertResponse)},
      {OP_INTERNAL_INSERT,       static_cast<idgs::actor::ActorMessageHandler>(&DataAggregatorActor::handleGlobalInsert)},
      {OP_UPDATE_RESPONSE,       static_cast<idgs::actor::ActorMessageHandler>(&DataAggregatorActor::handleUpdateResponse)},
      {OP_INTERNAL_UPDATE,       static_cast<idgs::actor::ActorMessageHandler>(&DataAggregatorActor::handleGlobalUpdate)},
      {OP_DELETE_RESPONSE,       static_cast<idgs::actor::ActorMessageHandler>(&DataAggregatorActor::handleDeleteResponse)},
      {OP_INTERNAL_DELETE,       static_cast<idgs::actor::ActorMessageHandler>(&DataAggregatorActor::handleGlobalDelete)},
      {OP_TRUNCATE_RESPONSE,     static_cast<idgs::actor::ActorMessageHandler>(&DataAggregatorActor::handleTruncateResponse)},
      {OP_INTERNAL_TRUNCATE,     static_cast<idgs::actor::ActorMessageHandler>(&DataAggregatorActor::handleGlobalTruncate)},
      {OP_INTERNAL_COUNT,        static_cast<idgs::actor::ActorMessageHandler>(&DataAggregatorActor::handleGlobalCount)},
      {OP_COUNT_RESPONSE,        static_cast<idgs::actor::ActorMessageHandler>(&DataAggregatorActor::handleCountResponse)}
  };

  return handlerMap;
}

ActorDescriptorPtr DataAggregatorActor::generateActorDescriptor() {
  static std::shared_ptr<ActorDescriptorWrapper> descriptor;
  if (descriptor) {
    return descriptor;
  }

  descriptor = make_shared<ActorDescriptorWrapper>();

  descriptor->setName(DATA_AGGREGATOR_ACTOR);
  descriptor->setDescription("Data CRUD aggregator for replicated store.");
  descriptor->setType(AT_STATEFUL);

  // insert
  ActorOperationDescriporWrapper internalInsert;
  internalInsert.setName(OP_INTERNAL_INSERT);
  internalInsert.setDescription("Receive the sync insert data request");
  internalInsert.setPayloadType("idgs.store.pb.InsertRequest");
  descriptor->setInOperation(internalInsert.getName(), internalInsert);
  descriptor->setOutOperation(internalInsert.getName(), internalInsert);

  ActorOperationDescriporWrapper insertResponse;
  insertResponse.setName(OP_INSERT_RESPONSE);
  insertResponse.setDescription("Receive insert response of one member, when received all members response, response to client.");
  insertResponse.setPayloadType("idgs.store.pb.InsertResponse");
  descriptor->setInOperation(insertResponse.getName(), insertResponse);
  descriptor->setOutOperation(insertResponse.getName(), insertResponse);

  // update
  ActorOperationDescriporWrapper internalUpdate;
  internalUpdate.setName(OP_INTERNAL_UPDATE);
  internalUpdate.setDescription("Receive the sync update data request");
  internalUpdate.setPayloadType("idgs.store.pb.UpdateRequest");
  descriptor->setInOperation(internalUpdate.getName(), internalUpdate);
  descriptor->setOutOperation(internalUpdate.getName(), internalUpdate);

  ActorOperationDescriporWrapper updateResponse;
  updateResponse.setName(OP_UPDATE_RESPONSE);
  updateResponse.setDescription("Receive update response of one member, when received all members response, response to client.");
  updateResponse.setPayloadType("idgs.store.pb.UpdateResponse");
  descriptor->setInOperation(updateResponse.getName(), updateResponse);
  descriptor->setOutOperation(updateResponse.getName(), updateResponse);

  // delete
  ActorOperationDescriporWrapper internalDelete;
  internalDelete.setName(OP_INTERNAL_DELETE);
  internalDelete.setDescription("Receive the sync delete data request");
  internalDelete.setPayloadType("idgs.store.pb.DeleteRequest");
  descriptor->setInOperation(internalDelete.getName(), internalDelete);
  descriptor->setOutOperation(internalDelete.getName(), internalDelete);

  ActorOperationDescriporWrapper deleteResponse;
  deleteResponse.setName(OP_DELETE_RESPONSE);
  deleteResponse.setDescription("Receive delete response of one member, when received all members response, response to client.");
  deleteResponse.setPayloadType("idgs.store.pb.DeleteResponse");
  descriptor->setInOperation(deleteResponse.getName(), deleteResponse);
  descriptor->setOutOperation(deleteResponse.getName(), deleteResponse);

  // truncate
  ActorOperationDescriporWrapper internalTruncate;
  internalTruncate.setName(OP_INTERNAL_TRUNCATE);
  internalTruncate.setDescription("Handle truncate store request message");
  internalTruncate.setPayloadType("idgs.store.pb.TruncateRequest");
  descriptor->setInOperation(internalTruncate.getName(), internalTruncate);
  descriptor->setOutOperation(internalTruncate.getName(), internalTruncate);

  ActorOperationDescriporWrapper truncateResponse;
  truncateResponse.setName(OP_TRUNCATE_RESPONSE);
  truncateResponse.setDescription("Aggregate truncate store response. and send result to client");
  truncateResponse.setPayloadType("idgs.store.pb.TruncateResponse");
  descriptor->setInOperation(truncateResponse.getName(), truncateResponse);
  descriptor->setOutOperation(truncateResponse.getName(), truncateResponse);

  // count
  ActorOperationDescriporWrapper internalCount;
  internalCount.setName(OP_INTERNAL_COUNT);
  internalCount.setDescription("Receive count store size request");
  internalCount.setPayloadType("idgs.store.pb.SizeRequest");
  descriptor->setInOperation(internalCount.getName(), internalCount);
  descriptor->setOutOperation(internalCount.getName(), internalCount);

  ActorOperationDescriporWrapper countReponse;
  countReponse.setName(OP_COUNT_RESPONSE);
  countReponse.setDescription("Collect count store size response, make the total size and response to client.");
  countReponse.setPayloadType("idgs.store.pb.SizeResponse");
  descriptor->setInOperation(countReponse.getName(), countReponse);
  descriptor->setOutOperation(countReponse.getName(), countReponse);

  // local access descriptor
  DataAggregatorActor::descriptor = descriptor;

  return descriptor;
}

const ActorDescriptorPtr& DataAggregatorActor::getDescriptor() const {
  return DataAggregatorActor::descriptor;
}

void DataAggregatorActor::handleGlobalInsert(const ActorMessagePtr& msg) {
  DVLOG_FIRST_N(2, 20) << "multicast store insert operation with actor id : " << getActorId();
  clientMsg = msg;
  resultCode = idgs::store::pb::SRC_SUCCESS;
  multicast(msg, ACTORID_STORE_SERVCIE, OP_INTERNAL_INSERT);
}

void DataAggregatorActor::handleInsertResponse(const ActorMessagePtr& msg) {
  DVLOG_FIRST_N(2, 20) << "receive store insert response of member " << msg->getSourceMemberId();
  idgs::store::pb::InsertResponse* response = dynamic_cast<idgs::store::pb::InsertResponse*>(msg->getPayload().get());

  // error
  if (response->result_code() != idgs::store::pb::SRC_SUCCESS) {
    resultCode = response->result_code();
    LOG(ERROR)<< "error in insert data to member " << msg->getSourceActorId() << " error code : " << resultCode;
  }

  // collect response from all members and calculate size and then response to client.
  ++ responseCount;
  DVLOG_FIRST_N(2, 20) << "member " << msg->getSourceMemberId() << " responsed, left " << requestCount - responseCount << " members.";
  if (requestCount == responseCount) {
    DVLOG_FIRST_N(2, 20) << "all member responsed, send response to client";
    shared_ptr<idgs::store::pb::InsertResponse> globalResponse = make_shared<idgs::store::pb::InsertResponse>();
    globalResponse->set_result_code(resultCode);
    sendResponse(OP_INSERT_RESPONSE, globalResponse);

    terminate();
  }
}

void DataAggregatorActor::handleGlobalUpdate(const ActorMessagePtr& msg) {
  DVLOG_FIRST_N(2, 20) << "multicast store update operation with actor id : " << getActorId();
  clientMsg = msg;
  resultCode = idgs::store::pb::SRC_SUCCESS;
  multicast(msg, ACTORID_STORE_SERVCIE, OP_INTERNAL_UPDATE);
}

void DataAggregatorActor::handleUpdateResponse(const ActorMessagePtr& msg) {
  DVLOG_FIRST_N(2, 20) << "receive store update response of member " << getActorId();
  idgs::store::pb::UpdateResponse* response = dynamic_cast<idgs::store::pb::UpdateResponse*>(msg->getPayload().get());

  // error
  if (response->result_code() != idgs::store::pb::SRC_SUCCESS) {
    LOG(ERROR)<< "error in update data to member " << msg->getSourceActorId();
    resultCode = response->result_code();
  }

  ++ responseCount;
  DVLOG_FIRST_N(2, 20) << "member " << msg->getSourceMemberId() << " responsed, left " << requestCount - responseCount << " members.";
  if (requestCount == responseCount) {
    DVLOG_FIRST_N(2, 20) << "all member responsed, send response to client";

    shared_ptr<idgs::store::pb::UpdateResponse> globalResponse = make_shared<idgs::store::pb::UpdateResponse>();
    globalResponse->set_result_code(resultCode);
    sendResponse(OP_UPDATE_RESPONSE, globalResponse);

    terminate();
  }
}

void DataAggregatorActor::handleGlobalDelete(const ActorMessagePtr& msg) {
  DVLOG_FIRST_N(2, 20) << "multicast store remove operation with actor id : " << getActorId();
  clientMsg = msg;
  resultCode = idgs::store::pb::SRC_SUCCESS;
  multicast(msg, ACTORID_STORE_SERVCIE, OP_INTERNAL_DELETE);
}

void DataAggregatorActor::handleDeleteResponse(const ActorMessagePtr& msg) {
  DVLOG_FIRST_N(2, 20) << "receive store remove response of member " << getActorId();
  auto response = dynamic_cast<idgs::store::pb::DeleteResponse*>(msg->getPayload().get());

  // error
  if (response->result_code() != idgs::store::pb::SRC_SUCCESS) {
    LOG(ERROR)<< "error in remove data to member " << msg->getSourceActorId();
    resultCode = response->result_code();
  }

  ++ responseCount;
  DVLOG_FIRST_N(2, 20) << "member " << msg->getSourceMemberId() << " response, left " << requestCount - responseCount << " members.";
  if (requestCount == responseCount) {
    DVLOG_FIRST_N(2, 20) << "all member response, send response to client.";

    auto globalResponse = make_shared<idgs::store::pb::DeleteResponse>();
    globalResponse->set_result_code(resultCode);
    sendResponse(OP_DELETE_RESPONSE, globalResponse);

    terminate();
  }
}

void DataAggregatorActor::handleGlobalTruncate(const idgs::actor::ActorMessagePtr& msg) {
  DVLOG_FIRST_N(2, 20) << "multicast store truncate operation with actor id : " << getActorId();
  clientMsg = msg;
  resultCode = idgs::store::pb::SRC_SUCCESS;
  multicast(msg, ACTORID_STORE_SERVCIE, OP_INTERNAL_TRUNCATE);
}

void DataAggregatorActor::handleTruncateResponse(const idgs::actor::ActorMessagePtr& msg) {
  DVLOG_FIRST_N(2, 20) << "aggregator handle truncate store response";
  idgs::store::pb::TruncateResponse* response = dynamic_cast<idgs::store::pb::TruncateResponse*>(msg->getPayload().get());

  if (response->result_code() != idgs::store::pb::SRC_SUCCESS) {
    LOG(ERROR)<< "Truncate store error at member " << msg->getSourceMemberId();
    resultCode = response->result_code();
  }

  ++ responseCount;
  DVLOG_FIRST_N(2, 20) << "member " << msg->getSourceMemberId() << " response, left " << requestCount - responseCount << " members.";
  if (requestCount == responseCount) {
    DVLOG_FIRST_N(2, 20) << "all member responsed, send response to client.";

    shared_ptr<idgs::store::pb::TruncateResponse> globalResponse = make_shared<idgs::store::pb::TruncateResponse>();
    globalResponse->set_result_code(resultCode);
    sendResponse(OP_TRUNCATE_RESPONSE, globalResponse);

    terminate();
  }
}

void DataAggregatorActor::handleGlobalCount(const idgs::actor::ActorMessagePtr& msg) {
  DVLOG_FIRST_N(2, 20) << "starting aggregator of handle count data to store.";
  clientMsg = msg;
  multicast(msg, ACTORID_STORE_SERVCIE, OP_INTERNAL_COUNT);
}

void DataAggregatorActor::handleCountResponse(const idgs::actor::ActorMessagePtr& msg) {
  DVLOG_FIRST_N(2, 20) << "receive store size of member " << msg->getSourceActorId();
  idgs::store::pb::SizeResponse* response = dynamic_cast<idgs::store::pb::SizeResponse*>(msg->getPayload().get());

  if (response->result_code() != idgs::store::pb::SRC_SUCCESS) {
    // error
    DVLOG_FIRST_N(2, 20) << "error at member " << msg->getSourceActorId();
    resultCode = response->result_code();
  }

  // collect response from all members and calculate size and then response to client.
  shared_ptr<idgs::store::pb::SizeResponse> globalResponse = make_shared<idgs::store::pb::SizeResponse>();
  dataSizeMap[msg->getSourceMemberId()] = response->size();
  if (dataSizeMap.size() == requestCount) {
    DVLOG_FIRST_N(2, 20) << "aggregate done.";
    if (resultCode != idgs::store::pb::SRC_SUCCESS) {
      globalResponse->set_result_code(resultCode);
    } else {
      // calculate size
      size_t size = 0;
      map<int32_t, size_t>::iterator it = dataSizeMap.begin();
      for (; it != dataSizeMap.end(); ++ it) {
        size += (* it).second;
      }

      globalResponse->set_result_code(idgs::store::pb::SRC_SUCCESS);
      globalResponse->set_size(size);
    }

    // response result
    DVLOG_FIRST_N(2, 20) << "response client.";
    sendResponse(OP_COUNT_RESPONSE, globalResponse);

    terminate();
  }
}

} // namespace store
} // namespace idgs
