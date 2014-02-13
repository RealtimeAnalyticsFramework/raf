
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/rdd/rdd_service_actor.h"
#include "idgs/cluster/cluster_framework.h"
#include "idgs/store/datastore_const.h"
#include "idgs/store/data_store.h"
#include "idgs/rdd/rdd_actor.h"
#include "idgs/rdd/rdd_partition.h"
#include "idgs/rdd/store_delegate_rdd_actor.h"
#include "idgs/rdd/store_delegate_rdd_partition.h"

#include "protobuf/message_helper.h"


using namespace idgs::pb;
using namespace idgs::actor;
using namespace idgs::store;
using namespace idgs::store::pb;
using namespace idgs::rdd::pb;
using namespace idgs::cluster;
using namespace idgs::util;
using namespace protobuf;
using namespace google::protobuf;

namespace idgs {
namespace rdd {

RddServiceActor::RddServiceActor() {
  this->actorId = RDD_SERVICE_ACTOR;

  descriptor = RddServiceActor::generateActorDescriptor();
}

RddServiceActor::~RddServiceActor() {
}

const ActorMessageHandlerMap& RddServiceActor::getMessageHandlerMap() const {
  static std::map<std::string, idgs::actor::ActorMessageHandler> handlerMap = {
      {CREATE_STORE_DELEGATE_RDD, static_cast<idgs::actor::ActorMessageHandler>(&RddServiceActor::handleCreateStoreDelegate)},
      {CREATE_DELEGATE_PARTITION, static_cast<idgs::actor::ActorMessageHandler>(&RddServiceActor::handleCreateDelegatePartition)},
      {CREATE_RDD, static_cast<idgs::actor::ActorMessageHandler>(&RddServiceActor::handleCreateRdd)},
      {CREATE_RDD_PARTITION, static_cast<idgs::actor::ActorMessageHandler>(&RddServiceActor::handleCreateRddPartition)},
      {RDD_ACTION_REQUEST, static_cast<idgs::actor::ActorMessageHandler>(&RddServiceActor::handleRddActionRequest)},
      {idgs::actor::OP_DESTROY, static_cast<idgs::actor::ActorMessageHandler>(&RddServiceActor::handleDestroyRddRequest)}
  };
  return handlerMap;
}

ActorDescriptorPtr RddServiceActor::generateActorDescriptor() {
  static std::shared_ptr<ActorDescriptorWrapper> descriptor;
  if (descriptor)
    return descriptor;
  descriptor.reset(new ::ActorDescriptorWrapper);

  descriptor->setName(RDD_SERVICE_ACTOR);
  descriptor->setDescription("RDD Service");
  descriptor->setType(AT_STATELESS);

  // in operation
  ActorOperationDescriporWrapper createDelegate;
  createDelegate.setName(CREATE_STORE_DELEGATE_RDD);
  createDelegate.setDescription("create store delegate rdd");
  createDelegate.setPayloadType("idgs.rdd.pb.CreateDelegateRddRequest");
  descriptor->setInOperation(createDelegate.getName(), createDelegate);

  ActorOperationDescriporWrapper createDelegatePartition;
  createDelegatePartition.setName(CREATE_DELEGATE_PARTITION);
  createDelegatePartition.setDescription("create rdd partition of store delegate");
  createDelegatePartition.setPayloadType("idgs.rdd.pb.CreateDelegatePartitionRequest");
  descriptor->setInOperation(createDelegatePartition.getName(), createDelegatePartition);

  ActorOperationDescriporWrapper filter;
  filter.setName(CREATE_RDD);
  filter.setDescription("create transform rdd");
  filter.setPayloadType("idgs.rdd.pb.CreateRddRequest");
  descriptor->setInOperation(filter.getName(), filter);

  ActorOperationDescriporWrapper rddPartition;
  rddPartition.setName(CREATE_RDD_PARTITION);
  rddPartition.setDescription("create rdd partition");
  rddPartition.setPayloadType("idgs.rdd.pb.CreateRddPartitionRequest");
  descriptor->setInOperation(rddPartition.getName(), rddPartition);

  ActorOperationDescriporWrapper destroyRdd;
  destroyRdd.setName(idgs::actor::OP_DESTROY);
  destroyRdd.setDescription("destroy named rdd");
  destroyRdd.setPayloadType("idgs.rdd.pb.DestroyRddRequest");
  descriptor->setInOperation(destroyRdd.getName(), destroyRdd);

  // out operation
  // out operation for CREATE_STORE_DELEGATE_RDD and CREATE_RDD
  ActorOperationDescriporWrapper createRddResponse;
  createRddResponse.setName(CREATE_RDD_RESPONSE);
  createRddResponse.setDescription("the response of create rdd.");
  createRddResponse.setPayloadType("idgs.rdd.pb.CreateDelegateRddResponse");
  descriptor->setOutOperation(createRddResponse.getName(), createRddResponse);

  // out operation for CREATE_DELEGATE_PARTITION and CREATE_RDD_PARTITION
  ActorOperationDescriporWrapper createPartitionResponse;
  createPartitionResponse.setName(CREATE_RDD_PARTITION_RESPONSE);
  createPartitionResponse.setDescription("response of create rdd partition");
  createPartitionResponse.setPayloadType("idgs.rdd.pb.CreateRddPartitionResponse");
  descriptor->setOutOperation(createPartitionResponse.getName(), createPartitionResponse);

  ActorOperationDescriporWrapper actionRequest;
  actionRequest.setName(RDD_ACTION_REQUEST);
  actionRequest.setDescription("handle action for rdd");
  actionRequest.setPayloadType("idgs.rdd.pb.ActionRequest");
  descriptor->setInOperation(actionRequest.getName(), actionRequest);

  return descriptor;
}

void RddServiceActor::handleCreateStoreDelegate(const idgs::actor::ActorMessagePtr& msg) {
  CreateDelegateRddRequest* request = dynamic_cast<CreateDelegateRddRequest*>(msg->getPayload().get());
  DVLOG(2) << "RDD Service begin to create store delegate, store name: " << request->store_name();
  DataStore& store = singleton<DataStore>::getInstance();
  shared_ptr<StoreConfigWrapper> config(new StoreConfigWrapper);
  auto code = store.loadStoreConfig(request->store_name(), config);
  shared_ptr<CreateDelegateRddResponse> response(new CreateDelegateRddResponse);
  if (code == RC_STORE_NOT_FOUND) {
    response->set_result_code(RRC_STORE_NOT_FOUND);
  } else {
    VLOG(1) << "RDD \"" << request->rdd_name() << "\" created, the state is INIT.";
    StoreDelegateRddActor* rdd = new StoreDelegateRddActor;
    singleton<RpcFramework>::getInstance().getActorFramework()->Register(rdd->getActorId(), rdd);

    uint32_t localMemberId = singleton<ClusterFramework>::getInstance().getMemberManager()->getLocalMemberId();
    response->mutable_rdd_id()->set_actor_id(rdd->getActorId());
    response->mutable_rdd_id()->set_member_id(localMemberId);
    response->set_result_code(RRC_SUCCESS);

    if (request->has_rdd_name()) {
      DVLOG(2)<<"save store delegate RDD with name " << request->rdd_name();
      rdd->setRddName(request->rdd_name());
      rdd->takeSnapShot()->save();
      //saveRddActor(rdd);
    } else {
      DVLOG(2)<<"save store delegate RDD with actor id " << rdd->getActorId();
      rdd->setRddName(rdd->getActorId());
      rdd->takeSnapShot()->save();
    }

    ActorMessagePtr routeMsg = const_cast<const ActorMessagePtr&>(msg);
    rdd->process(routeMsg);
  }

  auto respMsg = msg->createResponse();
  respMsg->setOperationName(CREATE_RDD_RESPONSE);
  respMsg->setPayload(response);
  idgs::actor::sendMessage(respMsg);
}

void RddServiceActor::handleCreateDelegatePartition(const idgs::actor::ActorMessagePtr& msg) {
  DVLOG(2) << "RddServiceActor : handle create store delegate RDD partition.";
  CreateDelegatePartitionRequest* request = dynamic_cast<CreateDelegatePartitionRequest*>(msg->getPayload().get());
  StoreDelegateRddPartition* partition = new StoreDelegateRddPartition(request->partition(), request->store_name());
  partition->setRddName(request->rdd_name());
  singleton<RpcFramework>::getInstance().getActorFramework()->Register(partition->getActorId(), partition);

  DataStore& store = singleton<DataStore>::getInstance();
  shared_ptr<StoreConfigWrapper> config(new StoreConfigWrapper);
  auto code = store.loadStoreConfig(request->store_name(), config);
  partition->setRddReplicated(config->getStoreConfig().partition_type() == idgs::store::pb::REPLICATED);

  msg->setOperationName(RDD_PARTITION_PROCESS);
  ActorMessagePtr routeMsg = const_cast<const ActorMessagePtr&>(msg);
  partition->process(routeMsg);

  auto localMemberId = singleton<ClusterFramework>::getInstance().getMemberManager()->getLocalMemberId();
  shared_ptr<CreateRddPartitionResponse> response(new CreateRddPartitionResponse);
  response->set_result_code(RRC_SUCCESS);
  response->mutable_rdd_partition()->set_partition(request->partition());
  response->mutable_rdd_partition()->mutable_actor_id()->set_actor_id(partition->getActorId());
  response->mutable_rdd_partition()->mutable_actor_id()->set_member_id(localMemberId);

  auto respMsg = msg->createResponse();
  respMsg->setOperationName(CREATE_RDD_PARTITION_RESPONSE);
  respMsg->setPayload(response);

  DVLOG(2) << "send response to store delegate.";
  idgs::actor::sendMessage(respMsg);
}

void RddServiceActor::handleCreateRdd(const idgs::actor::ActorMessagePtr& msg) {
  CreateRddRequest* request = dynamic_cast<CreateRddRequest*>(msg->getPayload().get());
  DVLOG(2) << "RDD Service begin to create RDD, RDD name: " << request->out_rdd().rdd_name();

  RddResultCode code = RRC_SUCCESS;
  RddActor* rdd = new RddActor;

  uint32_t localMemberId = singleton<ClusterFramework>::getInstance().getMemberManager()->getLocalMemberId();
  shared_ptr<CreateRddResponse> response(new CreateRddResponse);
  response->mutable_rdd_id()->set_actor_id(rdd->getActorId());
  response->mutable_rdd_id()->set_member_id(localMemberId);

  vector<ActorMessagePtr> reqs(request->in_rdd_size());

  for (int32_t i = 0; i < request->in_rdd_size(); ++i) {
    shared_ptr<ActorId> rddActor(new ActorId);
    rddActor->set_member_id(localMemberId);
    rddActor->set_actor_id(rdd->getActorId());

    ActorMessagePtr message = createActorMessage();

    VLOG(2) << "will load depending rdd for " << request->in_rdd(i).rdd_name();
    if (request->in_rdd(i).has_rdd_name()) {
      shared_ptr<RddSnapshot> snapshot = RddSnapshot::restore(request->in_rdd(i).rdd_name(), code);
      //code = loadRddActor(request->in_rdd(i).rdd_name(), actorid);
      if (code == RRC_SUCCESS) {
        const ActorId& actorid = snapshot->getSelfInfo().getActorId();
        rdd->addDependingRdd(actorid, request->in_rdd(i).rdd_name());

        message->setDestActorId(actorid.actor_id());
        message->setDestMemberId(actorid.member_id());
      } else {
        LOG(ERROR)<< "Error when loading depending RDD " << request->in_rdd(i).rdd_name() << ", caused by " << RddResultCode_Name(code);
        return;
      }
    } else {
      rdd->addDependingRdd(request->in_rdd(i).rdd_id(),"");

      message->setDestActorId(request->in_rdd(i).rdd_id().actor_id());
      message->setDestMemberId(request->in_rdd(i).rdd_id().member_id());
    }

    message->setOperationName(RECEIVE_DEPENDING_RDD);
    message->setPayload(rddActor);
    reqs.push_back(message);
    idgs::actor::sendMessage(message);
  }

  /// @fixme destroy previous RDD if exist.
  rdd->setRddName(request->out_rdd().rdd_name());
  code = rdd->takeSnapShot()->save();
  //code = saveRddActor(rdd);
  if (code != RRC_SUCCESS) {
    LOG(ERROR)<< "Error when saving RDD name, caused by " << RddResultCode_Name(code);
    return;
  }

/*  for (vector<ActorMessagePtr>::iterator itr = reqs.begin();itr < reqs.end();itr++) {
    idgs::actor::sendMessage(*itr);
  }*/

  VLOG(1) << "RDD \"" << request->out_rdd().rdd_name() << "\" created, the state is INIT.";
  singleton<RpcFramework>::getInstance().getActorFramework()->Register(rdd->getActorId(), rdd);

  ActorMessagePtr routeMsg = const_cast<const ActorMessagePtr&>(msg);
  rdd->process(routeMsg);

  response->set_result_code(code);
  auto respMsg = msg->createResponse();
  respMsg->setOperationName(CREATE_RDD_RESPONSE);
  respMsg->setPayload(response);
  idgs::actor::sendMessage(respMsg);
}

void RddServiceActor::handleCreateRddPartition(const idgs::actor::ActorMessagePtr& msg) {
  CreateRddPartitionRequest* request = dynamic_cast<CreateRddPartitionRequest*>(msg->getPayload().get());
  DVLOG(2) << "RDD Service begin to create RDD Partition " << request->partition() << ", RDD name: "
              << request->rdd_name();
  RddPartition* partition = new RddPartition(request->partition());
  partition->setRddName(request->rdd_name());
  partition->setRddId(request->rdd_id());
  singleton<RpcFramework>::getInstance().getActorFramework()->Register(partition->getActorId(), partition);

  auto localMemberId = singleton<ClusterFramework>::getInstance().getMemberManager()->getLocalMemberId();
  shared_ptr<CreateRddPartitionResponse> response(new CreateRddPartitionResponse);
  response->set_result_code(RRC_SUCCESS);
  response->mutable_rdd_partition()->mutable_actor_id()->set_actor_id(partition->getActorId());
  response->mutable_rdd_partition()->mutable_actor_id()->set_member_id(localMemberId);
  response->mutable_rdd_partition()->set_partition(request->partition());

  auto respMsg = msg->createResponse();
  respMsg->setOperationName(CREATE_RDD_PARTITION_RESPONSE);
  respMsg->setPayload(response);

  idgs::actor::sendMessage(respMsg);
}

void RddServiceActor::handleRddActionRequest(const idgs::actor::ActorMessagePtr& msg) {
  ActionRequest* request = dynamic_cast<ActionRequest*>(msg->getPayload().get());
  DVLOG(2) << "RDD Service begin to handle action, action id: " << request->action_id() << ", operation: "
              << request->action_op_name();
  RddResultCode code;
  shared_ptr<RddSnapshot> snapshot;
  if (!request->has_rdd_name()) {
    code = RRC_RDD_NOT_FOUND;
  } else {
    snapshot = RddSnapshot::restore(request->rdd_name(), code);
    //code = loadRddActor(request->rdd_name(), actor);
  }

  if (code == RRC_SUCCESS) {
    const ActorId& actor = snapshot->getSelfInfo().getActorId();
    auto respMsg = msg->createRouteMessage(actor.member_id(), actor.actor_id());

    DVLOG(3) << "sending action request to actor " << actor.actor_id() << " on member " << actor.member_id();
    idgs::actor::sendMessage(respMsg);
  } else {
    shared_ptr<ActionResponse> response(new ActionResponse);
    response->set_action_id(request->action_id());
    response->set_result_code(code);

    auto respMsg = msg->createResponse();
    respMsg->setOperationName(RDD_ACTION_RESPONSE);
    respMsg->setPayload(response);

    idgs::actor::sendMessage(respMsg);
  }
}

void RddServiceActor::handleDestroyRddRequest(const idgs::actor::ActorMessagePtr& msg) {
  DestroyRddRequest* request = dynamic_cast<DestroyRddRequest*>(msg->getPayload().get());
  VLOG(2) << "RDD Service begin to destroy named RDD: " << request->rdd_name();
  RddResultCode code;
  shared_ptr<RddSnapshot> snapshot = RddSnapshot::restore(request->rdd_name(), code);

  //code = loadRddActor(request->rdd_name(), actor);

  if (code == RRC_SUCCESS) {
    // send OP_DESTROY message to the RDD
    {
      const ActorId& actor = snapshot->getSelfInfo().getActorId();
      ActorMessagePtr msg = createActorMessage();
      msg->setDestMemberId(actor.member_id());
      msg->setDestActorId(actor.actor_id());
      msg->setOperationName(idgs::actor::OP_DESTROY);
      idgs::actor::sendMessage(msg);
    }

    // remove the rdd_name from the store
    {
      auto key = singleton<MessageHelper>::getInstance().createMessage(RDD_NAME_KEY);
      if (!key) {
        LOG(ERROR)<< "system store RddName key is not registered";
      }

      key->GetReflection()->SetString(key.get(), key->GetDescriptor()->FindFieldByName("rdd_name"),
          request->rdd_name());

      ActorMessagePtr msg = createActorMessage();
      msg->setOperationName(OP_DELETE);
      msg->setDestMemberId(idgs::pb::ANY_MEMBER);
      msg->setDestActorId(ACTORID_STORE_SERVCIE);

      shared_ptr<RemoveRequest> request(new RemoveRequest);
      request->set_store_name("RddName");
      msg->setPayload(request);

      msg->setAttachment(STORE_ATTACH_KEY, key);

      idgs::actor::sendMessage(msg);
    }
  }

  // send response
  shared_ptr<ActionResponse> response(new ActionResponse);
  response->set_result_code(code);

  auto respMsg = msg->createResponse();
  respMsg->setOperationName(RDD_ACTION_RESPONSE);
  respMsg->setPayload(response);

  idgs::actor::sendMessage(respMsg);
}


} // namespace rdd
} // namespace idgs 
