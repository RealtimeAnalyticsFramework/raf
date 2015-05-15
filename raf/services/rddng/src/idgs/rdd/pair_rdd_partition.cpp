
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "pair_rdd_partition.h"

#include "idgs/application.h"

#include "idgs/rdd/rdd_local.h"
#include "idgs/rdd/rdd_module.h"
#include "idgs/rdd/pb/rdd_internal.pb.h"

using namespace std;
using namespace idgs::actor;
using namespace idgs::pb;
using namespace idgs::rdd::pb;
using namespace protobuf;

namespace idgs {
namespace rdd {

ActorDescriptorPtr PairRddPartition::descriptor;

PairRddPartition::PairRddPartition(const std::string& rddname, const uint32_t& partitionId) : useRepartitionLocalCache(false) {
  rddName = rddname;
  partition = partitionId;
  partitionName = "RDD " + rddName + "[" + std::to_string(partition) + "]";
}

PairRddPartition::~PairRddPartition() {
  orderedDataMap.clear();
  unorderedDataMap.clear();
  localCache.clear();
}

const ActorMessageHandlerMap& PairRddPartition::getMessageHandlerMap() const {
  static idgs::actor::ActorMessageHandlerMap handlerMap = {
      {RDD_TRANSFORM,   {
          static_cast<ActorMessageHandler>(&PairRddPartition::handleRddTransform),
          &idgs::rdd::pb::RddRequest::default_instance()
      }},
      {PARTITION_STORE, {
          static_cast<ActorMessageHandler>(&PairRddPartition::handlePartitionStore),
          &idgs::rdd::pb::PartitionStoreOption::default_instance()
      }},
      {RE_PARTITION,   {
          static_cast<ActorMessageHandler>(&PairRddPartition::handleRePartition),
          &idgs::rdd::pb::RddRequest::default_instance()
      }},
      {CHECK_PARTITION_READY, {
          static_cast<ActorMessageHandler>(&PairRddPartition::handleCheckPartitionReady),
          &idgs::rdd::pb::RddRequest::default_instance()
      }},
      {RDD_ACTION_REQUEST,  {
          static_cast<ActorMessageHandler>(&PairRddPartition::handleActionRequest),
          &idgs::rdd::pb::ActionRequest::default_instance()
      }}
  };
  return handlerMap;
}

ActorDescriptorPtr PairRddPartition::generateActorDescriptor() {
  static std::shared_ptr<ActorDescriptorWrapper> descriptor;
  if (descriptor) {
    return descriptor;
  }

  descriptor = make_shared<ActorDescriptorWrapper>();

  descriptor->setName(PAIR_RDD_PARTITION);
  descriptor->setDescription("Partition of RDD");
  descriptor->setType(AT_STATEFUL);

  // in operation
  ActorOperationDescriporWrapper rddTransform;
  rddTransform.setName(RDD_TRANSFORM);
  rddTransform.setDescription("Handle transform for current RDD partition.");
  rddTransform.setPayloadType("idgs.rdd.pb.RddRequest");
  descriptor->setInOperation(rddTransform.getName(), rddTransform);

  ActorOperationDescriporWrapper putLocal;
  putLocal.setName(PARTITION_STORE);
  putLocal.setDescription("put data to partition");
  putLocal.setPayloadType("idgs.rdd.pb.PartitionStoreOption");
  descriptor->setInOperation(putLocal.getName(), putLocal);

  ActorOperationDescriporWrapper rePartition;
  rePartition.setName(RE_PARTITION);
  rePartition.setDescription("Handle repartitioned data.");
  rePartition.setPayloadType("idgs.rdd.pb.RddRequest");
  descriptor->setInOperation(rePartition.getName(), rePartition);

  ActorOperationDescriporWrapper checkPartitionReady;
  checkPartitionReady.setName(CHECK_PARTITION_READY);
  checkPartitionReady.setDescription("check whether transform is complete.");
  checkPartitionReady.setPayloadType("idgs.rdd.pb.RddRequest");
  descriptor->setInOperation(checkPartitionReady.getName(), checkPartitionReady);

  ActorOperationDescriporWrapper actionRequest;
  actionRequest.setName(RDD_ACTION_REQUEST);
  actionRequest.setDescription("Handle action.");
  actionRequest.setPayloadType("idgs.rdd.pb.ActionRequest");
  descriptor->setInOperation(actionRequest.getName(), actionRequest);

  // out operation

  // out descriptor for RDD_TRANSFORM
  ActorOperationDescriporWrapper transComplete;
  transComplete.setName(PARTITION_TRANSFORM_COMPLETE);
  transComplete.setDescription("Send transform complete to RDD");
  transComplete.setPayloadType("idgs.rdd.pb.RddResponse");
  descriptor->setOutOperation(transComplete.getName(), transComplete);

  // out descriptor for CHECK_idgs::pb::PS_READY
  descriptor->setOutOperation(checkPartitionReady.getName(), checkPartitionReady);
  ActorOperationDescriporWrapper partitionReady;
  partitionReady.setName(PARTITION_READY);
  partitionReady.setDescription("call RDD current partiton is ready.");
  partitionReady.setPayloadType("idgs.rdd.pb.RddResponse");
  descriptor->setOutOperation(partitionReady.getName(), partitionReady);

  // no out descriptor for RE_PARTITION_MIGRATE_RESPONSE

  // out descriptor for RDD_ACTION_REQUEST
  ActorOperationDescriporWrapper actionResponse;
  actionResponse.setName(RDD_ACTION_RESPONSE);
  actionResponse.setDescription("Response when action done.");
  actionResponse.setPayloadType("idgs.rdd.pb.RddResponse");
  descriptor->setOutOperation(actionResponse.getName(), actionResponse);

  PairRddPartition::descriptor = descriptor;
  return descriptor;
}

const ActorDescriptorPtr& PairRddPartition::getDescriptor() const {
  return PairRddPartition::descriptor;
}

void PairRddPartition::handleRddTransform(const ActorMessagePtr& msg) {
  transform();

  shared_ptr<RddResponse> response = make_shared<RddResponse>();
  response->set_partition(partition);
  response->set_result_code(RRC_SUCCESS);

  auto respMsg = msg->createResponse();
  respMsg->setOperationName(PARTITION_TRANSFORM_COMPLETE);
  respMsg->setPayload(response);
  sendMessage(respMsg);
}

void PairRddPartition::handlePartitionStore(const idgs::actor::ActorMessagePtr& msg) {
  PartitionStoreOption* option = dynamic_cast<PartitionStoreOption*>(msg->getPayload().get());
  bool unique = option->unique();

  auto attachments = msg->getAttachments();
  auto key = msg->getAttachement(RE_PARTITION_KEY);
  auto range = attachments.equal_range(RE_PARTITION_VALUE);
  for (auto it = range.first; it != range.second; ++ it) {
    persist(key, it->second, unique);
  }
}

void PairRddPartition::handleRePartition(const ActorMessagePtr& msg) {
  auto attachments = msg->getAttachments();
  auto key = msg->getAttachement(RE_PARTITION_KEY);
  auto range = attachments.equal_range(RE_PARTITION_VALUE);
  for (auto it = range.first; it != range.second; ++ it) {
    if (rddLocal->isAggregate()) {
      aggrCtx.getExpressionContext()->setKeyValue(&key, &it->second);
      aggrCtx.getExpressionContext()->setOutputKeyValue(&key, &it->second);
      RddResultCode code = rddLocal->getTransformer()->aggregate(&aggrCtx, this);
      if (code != RRC_SUCCESS) {
        LOG(ERROR) << getPartitionName() << " transform error, caused by " << RddResultCode_Name(code);
      }
    } else {
      if (rddLocal->getPersistType() == NONE) {
        transform(key, it->second);
      } else {
        persist(key, it->second);
      }
    }
  }
}

void PairRddPartition::handleCheckPartitionReady(const ActorMessagePtr& msg) {
  if (!msg_queue.empty()) {
    postMessage(const_cast<ActorMessagePtr&>(msg));
    return;
  }

  DVLOG(2) << getPartitionName() << " check done, transform complete. " << " total data size  "
      << ((rddLocal->getPersistType() == PersistType::ORDERED) ? orderedDataMap.size() : unorderedDataMap.size());
  shared_ptr<RddResponse> payload = make_shared<RddResponse>();
  payload->set_partition(partition);
  payload->set_result_code(RRC_SUCCESS);

  auto respMsg = createActorMessage();
  respMsg->setOperationName(PARTITION_READY);
  respMsg->setDestActorId(rddLocal->getRddId().actor_id());
  respMsg->setDestMemberId(rddLocal->getRddId().member_id());
  respMsg->setPayload(payload);

  sendMessage(respMsg);
}

void PairRddPartition::handleActionRequest(const ActorMessagePtr& msg) {
  ActionRequest* request = dynamic_cast<ActionRequest*>(msg->getPayload().get());
  DVLOG(3) << getPartitionName() << " process " << request->action_op_name() << " action";
  shared_ptr<ActionResponse> response = make_shared<ActionResponse>();
  response->set_action_id(request->action_id());
  response->set_partition(partition);

  ActionMgr& actionMgr = *idgs_rdd_module()->getActionManager();
  const ActionPtr& action = actionMgr.get(request->action_op_name());

  RddResultCode code = RRC_SUCCESS;
  if (action.get()) {
    if (rddLocal->isReplicatedRdd() && partition > 0) {
      code = RRC_SUCCESS;
    } else {
      action::ActionContext ctx;
      if (ctx.initContext(&msg, &rddLocal->getKeyTemplate(), &rddLocal->getValueTemplate())) {
        code = action->action(&ctx, this);
        if (code == RRC_SUCCESS) {
          auto& result = ctx.getPartitionResult();
          for (size_t i = 0; i < result.size(); ++ i) {
            response->add_action_value(result[i].toString());
          }
        }
      } else {
        code = RRC_INVALID_ACTION_PARAM;
      }
    }
  } else {
    LOG(ERROR)<< "action " << request->action_op_name() << " is not registered.";
    code = RRC_ACTION_NOT_FOUND;
  }

  response->set_result_code(code);

  DVLOG(3) << getPartitionName() << " process " << request->action_op_name() << " action done.";
  auto respMsg = msg->createResponse();
  respMsg->setOperationName(RDD_ACTION_RESPONSE);
  respMsg->setPayload(response);
  sendMessage(respMsg);
}

PbMessagePtr PairRddPartition::get(const PbMessagePtr& key) const {
  PbMessagePtr value;
  if (rddLocal->getPersistType() == PersistType::ORDERED) {
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

vector<PbMessagePtr> PairRddPartition::getValue(const PbMessagePtr& key) const {
  vector<PbMessagePtr> value;
  if (rddLocal->getPersistType() == PersistType::ORDERED) {
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

bool PairRddPartition::containKey(const PbMessagePtr& key) const {
  if (rddLocal->getPersistType() == PersistType::ORDERED) {
    auto it = orderedDataMap.find(key);
    return (it != orderedDataMap.end());
  } else {
    auto it = unorderedDataMap.find(key);
    return (it != unorderedDataMap.end());
  }
}

void PairRddPartition::flushLocalCache() {
  if (!localCache.empty()) {
    PbMessagePtr key;
    std::vector<PbMessagePtr> values;
    for (auto it = localCache.begin(); it != localCache.end(); ++it) {
      if (idgs::store::equals_to()(const_cast<PbMessagePtr&>(it->first), key)) {
        values.push_back(it->second);
      } else {
        if (!values.empty()) {
          repartition(key, values);
          values.clear();
        }
        values.clear();
        key = it->first;
        values.push_back(it->second);
      }
    }

    if (!values.empty()) {
      repartition(key, values);
      values.clear();
    }

    localCache.clear();
  }
}

void PairRddPartition::put(const PbMessagePtr key, std::vector<PbMessagePtr>& value) {
  if (!key.get()) {
    LOG(ERROR)<< getPartitionName() << " put data, key is null";
    return;
  }

  if(value.empty()) {
    LOG(ERROR) << getPartitionName() << " put data, value is empty";
    return;
  }

  shared_ptr<PartitionStoreOption> payload = make_shared<PartitionStoreOption>();
  payload->set_unique(false);

  auto message = createActorMessage();
  message->setOperationName(PARTITION_STORE);
  message->setDestActorId(message->getSourceActorId());
  message->setDestMemberId(message->getSourceMemberId());
  message->setPayload(payload);
  message->setAttachment(RE_PARTITION_KEY, key);
  auto it = value.begin();
  for (; it != value.end(); ++ it) {
    message->setAttachment(RE_PARTITION_VALUE, (* it));
  }

  sendMessage(message);
}

void PairRddPartition::put(const PbMessagePtr key, const PbMessagePtr value, const bool& unique) {
  if (!key.get()) {
    LOG(ERROR)<< getPartitionName() << "put data, key is null, call stack: " << idgs::util::stacktrace();
    return;
  }

  if(!value.get()) {
    LOG(ERROR) << getPartitionName() << " put data, value is null, key: " << key->DebugString() << ", call stack: " << std::endl << idgs::util::stacktrace();
    return;
  }

  shared_ptr<PartitionStoreOption> payload = make_shared<PartitionStoreOption>();
  payload->set_unique(unique);

  auto message = createActorMessage();
  message->setOperationName(PARTITION_STORE);
  message->setDestActorId(message->getSourceActorId());
  message->setDestMemberId(message->getSourceMemberId());
  message->setPayload(payload);
  message->setAttachment(RE_PARTITION_KEY, key);
  message->setAttachment(RE_PARTITION_VALUE, value);

  sendMessage(message);
}

void PairRddPartition::persist(const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value, const bool& unique) {
  if (!key.get()) {
    LOG(ERROR)<< getPartitionName() << " persist data, key is null";
    return;
  }

  if(!value.get()) {
    LOG(ERROR) << getPartitionName() << " persist data, value is null";
    return;
  }

  if (rddLocal->getPersistType() == PersistType::ORDERED) {
    if (unique) {
      orderedDataMap[key].clear();
    }
    orderedDataMap[key].push_back(value);
  } else if (rddLocal->getPersistType() == PersistType::UNORDERED) {
    if (unique) {
      unorderedDataMap[key].clear();
    }
    unorderedDataMap[key].push_back(value);
  }
}

void PairRddPartition::repartition(const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
  if (!key.get()) {
    LOG(ERROR)<< getPartitionName() << " repartition data, key is null";
    return;
  }

  if(!value.get()) {
    LOG(ERROR) << getPartitionName() << " repartition data, value is null";
    return;
  }

  if (useRepartitionLocalCache) {
    localCache.insert(std::pair<PbMessagePtr, PbMessagePtr>(key, value));

    auto cluster = idgs_application()->getClusterFramework();
    static idgs::pb::ClusterConfig* clusterConfig = cluster->getClusterConfig();

    if(localCache.size() > clusterConfig->batch_message()) {
      flushLocalCache();
    }
    return;
  }

  auto p = calcPartitionId(key);
  ActorMessagePtr message = createActorMessage();
  message->setSerdesType(idgs::pb::PB_BINARY);
  message->setOperationName(RE_PARTITION);
  message->setDestActorId(rddLocal->getPartitionActor(p).actor_id());
  message->setDestMemberId(rddLocal->getPartitionActor(p).member_id());
  message->setPayload(make_shared<RddRequest>());

  message->setAttachment(RE_PARTITION_KEY, key);
  message->setAttachment(RE_PARTITION_VALUE, value);

  sendMessage(message);
}

void PairRddPartition::repartition(const idgs::actor::PbMessagePtr& key, std::vector<idgs::actor::PbMessagePtr>& value) {
  if (!key.get()) {
    LOG(ERROR)<< getPartitionName() << " repartition data, key is null";
    return;
  }

  if(value.empty()) {
    LOG(ERROR) << getPartitionName() << " repartition data, value is empty";
    return;
  }

  auto p = calcPartitionId(key);
  ActorMessagePtr message = createActorMessage();
  message->setSerdesType(idgs::pb::PB_BINARY);
  message->setOperationName(RE_PARTITION);
  message->setDestActorId(rddLocal->getPartitionActor(p).actor_id());
  message->setDestMemberId(rddLocal->getPartitionActor(p).member_id());
  message->setPayload(make_shared<RddRequest>());

  message->setAttachment(RE_PARTITION_KEY, key);

  for (PbMessagePtr& v : value) {
    message->setAttachment(RE_PARTITION_VALUE, v);
  }

  sendMessage(message);
}

size_t PairRddPartition::calcPartitionId(const idgs::actor::PbMessagePtr& key) {
  hashcode_t hash_code = HashCode::hashcode(key.get());
  static size_t partitionSize = idgs_application()->getClusterFramework()->getPartitionCount();
  size_t p = hash_code % partitionSize;
  return p;
}

void PairRddPartition::processRow(const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value, const bool& putUnique) {
  if (rddLocal->isRepartition()) {
    repartition(key, value);
    return;
  }

  if (rddLocal->getPersistType() == NONE) {
    if (!key.get()) {
      LOG(ERROR)<< getPartitionName() << "put data, key is null, call stack: " << idgs::util::stacktrace();
      return;
    }

    if(!value.get()) {
      LOG(ERROR) << getPartitionName() << " put data, value is null, key: " << key->DebugString() << ", call stack: " << std::endl << idgs::util::stacktrace();
      return;
    }
    transform(key, value);
  } else {
    if (!key.get()) {
      LOG(ERROR)<< getPartitionName() << "put data, key is null, call stack: " << idgs::util::stacktrace();
      return;
    }

    if(!value.get()) {
      LOG(ERROR) << getPartitionName() << " put data, value is null, key: " << key->DebugString() << ", call stack: " << std::endl << idgs::util::stacktrace();
      return;
    }
    put(key, value, putUnique);
  }
}

bool PairRddPartition::empty() const {
  if (rddLocal->getPersistType() == PersistType::ORDERED) {
    return orderedDataMap.empty();
  } else if (rddLocal->getPersistType() == PersistType::UNORDERED) {
    return unorderedDataMap.empty();
  }

  return true;
}

size_t PairRddPartition::keySize() const {
  if (rddLocal->getPersistType() == PersistType::ORDERED) {
    return orderedDataMap.size();
  } else if (rddLocal->getPersistType() == PersistType::UNORDERED) {
    return unorderedDataMap.size();
  }

  return 0;
}

size_t PairRddPartition::valueSize() const {
  size_t sum = 0;
  if (rddLocal->getPersistType() == PersistType::ORDERED) {
    auto it = orderedDataMap.begin();
    for (; it != orderedDataMap.end(); ++it) {
      sum += it->second.size();
    }
  } else if (rddLocal->getPersistType() == PersistType::UNORDERED) {
    auto it = unorderedDataMap.begin();
    for (; it != unorderedDataMap.end(); ++it) {
      sum += it->second.size();
    }
  }

  return sum;
}

void PairRddPartition::foreach(RddEntryFunc fn) const {
  if (rddLocal->getPersistType() == PersistType::ORDERED) {
    auto it = orderedDataMap.begin();
    for (; it != orderedDataMap.end(); ++it) {
      for (size_t i = 0; i < it->second.size(); ++i) {
        fn(it->first, it->second[i]);
      }
    }
  } else if (rddLocal->getPersistType() == PersistType::UNORDERED) {
    auto it = unorderedDataMap.begin();
    for (; it != unorderedDataMap.end(); ++it) {
      for (size_t i = 0; i < it->second.size(); ++i) {
        fn(it->first, it->second[i]);
      }
    }
  }
}

void PairRddPartition::foreachGroup(RddGroupEntryFunc fn) const {
  if (rddLocal->getPersistType() == PersistType::ORDERED) {
    auto it = orderedDataMap.begin();
    for (; it != orderedDataMap.end(); ++it) {
      fn(it->first, it->second);
    }
  } else if (rddLocal->getPersistType() == PersistType::UNORDERED) {
    auto it = unorderedDataMap.begin();
    for (; it != unorderedDataMap.end(); ++it) {
      fn(it->first, it->second);
    }
  }
}

bool PairRddPartition::parse(idgs::actor::ActorMessagePtr& msg) {
  idgs::actor::Actor::parse(msg);

  auto& opName = msg->getOperationName();
  if (opName == RE_PARTITION || opName == PARTITION_STORE) {
    PbMessagePtr key(rddLocal->getKeyTemplate()->New());
    if (!msg->parseAttachment1(RE_PARTITION_KEY, key)) {
      LOG(ERROR)<< "repartition key data is invalid";
      return false;
    }

    PbMessagePtr value(rddLocal->getValueTemplate()->New());
    if (!msg->parseAttachment1(RE_PARTITION_VALUE, value)) {
      LOG(ERROR)<< "repartition value data is invalid";
      return false;
    }

    msg->getRawAttachments().clear();
  }

  return true;
}

} // namespace rdd
} // namespace idgs 
