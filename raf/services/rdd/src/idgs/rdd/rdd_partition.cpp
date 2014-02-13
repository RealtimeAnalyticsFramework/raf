
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "rdd_partition.h"
#include "idgs/util/utillity.h"
#include "idgs/cluster/cluster_framework.h"
#include "idgs/rdd/pb/rdd_internal.pb.h"
#include "protobuf/message_helper.h"
#include "protobuf/type_composer.h"

#include "idgs/rdd/rdd_module.h"

using namespace std;
using namespace idgs::util;
using namespace idgs::actor;
using namespace idgs::cluster;
using namespace idgs::pb;
using namespace idgs::rdd::pb;
using namespace protobuf;

namespace idgs {
namespace rdd {

ActorDescriptorPtr RddPartition::descriptor;

tbb::spin_rw_mutex RddPartition::mutex;

RddPartition::RddPartition(const uint32_t& partitionId) :
    dataType(T_ORDERED), reUseKey(false) {
  partition = partitionId;
  ClusterFramework& cluster = singleton<ClusterFramework>::getInstance();
  auto partitionSize = cluster.getPartitionCount();
  partitionActors.resize(partitionSize);
}

RddPartition::~RddPartition() {
  dependingPartitionActors.clear();
  partitionActors.clear();
  orderedDataMap.clear();
  unorderedDataMap.clear();


  inRddInfo.clear();
}

void RddPartition::setRddId(const ActorId& actorId) {
  rddId.set_member_id(actorId.member_id());
  rddId.set_actor_id(actorId.actor_id());
}

RddResultCode RddPartition::addRddPartitionActor(const std::string& partitonActor) {
  auto actorFramework = ::idgs::util::singleton<RpcFramework>::getInstance().getActorFramework();
  Actor* actor = actorFramework->getActor(partitonActor);
  if (!actor) {
    LOG(ERROR)<< "RDD \"" << getRddName() << "\" partition[" << partition << "] actor with id " << partitonActor << " <<  is null";
    return RRC_PARTITION_ACTOR_NOT_FOUND;
  } else {
    BaseRddPartition* rddPartition = dynamic_cast<BaseRddPartition*>(actor);
    if (rddPartition == NULL) {
      LOG(ERROR) << "the actor " << actor->getActorName() << " is not a RDD partition.";
      return RRC_INVALID_PARTITION_ACTOR;
    }

    dependingPartitionActors.push_back(rddPartition);
  }

  return RRC_SUCCESS;
}

const ActorMessageHandlerMap& RddPartition::getMessageHandlerMap() const {
  static std::map<std::string, idgs::actor::ActorMessageHandler> handlerMap = {
      {PARTITION_PREPARED_REQUEST, static_cast<idgs::actor::ActorMessageHandler>(&RddPartition::handlePartitionPreparedRequest)},
      {RDD_PARTITION_PROCESS, static_cast<idgs::actor::ActorMessageHandler>(&RddPartition::handleProcess)},
      {RDD_ACTION_REQUEST, static_cast<idgs::actor::ActorMessageHandler>(&RddPartition::handleAction)},
      {RE_PARTITION_MIGRATE, static_cast<idgs::actor::ActorMessageHandler>(&RddPartition::handleRePartitionMigrate)},
      {SEND_RDD_INFO, static_cast<idgs::actor::ActorMessageHandler>(&RddPartition::handleSendRddInfo)},
      {CHECK_PARTITION_READY, static_cast<idgs::actor::ActorMessageHandler>(&RddPartition::handleCheckPartitionReady)},
      {RDD_PARTITION_DESTROY, static_cast<idgs::actor::ActorMessageHandler>(&RddPartition::handleDestory)}
  };
  return handlerMap;
}

ActorDescriptorPtr RddPartition::generateActorDescriptor() {
  static std::shared_ptr<ActorDescriptorWrapper> descriptor;
  if (descriptor)
    return descriptor;
  descriptor.reset(new ::ActorDescriptorWrapper);

  descriptor->setName(RDD_PARTITION);
  descriptor->setDescription("Partition of RDD");
  descriptor->setType(AT_STATEFUL);

  // in operation
  ActorOperationDescriporWrapper inCreateRddPartition;
  inCreateRddPartition.setName(CREATE_RDD_PARTITION);
  inCreateRddPartition.setDescription("Process RDD partition initialization.");
  inCreateRddPartition.setPayloadType("idgs.rdd.pb.CreateRddPartitionRequest");
  descriptor->setInOperation(inCreateRddPartition.getName(), inCreateRddPartition);

  ActorOperationDescriporWrapper inDependingPartitionRequest;
  inDependingPartitionRequest.setName(PARTITION_PREPARED_REQUEST);
  inDependingPartitionRequest.setDescription("Receive depending RDD actor.");
  inDependingPartitionRequest.setPayloadType("idgs.rdd.pb.DependingRddPartition");
  descriptor->setInOperation(inDependingPartitionRequest.getName(), inDependingPartitionRequest);

  ActorOperationDescriporWrapper inRddPartitionProcess;
  inRddPartitionProcess.setName(RDD_PARTITION_PROCESS);
  inRddPartitionProcess.setDescription("RDD partition process");
  inRddPartitionProcess.setPayloadType("idgs.rdd.pb.CreateRddRequest");
  descriptor->setInOperation(inRddPartitionProcess.getName(), inRddPartitionProcess);

  ActorOperationDescriporWrapper inRddActionRequest;
  inRddActionRequest.setName(RDD_ACTION_REQUEST);
  inRddActionRequest.setDescription("The action of count data size.");
  inRddActionRequest.setPayloadType("idgs.rdd.pb.ActionRequest");
  descriptor->setInOperation(inRddActionRequest.getName(), inRddActionRequest);

  ActorOperationDescriporWrapper inRePartitionMigrate;
  inRePartitionMigrate.setName(RE_PARTITION_MIGRATE);
  inRePartitionMigrate.setDescription("Migrate data from another partition.");
  inRePartitionMigrate.setPayloadType("idgs.rdd.pb.RddRequest");
  descriptor->setInOperation(inRePartitionMigrate.getName(), inRePartitionMigrate);

  ActorOperationDescriporWrapper inSendRddInfo;
  inSendRddInfo.setName(SEND_RDD_INFO);
  inSendRddInfo.setDescription("Receive information of rdd and all it's partition.");
  inSendRddInfo.setPayloadType("idgs.rdd.pb.RddActorInfo");
  descriptor->setInOperation(inSendRddInfo.getName(), inSendRddInfo);

  ActorOperationDescriporWrapper inCheckPartitionReady;
  inCheckPartitionReady.setName(CHECK_PARTITION_READY);
  inCheckPartitionReady.setDescription("To check whether partition is process finished.");
  inCheckPartitionReady.setPayloadType("idgs.rdd.pb.RddRequest");
  descriptor->setInOperation(inCheckPartitionReady.getName(), inCheckPartitionReady);

  // out operation
  // no out operation for CREATE_RDD_PARTITION

  ActorOperationDescriporWrapper outDependingPartitionResponse;
  outDependingPartitionResponse.setName(PARTITION_PREPARED_RESPONSE);
  outDependingPartitionResponse.setDescription("The response of depending RDD actor.");
  outDependingPartitionResponse.setPayloadType("idgs.rdd.pb.RddRequest");
  descriptor->setOutOperation(outDependingPartitionResponse.getName(), outDependingPartitionResponse);

  // out operation for RDD_PARTITION_PROCESS
  ActorOperationDescriporWrapper outPartitionTransformComplete;
  outPartitionTransformComplete.setName(PARTITION_TRANSFORM_COMPLETE);
  outPartitionTransformComplete.setDescription("The response of action.");
  outPartitionTransformComplete.setPayloadType("idgs.rdd.pb.PartitionProcessResult");
  descriptor->setOutOperation(outPartitionTransformComplete.getName(), outPartitionTransformComplete);

  // out operation for RDD_ACTION_REQUEST
  ActorOperationDescriporWrapper outRddActionResponse;
  outRddActionResponse.setName(RDD_ACTION_RESPONSE);
  outRddActionResponse.setDescription("Out for partition process, send result.");
  outRddActionResponse.setPayloadType("idgs.rdd.pb.ActionResponse");
  descriptor->setOutOperation(outRddActionResponse.getName(), outRddActionResponse);

  //no out operation for RE_PARTITION_MIGRATE

  ActorOperationDescriporWrapper outSendRddInfoResponse;
  outSendRddInfoResponse.setName(SEND_RDD_INFO_RESPONSE);
  outSendRddInfoResponse.setDescription("The response of receive information of rdd and all it's partition.");
  outSendRddInfoResponse.setPayloadType("idgs.rdd.pb.RddRequest");
  descriptor->setOutOperation(outSendRddInfoResponse.getName(), outSendRddInfoResponse);

  ActorOperationDescriporWrapper outRddPartitionReady;
  outRddPartitionReady.setName(RDD_PARTITION_READY);
  outRddPartitionReady.setDescription("To send ready to RDD when this partition is really ready.");
  outRddPartitionReady.setPayloadType("idgs.rdd.pb.PartitionProcessResult");
  descriptor->setOutOperation(outRddPartitionReady.getName(), outRddPartitionReady);

  RddPartition::descriptor = descriptor;
  return descriptor;
}

const ActorDescriptorPtr& RddPartition::getDescriptor() const {
  return RddPartition::descriptor;
}

void RddPartition::handlePartitionPreparedRequest(const idgs::actor::ActorMessagePtr& msg) {
  DependingRddPartition* dependingPartition = dynamic_cast<DependingRddPartition*>(msg->getPayload().get());

  CreateRddRequest request;
  if (!msg->parseAttachment(TRANSFORM_REQUEST, &request)) {
    LOG(ERROR)<< "no create param found";
    return;
  }

  dataType = (DataType) ((int32_t) request.out_rdd().data_type());

  MessageHelper& helper = singleton<MessageHelper>::getInstance();
  auto& outputRddInfo = request.out_rdd();

  createDynamicMessage(partition, request);

  auto key = helper.createMessage(outputRddInfo.key_type_name());
  auto value = helper.createMessage(outputRddInfo.value_type_name());
  setRddPartitionContext(key, value);

  inRddInfo.resize(request.in_rdd_size());
  RddResultCode code = RRC_SUCCESS;
  bool isReplicated = true;
  for (int32_t i = 0; i < dependingPartition->depending_partition_actor_id_size(); ++i) {
    if (code != RRC_SUCCESS) {
      break;
    }

    code = addRddPartitionActor(dependingPartition->depending_partition_actor_id(i));

    if (code == RRC_SUCCESS) {
      auto depending = dependingPartitionActors[i];
      isReplicated = isReplicated & depending->isReplicatedRdd();
      auto& inputRddInfo = request.in_rdd(i);

      if(!inRddInfo[i].parse(inputRddInfo, outputRddInfo, depending, this)){
        code = RRC_UNKOWN_ERROR;
        break;
      }

    } else {
      break;
    }
  }

  if (inRddInfo.size() == 1) {
    reUseKey = inRddInfo[0].outMsg.key.getReuse() == idgs::rdd::op::REUSE_KEY;
  } else {
    reUseKey = false;
  }

  setRddReplicated(isReplicated);
  done: shared_ptr<RddResponse> response(new RddResponse);
  response->set_result_code(code);
  response->set_partition(partition);

  auto respMsg = msg->createResponse();
  respMsg->setOperationName(PARTITION_PREPARED_RESPONSE);
  respMsg->setPayload(response);

  idgs::actor::sendMessage(respMsg);

  DVLOG(2) << "RDD \"" << getRddName() << " partition[" << partition << "] prepared.";
}

void RddPartition::setRddPartitionContext(const PbMessagePtr& key, const PbMessagePtr& value) {
  keyTemplate = key;
  valueTemplate = value;
}

void RddPartition::handleRePartitionMigrate(const idgs::actor::ActorMessagePtr& msg) {
  std::vector<PbMessagePtr> values;
  auto attachments = msg->getAttachments();
  auto range = attachments.equal_range(RE_PARTITION_VALUE_DATA);
  for (auto it = range.first; it != range.second; ++it) {
    values.push_back(it->second);
  }

  putLocal(msg->getAttachement(RE_PARTITION_KEY_DATA), values);
}

void RddPartition::handleProcess(const ActorMessagePtr& msg) {
  DVLOG(3) << "RDD \"" << getRddName() << "\" partition " << partition << " handle partition process.";
  CreateRddRequest* request = dynamic_cast<CreateRddRequest*>(msg->getPayload().get());
  LOG_FIRST_N(INFO, 1) << "Transform " << request->transformer_op_name() << " " << getRddName() << "(" << getPartition() << ")";
  auto actorFramework = ::idgs::util::singleton<RpcFramework>::getInstance().getActorFramework();
  shared_ptr<PartitionProcessResult> response(new PartitionProcessResult);
  response->set_partition(partition);

  TransformerMgr& transformerMgr = idgs_rdd_module()->getTransformManager();
  const TransformerPtr& transformer = transformerMgr.get(request->transformer_op_name());

  if (transformer.get()) {
    VLOG(2) << "Transform " << request->transformer_op_name() << " " << getRddName() << "(" << getPartition() << ")";
    auto startTime = sys::getCurrentTime();

    DVLOG(3) << "run transform with " << request->transformer_op_name();
    RddResultCode code = transformer->transform(msg, dependingPartitionActors, this);
    response->set_result_code(code);

    VLOG(2) << "Transform " << request->transformer_op_name() << " " << getRddName() << "(" << getPartition()
               << ") spent time: " << sys::formatTime((sys::getCurrentTime() - startTime));

  } else {
    LOG(ERROR)<< "transformer " << request->transformer_op_name() << " is not registered.";
    response->set_result_code(RRC_TRANSFORMER_NOT_FOUND);
  }

  auto respMsg = msg->createResponse();
  respMsg->setOperationName(PARTITION_TRANSFORM_COMPLETE);
  respMsg->setPayload(response);

  idgs::actor::sendMessage(respMsg);
  //singleton<ScheduledMessageService>::getInstance().schedule(respMsg, 30);
}

void RddPartition::handleAction(const ActorMessagePtr& msg) {
  DVLOG(3) << "partition " << partition << " handle action" << msg->toString();
  ActionRequest* request = dynamic_cast<ActionRequest*>(msg->getPayload().get());
  LOG_FIRST_N(INFO, 1) << "Action " << request->action_op_name() << " " << getRddName() << "(" << getPartition() << ")";
  shared_ptr<ActionResponse> response(new ActionResponse);
  response->set_action_id(request->action_id());
  response->set_partition(partition);

  ActionMgr& actionMgr = idgs_rdd_module()->getActionManager();
  const ActionPtr& action = actionMgr.get(request->action_op_name());

  if (action.get()) {
    VLOG(2) << "Action " << request->action_op_name() << " " << getRddName() << "(" << getPartition() << ")";
    auto startTime = sys::getCurrentTime();

    vector<protobuf::PbVariant> result;
    if (isReplicatedRdd() && partition > 0) {
      response->set_result_code(RRC_SUCCESS);
    } else {
      RddResultCode code = action->action(msg, this, result);
      response->set_result_code(code);

      if (code == RRC_SUCCESS) {
        for (size_t i = 0; i < result.size(); ++i) {
          response->add_action_value(result[i].toString());
        }
      }
    }

    VLOG(2) << "Action " << request->action_op_name() << " " << getRddName() << "(" << getPartition() << ") spent time: " << sys::formatTime((sys::getCurrentTime() - startTime));
  } else {
    LOG(ERROR)<< "action " << request->action_op_name() << " is not registered.";
    response->set_result_code(RRC_ACTION_NOT_FOUND);
  }
  auto respMsg = msg->createResponse();
  respMsg->setOperationName(RDD_ACTION_RESPONSE);
  respMsg->setPayload(response);
  idgs::actor::sendMessage(respMsg);
}

void RddPartition::handleSendRddInfo(const idgs::actor::ActorMessagePtr& msg) {
  RddActorInfo* rddActorInfo = dynamic_cast<RddActorInfo*>(msg->getPayload().get());

  auto it = rddActorInfo->rdd_partition().begin();
  for (; it != rddActorInfo->rdd_partition().end(); ++it) {
    partitionActors[it->partition()] = it->actor_id();
  }

  auto respMsg = msg->createResponse();
  respMsg->setOperationName(SEND_RDD_INFO_RESPONSE);
  respMsg->setPayload(shared_ptr<RddRequest>(new RddRequest));

  idgs::actor::sendMessage(respMsg);
}

void RddPartition::handleCheckPartitionReady(const idgs::actor::ActorMessagePtr& msg) {
  DVLOG(3) << "RDD \"" << getRddName() << "\" check partition " << partition << " is ready.";
//      if (queue.empty()) {
  shared_ptr<PartitionProcessResult> response(new PartitionProcessResult);
  response->set_partition(partition);
  response->set_result_code(RRC_SUCCESS);

  auto respMsg = msg->createResponse();
  respMsg->setOperationName(RDD_PARTITION_READY);
  respMsg->setPayload(response);

  idgs::actor::sendMessage(respMsg);
//      } else {
//        DVLOG(3) << "RDD \"" << getRddName() << "\" have " << queue.unsafe_size() << " message left.";
//        idgs::actor::sendMessage(respMsg);
//      }
}

void RddPartition::handleDestory(const idgs::actor::ActorMessagePtr& msg) {
  delete this;
}

PbMessagePtr RddPartition::get(const PbMessagePtr& key) const {
  PbMessagePtr value;
  if (dataType == T_ORDERED) {
    auto it = orderedDataMap.find(key);
    if (it != orderedDataMap.end()) {
      if (!it->second.empty()) {
        value = it->second[0];
      }
    }
  } else {
    auto it = unorderedDataMap.find(key);
    if (it != unorderedDataMap.end()) {
      if (!it->second.empty()) {
        value = it->second[0];
      }
    }
  }

  return value;
}

vector<PbMessagePtr> RddPartition::getValue(const PbMessagePtr& key) const {
  vector<PbMessagePtr> value;
  if (dataType == T_ORDERED) {
    auto it = orderedDataMap.find(key);
    if (it != orderedDataMap.end()) {
      if (!it->second.empty()) {
        const vector<PbMessagePtr>& v = it->second;
        return v;
      }
    }
  } else {
    auto it = unorderedDataMap.find(key);
    if (it != unorderedDataMap.end()) {
      if (!it->second.empty()) {
        const vector<PbMessagePtr>& v = it->second;
        return v;
      }
    }
  }

  return value;
}

bool RddPartition::containKey(const idgs::actor::PbMessagePtr& key) const {
  if (dataType == T_ORDERED) {
    auto it = orderedDataMap.find(key);
    return (it != orderedDataMap.end());
  } else {
    auto it = unorderedDataMap.find(key);
    return (it != unorderedDataMap.end());
  }
}

void RddPartition::put(const PbMessagePtr& key, std::vector<idgs::actor::PbMessagePtr>& value) {
  if (!key.get()) {
    LOG(ERROR)<< "put data into RDD in partition " << partition << ", key is empty";
    return;
  }

  if(value.empty()) {
    LOG(ERROR) << "put data into RDD in partition " << partition << ", value is empty";
    return;
  }

  if (isReplicatedRdd()) {
    putLocal(key, value);
    return;
  }

  hashcode_t hash_code = ::idgs::util::singleton<HashCode>::getInstance().hashcode(key.get());
  ClusterFramework& cluster = ::idgs::util::singleton<ClusterFramework>::getInstance();
  size_t p = hash_code % cluster.getPartitionCount();
  if (p == partition) {
    putLocal(key, value);
  } else {
    int32_t localMemberId = cluster.getMemberManager()->getLocalMemberId();

    shared_ptr<RddRequest> request(new RddRequest);
    ActorMessagePtr message = createActorMessage();
    message->setSerdesType(idgs::pb::PB_BINARY);
    message->setOperationName(RE_PARTITION_MIGRATE);
    message->setDestActorId(partitionActors[p].actor_id());
    message->setDestMemberId(partitionActors[p].member_id());
    message->setPayload(request);

    message->setAttachment(RE_PARTITION_KEY_DATA, key);

    for (PbMessagePtr& v : value) {
      message->setAttachment(RE_PARTITION_VALUE_DATA, v);
    }

    idgs::actor::sendMessage(message);
  }
}

void RddPartition::put(const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
  if (!key.get()) {
      LOG(ERROR)<< "put data into RDD in partition " << partition << ", key is empty";
      return;
  }

  if(!value.get()) {
    LOG(ERROR) << "put data into RDD in partition " << partition << ", value is empty";
    return;
  }

  if (isReplicatedRdd()) {
    putLocal(key, value);
    return;
  }

  hashcode_t hash_code = ::idgs::util::singleton<HashCode>::getInstance().hashcode(key.get());
  ClusterFramework& cluster = ::idgs::util::singleton<ClusterFramework>::getInstance();
  size_t p = hash_code % cluster.getPartitionCount();
  if (p == partition) {
    putLocal(key, value);
  } else {
    int32_t localMemberId = cluster.getMemberManager()->getLocalMemberId();

    shared_ptr<RddRequest> request(new RddRequest);
    ActorMessagePtr message = createActorMessage();
    message->setSerdesType(idgs::pb::PB_BINARY);
    message->setOperationName(RE_PARTITION_MIGRATE);
    message->setDestActorId(partitionActors[p].actor_id());
    message->setDestMemberId(partitionActors[p].member_id());
    message->setPayload(request);

    message->setAttachment(RE_PARTITION_KEY_DATA, key);
    message->setAttachment(RE_PARTITION_VALUE_DATA, value);

    idgs::actor::sendMessage(message);
  }
}

void RddPartition::putLocal(const PbMessagePtr& key, std::vector<idgs::actor::PbMessagePtr>& value) {
  if (!key.get()) {
    LOG(ERROR)<< "put data into local RDD in partition " << partition << ", key is empty";
    return;
  }

  if(value.empty()) {
    LOG(ERROR) << "put data into local RDD in partition " << partition << ", value is empty";
    return;
  }

  if (dataType == T_ORDERED) {
    std::vector<idgs::actor::PbMessagePtr>& v = orderedDataMap[key];
    v.insert(v.end(), value.begin(), value.end());
  } else {
    std::vector<idgs::actor::PbMessagePtr>& v = unorderedDataMap[key];
    v.insert(v.end(), value.begin(), value.end());
  }
}

void RddPartition::putLocal(const PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
  if (!key.get()) {
    LOG(ERROR)<< "put data into local RDD in partition " << partition << ", key is empty";
    return;
  }

  if(!value.get()) {
    LOG(ERROR) << "put data into local RDD in partition " << partition << ", value is empty";
    return;
  }

  if (dataType == T_ORDERED) {
    orderedDataMap[key].push_back(value);
  } else {
    unorderedDataMap[key].push_back(value);
  }
}

bool RddPartition::empty() const {
  if (dataType == T_ORDERED) {
    return orderedDataMap.empty();
  } else {
    return unorderedDataMap.empty();
  }
}

/// @todo maybe two method size should be provided.
size_t RddPartition::size() const {
  size_t sum = 0;
  if (dataType == T_ORDERED) {
    auto it = orderedDataMap.begin();
    for (; it != orderedDataMap.end(); ++it) {
      sum += it->second.size();
    }
  } else {
    auto it = unorderedDataMap.begin();
    for (; it != unorderedDataMap.end(); ++it) {
      sum += it->second.size();
    }
  }

  return sum;
}

void RddPartition::foreach(RddEntryFunc fn) const {
  if (dataType == T_ORDERED) {
    auto it = orderedDataMap.begin();
    for (; it != orderedDataMap.end(); ++it) {
      for (size_t i = 0; i < it->second.size(); ++i) {
        fn(it->first, it->second[i]);
      }
    }
  } else {
    auto it = unorderedDataMap.begin();
    for (; it != unorderedDataMap.end(); ++it) {
      for (size_t i = 0; i < it->second.size(); ++i) {
        fn(it->first, it->second[i]);
      }
    }
  }
}

void RddPartition::foreachGroup(RddGroupEntryFunc fn) const {
  if (dataType == T_ORDERED) {
    auto it = orderedDataMap.begin();
    for (; it != orderedDataMap.end(); ++it) {
      fn(it->first, it->second);
    }
  } else {
    auto it = unorderedDataMap.begin();
    for (; it != unorderedDataMap.end(); ++it) {
      fn(it->first, it->second);
    }
  }
}

void RddPartition::createDynamicMessage(const uint32_t partition, const pb::CreateRddRequest& request) {
  MessageHelper& helper = singleton<MessageHelper>::getInstance();
  auto out = request.out_rdd();
  string keyType = out.key_type_name();
  string valueType = out.value_type_name();

  if (helper.isMessageRegistered(keyType) && helper.isMessageRegistered(valueType)) {
    return;
  }

  {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
    bool isKeyReg = helper.isMessageRegistered(keyType);
    bool isValueReg = helper.isMessageRegistered(valueType);
    if (isKeyReg && isValueReg) {
      return;
    }

    const string& rddName = request.out_rdd().rdd_name();
    string fileName = RDD_DYNAMIC_PROTO_PATH + "DM_" + rddName + "_" + to_string(partition) + ".proto";
    if (out.has_proto_message()) {
      sys::saveFile(fileName, out.proto_message());
    } else {
      auto pos = valueType.find_last_of(".");
      string package = valueType.substr(0, pos);
      string keyName = keyType.substr(keyType.find_last_of(".") + 1, keyType.length());
      string valueName = valueType.substr(pos + 1, valueType.length());

      DynamicTypeComposer composer;
      composer.setPackage(package);
      composer.setName(valueName);

      if (!isKeyReg) {
        DynamicMessage dmKey;
        dmKey.name = keyName;
        for (int32_t i = 0; i < out.key_fields_size(); ++i) {
          string name = out.key_fields(i).field_name();
          string type = idgs::str::toLower(DataType_Name(out.key_fields(i).field_type()));

          DynamicField field = DynamicField("required", type, name);
          dmKey.addField(field);
        }

        composer.addMessage(dmKey);
      }

      if (!isValueReg) {
        DynamicMessage dmValue;
        dmValue.name = valueName;
        for (int32_t i = 0; i < out.value_fields_size(); ++i) {
          string name = out.value_fields(i).field_name();
          string type = idgs::str::toLower(DataType_Name(out.value_fields(i).field_type()));

          DynamicField field = DynamicField("optional", type, name);
          dmValue.addField(field);
        }

        composer.addMessage(dmValue);
      }

      composer.saveFile(fileName);
    }

    auto code = helper.registerDynamicMessage(fileName);
    if (code != RC_SUCCESS) {
      LOG(ERROR)<< "Invalid message type, caused by " << getErrorDescription(code);
    }

    remove(fileName.c_str());
  }
}

const idgs::expr::Expression* RddPartition::getFilterExpression(const int32_t inRddIndex) {
  return inRddInfo[inRddIndex].filterExpr;
}

bool RddPartition::parse(idgs::actor::ActorMessagePtr& msg) {
  idgs::actor::Actor::parse(msg);

  if (msg->getOperationName() == RE_PARTITION_MIGRATE) {
    PbMessagePtr key(getKeyTemplate()->New());
    if (!msg->parseAttachment1(RE_PARTITION_KEY_DATA, key)) {
      LOG(ERROR)<< "repartition key data is invalid";
      return false;
    }

    PbMessagePtr value(getValueTemplate()->New());
    if (!msg->parseAttachment1(RE_PARTITION_VALUE_DATA, value)) {
      LOG(ERROR)<< "repartition value data is invalid";
      return false;
    }
    msg->getRawAttachments().clear();
  }
  return true;
}

const idgs::rdd::op::OutMessagePair& RddPartition::getOutMessage(const int32_t inRddIndex) const {
  return inRddInfo[inRddIndex].outMsg;
}


} // namespace rdd
} // namespace idgs 
