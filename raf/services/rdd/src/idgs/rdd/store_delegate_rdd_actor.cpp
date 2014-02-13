
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "store_delegate_rdd_actor.h"
#include "idgs/store/metadata_helper.h"
#include "idgs/cluster/cluster_framework.h"
#include "idgs/store/datastore_const.h"

using namespace idgs::actor;
using namespace idgs::cluster;
using namespace idgs::rdd::pb;
using namespace google::protobuf;

namespace idgs {
namespace rdd {

ActorDescriptorPtr StoreDelegateRddActor::descriptor;

StoreDelegateRddActor::StoreDelegateRddActor() {
}

StoreDelegateRddActor::~StoreDelegateRddActor() {
}

const ActorMessageHandlerMap& StoreDelegateRddActor::getMessageHandlerMap() const {
  static std::map<std::string, idgs::actor::ActorMessageHandler> handlerMap = {
      {CREATE_RDD_PARTITION_RESPONSE, static_cast<idgs::actor::ActorMessageHandler>(&StoreDelegateRddActor::handleRddCreatePartitionResponse)},
      {PARTITION_PREPARED_RESPONSE, static_cast<idgs::actor::ActorMessageHandler>(&StoreDelegateRddActor::handlePartitionPreparedResposne)},
      {GET_PARTITION_ACTOR, static_cast<idgs::actor::ActorMessageHandler>(&StoreDelegateRddActor::handleGetPartitionActor)},
      {GET_PARTITION_ACTOR_RESPONSE, static_cast<idgs::actor::ActorMessageHandler>(&StoreDelegateRddActor::handleGetPartitionActorResponse)},
      {RECEIVE_DEPENDING_RDD, static_cast<idgs::actor::ActorMessageHandler>(&StoreDelegateRddActor::handleReceiveDependedRdd)},
      {RDD_TRANSFORM, static_cast<idgs::actor::ActorMessageHandler>(&StoreDelegateRddActor::handleRddTransform)},
      {RDD_READY, static_cast<idgs::actor::ActorMessageHandler>(&StoreDelegateRddActor::handleDependingRddReady)},
      {RDD_PARTITION_READY, static_cast<idgs::actor::ActorMessageHandler>(&StoreDelegateRddActor::handleRddPartitionReady)},
      {ACTION_MESSAGE_PROCESS, static_cast<idgs::actor::ActorMessageHandler>(&StoreDelegateRddActor::handleActionProcess)},
      {RDD_ACTION_REQUEST, static_cast<idgs::actor::ActorMessageHandler>(&StoreDelegateRddActor::handleRddActionRequest)},
      {RDD_ACTION_RESPONSE, static_cast<idgs::actor::ActorMessageHandler>(&StoreDelegateRddActor::handleRddActionResponse)},
      {SEND_RDD_INFO_RESPONSE, static_cast<idgs::actor::ActorMessageHandler>(&StoreDelegateRddActor::handleSendRddInfoResponse)},
      {PARTITION_TRANSFORM_COMPLETE, static_cast<idgs::actor::ActorMessageHandler>(&StoreDelegateRddActor::handleTransformComplete)},
      {RDD_STATE_REQUEST, static_cast<idgs::actor::ActorMessageHandler>(&StoreDelegateRddActor::handleRddStateRequest)},
      {RDD_STATE_RESPONSE, static_cast<idgs::actor::ActorMessageHandler>(&StoreDelegateRddActor::handleRddStateResponse)},
      {idgs::store::DATA_STORE_INSERT_RESPONSE, static_cast<idgs::actor::ActorMessageHandler>(&StoreDelegateRddActor::handleInsertRDDInfoResponse)},
      {CREATE_STORE_DELEGATE_RDD, static_cast<idgs::actor::ActorMessageHandler>(&StoreDelegateRddActor::handleRddCreate)}
  };
  return handlerMap;
}

ActorDescriptorPtr StoreDelegateRddActor::generateActorDescriptor() {
  static std::shared_ptr<ActorDescriptorWrapper> descriptor;
  if (descriptor)
    return descriptor;

  descriptor = generateBaseActorDescriptor(STORE_DELEGATE_RDD_ACTOR);

  // in operation
  ActorOperationDescriporWrapper inCreateDelegateRdd;
  inCreateDelegateRdd.setName(CREATE_STORE_DELEGATE_RDD);
  inCreateDelegateRdd.setDescription("Create store delegate RDD.");
  inCreateDelegateRdd.setPayloadType("idgs.rdd.pb.CreateDelegateRddRequest");
  descriptor->setInOperation(inCreateDelegateRdd.getName(), inCreateDelegateRdd);

  // out operation
  // out operation for CREATE_STORE_DELEGATE_RDD
  ActorOperationDescriporWrapper inCreateDelegatePartition;
  inCreateDelegatePartition.setName(CREATE_DELEGATE_PARTITION);
  inCreateDelegatePartition.setDescription("Out for create store delegate RDD, to create delegate partition.");
  inCreateDelegatePartition.setPayloadType("idgs.rdd.pb.CreateDelegatePartitionRequest");
  descriptor->setOutOperation(inCreateDelegatePartition.getName(), inCreateDelegatePartition);

  // out descriptor for message RDD_TRANSFORM, RDD_READY
  ActorOperationDescriporWrapper outRddPartitionProcess;
  outRddPartitionProcess.setName(RDD_PARTITION_PROCESS);
  outRddPartitionProcess.setDescription("out message for RDD_TRANSFORM, RDD_READY");
  outRddPartitionProcess.setPayloadType("idgs.rdd.pb.CreateDelegateRddRequest");
  descriptor->setOutOperation(outRddPartitionProcess.getName(), outRddPartitionProcess);

  StoreDelegateRddActor::descriptor = descriptor;
  return descriptor;
}

const idgs::actor::ActorDescriptorPtr& StoreDelegateRddActor::getDescriptor() const {
  return StoreDelegateRddActor::descriptor;
}

void StoreDelegateRddActor::handleRddCreate(const ActorMessagePtr& msg) {
  DVLOG(2) << "StoreDelegateRddActor : handle create store delegate.";
  rawMsg = msg;
  CreateDelegateRddRequest* request = dynamic_cast<CreateDelegateRddRequest*>(msg->getPayload().get());

  idgs::store::MetadataHelper::loadStoreMetadata(request->store_name(), metadata.get());

  ClusterFramework& cluster = ::idgs::util::singleton<ClusterFramework>::getInstance();
  for (int32_t partition = 0; partition < partitionSize; ++partition) {
    int32_t memberId = cluster.getPartitionManager()->getPartition(partition)->getPrimaryMemberId();
    shared_ptr<CreateDelegatePartitionRequest> payload(new CreateDelegatePartitionRequest);
    payload->set_store_name(request->store_name());
    payload->set_partition(partition);
    payload->set_rdd_name(getRddName());

    ActorMessagePtr reqMsg = createActorMessage();
    reqMsg->setOperationName(CREATE_DELEGATE_PARTITION);
    reqMsg->setDestActorId(RDD_SERVICE_ACTOR);
    reqMsg->setDestMemberId(memberId);
    reqMsg->setPayload(payload);

    DVLOG(3) << "RDD \"" << getRddName() << "\" sending create RDD partition to member " << memberId;

    ::idgs::actor::postMessage(reqMsg);
  }
}

} // namespace rdd
} // namespace idgs 
