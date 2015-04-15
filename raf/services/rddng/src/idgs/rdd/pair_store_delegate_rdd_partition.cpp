
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "pair_store_delegate_rdd_partition.h"

#include "idgs/rdd/rdd_local.h"
#include "idgs/rdd/rdd_module.h"
#include "idgs/rdd/pb/rdd_internal.pb.h"

#include "idgs/store/store_module.h"

using namespace idgs::pb;
using namespace idgs::actor;
using namespace idgs::store;
using namespace idgs::rdd::pb;
using namespace google::protobuf;

namespace idgs {
namespace rdd {

ActorDescriptorPtr PairStoreDelegateRddPartition::descriptor;

PairStoreDelegateRddPartition::PairStoreDelegateRddPartition(const string& rddname, const uint32_t& partitionId) : active(false) {
  rddName = rddname;
  partition = partitionId;
  partitionName = "RDD " + rddName + "[" + std::to_string(partition) + "]";
}

PairStoreDelegateRddPartition::~PairStoreDelegateRddPartition() {
}

const ActorMessageHandlerMap& PairStoreDelegateRddPartition::getMessageHandlerMap() const {
  static map<string, ActorMessageHandler> handlerMap = {
      {RDD_TRANSFORM,         static_cast<ActorMessageHandler>(&PairStoreDelegateRddPartition::handleRddTransform)},
      {RDD_ACTION_REQUEST,    static_cast<ActorMessageHandler>(&PairStoreDelegateRddPartition::handleActionRequest)},
      {RDD_STORE_LISTENER,    static_cast<ActorMessageHandler>(&PairStoreDelegateRddPartition::handleRddStoreListener)}
  };
  return handlerMap;
}

ActorDescriptorPtr PairStoreDelegateRddPartition::generateActorDescriptor() {
  static shared_ptr<ActorDescriptorWrapper> descriptor;
  if (descriptor) {
    return descriptor;
  }

  descriptor = std::make_shared<ActorDescriptorWrapper>();

  descriptor->setName(PAIR_STORE_DELEGATE_RDD_PARTITION);
  descriptor->setDescription("Partition of store Delegate RDD");
  descriptor->setType(AT_STATEFUL);

  // in operation
  ActorOperationDescriporWrapper rddTransform;
  rddTransform.setName(RDD_TRANSFORM);
  rddTransform.setDescription("Handle transform for current RDD partition.");
  rddTransform.setPayloadType("idgs.rdd.pb.RddRequest");
  descriptor->setInOperation(rddTransform.getName(), rddTransform);

  ActorOperationDescriporWrapper actionRequest;
  actionRequest.setName(RDD_ACTION_REQUEST);
  actionRequest.setDescription("Handle action.");
  actionRequest.setPayloadType("idgs.rdd.pb.ActionRequest");
  descriptor->setInOperation(actionRequest.getName(), actionRequest);

  ActorOperationDescriporWrapper rddStoreListener;
  rddStoreListener.setName(RDD_STORE_LISTENER);
  rddStoreListener.setDescription("Handle listener from store when insert data.");
  rddStoreListener.setPayloadType("idgs.rdd.pb.RddRequest");
  descriptor->setInOperation(rddStoreListener.getName(), rddStoreListener);

  // out operation
  // no out descriptor for STORE_DELEGATE_RDD_PARTITION

  // out descriptor for RDD_TRANSFORM
  ActorOperationDescriporWrapper transComplete;
  transComplete.setName(PARTITION_TRANSFORM_COMPLETE);
  transComplete.setDescription("Send transform complete to RDD");
  transComplete.setPayloadType("idgs.rdd.pb.RddResponse");
  descriptor->setOutOperation(transComplete.getName(), transComplete);

  // out descriptor for RDD_ACTION_REQUEST
  ActorOperationDescriporWrapper actionResponse;
  actionResponse.setName(RDD_ACTION_RESPONSE);
  actionResponse.setDescription("Response when action done.");
  actionResponse.setPayloadType("idgs.rdd.pb.RddResponse");
  descriptor->setOutOperation(actionResponse.getName(), actionResponse);

  PairStoreDelegateRddPartition::descriptor = descriptor;
  return descriptor;
}

const ActorDescriptorPtr& PairStoreDelegateRddPartition::getDescriptor() const {
  return descriptor;
}

void PairStoreDelegateRddPartition::initPartitionStore(const idgs::store::StorePtr& store) {
  schemaName = store->getStoreConfigWrapper()->getSchema();
  storeName = store->getStoreConfigWrapper()->getStoreConfig().name();

  auto& storeConfigWrapper = store->getStoreConfigWrapper();
  switch (storeConfigWrapper->getStoreConfig().partition_type()) {
    case idgs::store::pb::PARTITION_TABLE: {
      PartitionStore* pstore = dynamic_cast<PartitionStore*>(store.get());
      pstore->scanPartitionData(partition, dataMap);
      break;
    }
    case idgs::store::pb::REPLICATED: {
      ReplicatedStore* rstore = dynamic_cast<ReplicatedStore*>(store.get());
      rstore->scanData(dataMap);
      break;
    }
    default: {
      break;
    }
  }
}

void PairStoreDelegateRddPartition::handleRddTransform(const idgs::actor::ActorMessagePtr& msg) {
  if (!active) {
    active = true;
  }

  transform();

  rddLocal->setPartitionState(partition, TRANSFORM_COMPLETE);
  shared_ptr<RddResponse> response = make_shared<RddResponse>();
  response->set_partition(partition);
  response->set_result_code(RRC_SUCCESS);

  auto respMsg = createActorMessage();
  respMsg->setDestActorId(rddLocal->getRddId().actor_id());
  respMsg->setDestMemberId(rddLocal->getRddId().member_id());
  respMsg->setOperationName(PARTITION_TRANSFORM_COMPLETE);
  respMsg->setPayload(response);
  sendMessage(respMsg);
}

void PairStoreDelegateRddPartition::handleActionRequest(const idgs::actor::ActorMessagePtr& msg) {
  ActionRequest* request = dynamic_cast<ActionRequest*>(msg->getPayload().get());
  DVLOG(3) << getPartitionName() << " run " << request->action_op_name() << " action";

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
          for (int32_t i = 0; i < result.size(); ++i) {
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

  VLOG(2) << getPartitionName() << " process " << request->action_op_name() << " action done.";
  auto respMsg = msg->createResponse();
  respMsg->setOperationName(RDD_ACTION_RESPONSE);
  respMsg->setPayload(response);
  idgs::actor::sendMessage(respMsg);
}

void PairStoreDelegateRddPartition::handleRddStoreListener(const idgs::actor::ActorMessagePtr& msg) {
  if (!active) {
    return;
  }

  PbMessagePtr key(rddLocal->getKeyTemplate()->New());
  if (!msg->parseAttachment1(RE_PARTITION_KEY, key)) {
    LOG(ERROR)<< "RDD store listener key data is invalid";
    return;
  }

  PbMessagePtr value(rddLocal->getValueTemplate()->New());
  if (!msg->parseAttachment1(RE_PARTITION_VALUE, value)) {
    LOG(ERROR)<< "RDD store listener value data is invalid";
    return;
  }

  transform(key, value);
}

PbMessagePtr PairStoreDelegateRddPartition::get(const PbMessagePtr& key) const {
  PbMessagePtr value;

  StoreValue<google::protobuf::Message> storeValue;
  ResultCode rc = dataMap->get(key, storeValue);
  if (rc == RC_OK) {
    value = storeValue.get();
  }
  return value;
}

vector<PbMessagePtr> PairStoreDelegateRddPartition::getValue(const PbMessagePtr& key) const {
  vector<PbMessagePtr> value;

  StoreValue<google::protobuf::Message> storeValue;
  ResultCode rc = dataMap->get(key, storeValue);
  if (rc == RC_OK) {
    value.push_back(storeValue.get());
  }
  return value;
}

bool PairStoreDelegateRddPartition::containKey(const PbMessagePtr& key) const {
  StoreValue<google::protobuf::Message> storeValue;
  ResultCode rc = dataMap->get(key, storeValue);
  return (rc == RC_OK);
}

bool PairStoreDelegateRddPartition::empty() const {
  return dataMap->size() == 0;
}

size_t PairStoreDelegateRddPartition::keySize() const {
  return dataMap->size();
}

size_t PairStoreDelegateRddPartition::valueSize() const {
  return dataMap->size();
}

void PairStoreDelegateRddPartition::foreach(RddEntryFunc fn) const {
  if (!dataMap) {
    LOG(ERROR)<< "store " << schemaName << "." << storeName << " in partition " << partition << " is not found";
    return;
  }

  dataMap->foreach([&fn] (const StoreKey<Message>& key, const StoreValue<Message>& value) {
    fn(key, value.get());
  });
}

void PairStoreDelegateRddPartition::foreachGroup(RddGroupEntryFunc fn) const {
  if (!dataMap) {
    LOG(ERROR)<< "store " << schemaName << "." << storeName << " in partition " << partition << " is not found";
    return;
  }

  dataMap->foreach([&fn] (const StoreKey<Message>& key, const StoreValue<Message>& value) {
    std::vector<PbMessagePtr> values;
    values.push_back(value.get());
    fn(key, values);
  });
}

} // namespace rdd
} // namespace idgs 
