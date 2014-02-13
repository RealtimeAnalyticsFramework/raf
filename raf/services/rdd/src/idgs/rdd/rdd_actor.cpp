
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "rdd_actor.h"
#include "idgs/store/metadata_helper.h"
#include "idgs/cluster/cluster_framework.h"
#include "protobuf/message_helper.h"
#include "idgs/store/datastore_const.h"

using namespace std;
using namespace idgs::actor;
using namespace idgs::cluster;
using namespace idgs::rdd::pb;

namespace idgs {
namespace rdd {

ActorDescriptorPtr RddActor::descriptor;

RddActor::RddActor() {
}

RddActor::~RddActor() {
}

const ActorMessageHandlerMap& RddActor::getMessageHandlerMap() const {
  static std::map<std::string, idgs::actor::ActorMessageHandler> handlerMap = {
      {CREATE_RDD_PARTITION_RESPONSE,           static_cast<idgs::actor::ActorMessageHandler>(&RddActor::handleRddCreatePartitionResponse)},
      {PARTITION_PREPARED_RESPONSE,             static_cast<idgs::actor::ActorMessageHandler>(&RddActor::handlePartitionPreparedResposne)},
      {GET_PARTITION_ACTOR,                     static_cast<idgs::actor::ActorMessageHandler>(&RddActor::handleGetPartitionActor)},
      {GET_PARTITION_ACTOR_RESPONSE,            static_cast<idgs::actor::ActorMessageHandler>(&RddActor::handleGetPartitionActorResponse)},
      {RECEIVE_DEPENDING_RDD,                   static_cast<idgs::actor::ActorMessageHandler>(&RddActor::handleReceiveDependedRdd)},
      {RDD_TRANSFORM,                           static_cast<idgs::actor::ActorMessageHandler>(&RddActor::handleRddTransform)},
      {RDD_READY,                               static_cast<idgs::actor::ActorMessageHandler>(&RddActor::handleDependingRddReady)},
      {RDD_PARTITION_READY,                     static_cast<idgs::actor::ActorMessageHandler>(&RddActor::handleRddPartitionReady)},
      {ACTION_MESSAGE_PROCESS,                  static_cast<idgs::actor::ActorMessageHandler>(&RddActor::handleActionProcess)},
      {RDD_ACTION_REQUEST,                      static_cast<idgs::actor::ActorMessageHandler>(&RddActor::handleRddActionRequest)},
      {RDD_ACTION_RESPONSE,                     static_cast<idgs::actor::ActorMessageHandler>(&RddActor::handleRddActionResponse)},
      {SEND_RDD_INFO_RESPONSE,                  static_cast<idgs::actor::ActorMessageHandler>(&RddActor::handleSendRddInfoResponse)},
      {PARTITION_TRANSFORM_COMPLETE,            static_cast<idgs::actor::ActorMessageHandler>(&RddActor::handleTransformComplete)},
      {RDD_STATE_REQUEST,                       static_cast<idgs::actor::ActorMessageHandler>(&RddActor::handleRddStateRequest)},
      {RDD_STATE_RESPONSE,                      static_cast<idgs::actor::ActorMessageHandler>(&RddActor::handleRddStateResponse)},
      {idgs::store::DATA_STORE_INSERT_RESPONSE, static_cast<idgs::actor::ActorMessageHandler>(&RddActor::handleInsertRDDInfoResponse)},
      {CREATE_RDD,                              static_cast<idgs::actor::ActorMessageHandler>(&RddActor::handleRddCreate)}
  };
  return handlerMap;
}

ActorDescriptorPtr RddActor::generateActorDescriptor() {
  static std::shared_ptr<ActorDescriptorWrapper> descriptor;
  if (descriptor)
    return descriptor;

  descriptor = generateBaseActorDescriptor(RDD_ACTOR);

  // in operation
  ActorOperationDescriporWrapper inCreateRdd;
  inCreateRdd.setName(CREATE_RDD);
  inCreateRdd.setDescription("Create RDD.");
  inCreateRdd.setPayloadType("idgs.rdd.pb.CreateRddRequest");
  descriptor->setInOperation(inCreateRdd.getName(), inCreateRdd);

  // out operation
  // out operation for CREATE_RDD
  ActorOperationDescriporWrapper inGetPartitionActor;
  inGetPartitionActor.setName(GET_PARTITION_ACTOR);
  inGetPartitionActor.setDescription("Out operation for Create RDD, to get the partition actor id of depending rdds");
  inGetPartitionActor.setPayloadType("idgs.rdd.pb.RddRequest");
  descriptor->setOutOperation(inGetPartitionActor.getName(), inGetPartitionActor);

  // out descriptor for message RDD_TRANSFORM, RDD_READY
  ActorOperationDescriporWrapper outRddPartitionProcess;
  outRddPartitionProcess.setName(RDD_PARTITION_PROCESS);
  outRddPartitionProcess.setDescription("out message for RDD_TRANSFORM, RDD_READY");
  outRddPartitionProcess.setPayloadType("idgs.rdd.pb.CreateRddRequest");
  descriptor->setOutOperation(outRddPartitionProcess.getName(), outRddPartitionProcess);

  RddActor::descriptor = descriptor;
  return descriptor;
}

const ActorDescriptorPtr& RddActor::getDescriptor() const {
  return RddActor::descriptor;
}

void RddActor::handleRddCreate(const ActorMessagePtr& msg) {
  DVLOG(2) << "filter rdd : handle create";
  rawMsg = msg;
  CreateRddRequest* request = dynamic_cast<CreateRddRequest*>(msg->getPayload().get());

  string key_type = request->out_rdd().key_type_name();
  string value_type = request->out_rdd().value_type_name();

  protobuf::MessageHelper& helper = ::idgs::util::singleton<protobuf::MessageHelper>::getInstance();
  if (helper.isMessageRegistered(key_type)) {
    auto key = helper.createMessage(key_type);
    idgs::store::MetadataHelper::messageToMetadata(key.get(), metadata->mutable_key());
  } else {
    metadata->mutable_key()->set_type_name(key_type);
    for (int32_t i = 0; i < request->out_rdd().key_fields_size(); ++ i) {
      auto key_field = request->out_rdd().key_fields(i);
      auto fld = metadata->mutable_key()->add_field();
      fld->set_label(idgs::store::pb::LABEL_REQUIRED);
      fld->set_type(key_field.field_type());
      fld->set_name(key_field.field_name());
      fld->set_number(i + 1);
    }
  }

  if (helper.isMessageRegistered(value_type)) {
    auto value = helper.createMessage(value_type);
    idgs::store::MetadataHelper::messageToMetadata(value.get(), metadata->mutable_value());
  } else {
    metadata->mutable_value()->set_type_name(value_type);
    for (int32_t i = 0; i < request->out_rdd().value_fields_size(); ++ i) {
      auto value_field = request->out_rdd().value_fields(i);
      auto fld = metadata->mutable_value()->add_field();
      fld->set_label(idgs::store::pb::LABEL_OPTIONAL);
      fld->set_type(value_field.field_type());
      fld->set_name(value_field.field_name());
      fld->set_number(i + 1);
    }
  }

  ClusterFramework& cluster = ::idgs::util::singleton<ClusterFramework>::getInstance();
  for (size_t partition = 0; partition < partitionSize; ++partition) {
    int32_t memberId = cluster.getPartitionManager()->getPartition(partition)->getPrimaryMemberId();
    shared_ptr<CreateRddPartitionRequest> payload(new CreateRddPartitionRequest);
    payload->set_partition(partition);
    payload->set_rdd_name(getRddName());
    payload->mutable_rdd_id()->set_member_id(localMemberId);
    payload->mutable_rdd_id()->set_actor_id(getActorId());

    ActorMessagePtr message = createActorMessage();
    message->setOperationName(CREATE_RDD_PARTITION);
    message->setDestActorId(RDD_SERVICE_ACTOR);
    message->setDestMemberId(memberId);
    message->setPayload(payload);
//        message->setAttachment(TRANSFORM_REQUEST, rawMsg->getPayload());

    idgs::actor::postMessage(message);
  }
}

} // namespace rdd
} // namespace idgs 
