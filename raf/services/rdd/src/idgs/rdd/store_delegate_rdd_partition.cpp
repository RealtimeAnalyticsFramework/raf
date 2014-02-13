
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "store_delegate_rdd_partition.h"
#include "idgs/store/data_store.h"
#include "protobuf/message_helper.h"

#include "idgs/rdd/rdd_module.h"

using namespace idgs::pb;
using namespace idgs::actor;
using namespace idgs::rdd::pb;
using namespace idgs::store;
using namespace protobuf;
using namespace google::protobuf;

namespace idgs {
namespace rdd {

idgs::actor::ActorDescriptorPtr StoreDelegateRddPartition::descriptor;

StoreDelegateRddPartition::StoreDelegateRddPartition(const uint32_t& partitionId, const std::string& storename) :
    storeName(storename) {
  partition = partitionId;
}

StoreDelegateRddPartition::~StoreDelegateRddPartition() {
}

const ActorMessageHandlerMap& StoreDelegateRddPartition::getMessageHandlerMap() const {
  static std::map<std::string, idgs::actor::ActorMessageHandler> handlerMap = {
      {RDD_PARTITION_PROCESS, static_cast<idgs::actor::ActorMessageHandler>(&StoreDelegateRddPartition::handleProcess)},
      {RDD_ACTION_REQUEST, static_cast<idgs::actor::ActorMessageHandler>(&StoreDelegateRddPartition::handleAction)},
      {RDD_PARTITION_DESTROY, static_cast<idgs::actor::ActorMessageHandler>(&StoreDelegateRddPartition::handleDestory)}
  };
  return handlerMap;
}

ActorDescriptorPtr StoreDelegateRddPartition::generateActorDescriptor() {
  static std::shared_ptr<ActorDescriptorWrapper> descriptor;
  if (descriptor)
    return descriptor;
  descriptor.reset(new ::ActorDescriptorWrapper);

  descriptor->setName(STORE_DELEGATE_RDD_PARTITION);
  descriptor->setDescription("Partition of store Delegate RDD");
  descriptor->setType(AT_STATEFUL);

  // in operation
  ActorOperationDescriporWrapper inRddParititonProcess;
  inRddParititonProcess.setName(RDD_PARTITION_PROCESS);
  inRddParititonProcess.setDescription("Store delegate partition process");
  inRddParititonProcess.setPayloadType("idgs.rdd.pb.CreateDelegateRddRequest");
  descriptor->setInOperation(inRddParititonProcess.getName(), inRddParititonProcess);

  ActorOperationDescriporWrapper inRddActionRequest;
  inRddActionRequest.setName(RDD_ACTION_REQUEST);
  inRddActionRequest.setDescription("The action of count data size for partition.");
  inRddActionRequest.setPayloadType("idgs.rdd.pb.ActionRequest");
  descriptor->setInOperation(inRddActionRequest.getName(), inRddActionRequest);

  ActorOperationDescriporWrapper inSendRddInfo;
  inSendRddInfo.setName(SEND_RDD_INFO);
  inSendRddInfo.setDescription("Receive information of rdd and all it's partition.");
  inSendRddInfo.setPayloadType("idgs.rdd.pb.RddActorInfo");
  descriptor->setInOperation(inSendRddInfo.getName(), inSendRddInfo);

  // out operation
  // out operation for RDD_PARTITION_PROCESS
  ActorOperationDescriporWrapper outRddActionResponse;
  outRddActionResponse.setName(RDD_ACTION_REQUEST);
  outRddActionResponse.setDescription("The response of action.");
  outRddActionResponse.setPayloadType("idgs.rdd.pb.ActorResponse");
  descriptor->setOutOperation(outRddActionResponse.getName(), outRddActionResponse);

  // out operation for SEND_RDD_INFO
  ActorOperationDescriporWrapper outSendRddInfoResponse;
  outSendRddInfoResponse.setName(SEND_RDD_INFO_RESPONSE);
  outSendRddInfoResponse.setDescription("The response of receive information of rdd and all it's partition.");
  outSendRddInfoResponse.setPayloadType("idgs.rdd.pb.RddRequest");
  descriptor->setOutOperation(outSendRddInfoResponse.getName(), outSendRddInfoResponse);

  StoreDelegateRddPartition::descriptor = descriptor;
  return descriptor;
}

const ActorDescriptorPtr& StoreDelegateRddPartition::getDescriptor() const {
  return descriptor;
}

void StoreDelegateRddPartition::handleProcess(const ActorMessagePtr& msg) {
  DVLOG(2) << "partition " << partition << " handle process";
  DataStore& store = ::idgs::util::singleton<DataStore>::getInstance();
  ResultCode code = store.scanPartitionData(storeName, partition, dataMap);
  if (code != RC_SUCCESS) {
    LOG(ERROR)<< "cannot get data of store " << storeName << " in partition " << partition << ", caused by " << getErrorDescription(code);
    return;
  }

  shared_ptr<StoreConfigWrapper> config(new StoreConfigWrapper);
  store.loadStoreConfig(storeName, config);

  MessageHelper& helper = ::idgs::util::singleton<MessageHelper>::getInstance();
  keyTemplate = helper.createMessage(config->getStoreConfig().key_type());
  valueTemplate = helper.createMessage(config->getStoreConfig().value_type());
}

const StoreDataMap& StoreDelegateRddPartition::getData() const {
  return dataMap;
}

void StoreDelegateRddPartition::handleAction(const ActorMessagePtr& msg) {
  ActionRequest* request = dynamic_cast<ActionRequest*>(msg->getPayload().get());
  DVLOG(2) << "Store Delegate RDD " << getRddName() << " partition[" << partition << "] handle " << request->action_op_name() << " action";

  shared_ptr<ActionResponse> response(new ActionResponse);
  response->set_action_id(request->action_id());
  response->set_partition(partition);
  ActionMgr& actionMgr = idgs_rdd_module()->getActionManager();
  const ActionPtr& action = actionMgr.get(request->action_op_name());

  if (action.get()) {
    if (isReplicatedRdd() && partition > 0) {
      response->set_result_code(RRC_SUCCESS);
    } else {
      vector<protobuf::PbVariant> result;
      RddResultCode code = action->action(msg, this, result);
      response->set_result_code(code);

      if (code == RRC_SUCCESS) {
        for (int32_t i = 0; i < result.size(); ++i) {
          response->add_action_value(result[i].toString());
        }
      }
    }
  } else {
    LOG(ERROR)<< "action " << request->action_op_name() << " is not registered.";
    response->set_result_code(RRC_ACTION_NOT_FOUND);
  }

  auto respMsg = msg->createResponse();
  respMsg->setOperationName(RDD_ACTION_RESPONSE);
  respMsg->setPayload(response);
  idgs::actor::sendMessage(respMsg);
}

idgs::actor::PbMessagePtr StoreDelegateRddPartition::get(const idgs::actor::PbMessagePtr& key) const {
  idgs::actor::PbMessagePtr value;
  const StoreDataMap& data = getData();
  if (!data) {
    LOG(ERROR)<< "store " << storeName << " in partition " << partition << " is not found";
    return value;
  }

  StoreValue<google::protobuf::Message> storeValue;
  ResultCode rc = data->get(key, storeValue);
  if (rc == RC_OK) {
    value = storeValue.get();
  }
  return value;
}

vector<idgs::actor::PbMessagePtr> StoreDelegateRddPartition::getValue(const PbMessagePtr& key) const {
  vector<idgs::actor::PbMessagePtr> value;
  const StoreDataMap& data = getData();
  if (!data) {
    LOG(ERROR)<< "store " << storeName << " in partition " << partition << " is not found";
    return value;
  }

  StoreValue<google::protobuf::Message> storeValue;
  ResultCode rc = data->get(key, storeValue);
  if (rc == RC_OK) {
    value.push_back(storeValue.get());
  }
  return value;
}

bool StoreDelegateRddPartition::containKey(const idgs::actor::PbMessagePtr& key) const {
  const StoreDataMap& data = getData();
  if (!data) {
    LOG(ERROR)<< "store " << storeName << " in partition " << partition << " is not found";
    return false;
  }

  StoreValue<google::protobuf::Message> storeValue;
  ResultCode rc = data->get(key, storeValue);
  return (rc == RC_OK);
}

void StoreDelegateRddPartition::put(const idgs::actor::PbMessagePtr& key,
    std::vector<idgs::actor::PbMessagePtr>& value) {
  LOG(WARNING)<< "Store delegate RDD can not put data.";
}

void StoreDelegateRddPartition::put(const idgs::actor::PbMessagePtr& key,
    const idgs::actor::PbMessagePtr& value) {
  LOG(WARNING)<< "Store delegate RDD can not put data.";
}

void StoreDelegateRddPartition::putLocal(const idgs::actor::PbMessagePtr& key,
    std::vector<idgs::actor::PbMessagePtr>& value) {
  LOG(WARNING)<< "Store delegate RDD can not put data.";
}
void StoreDelegateRddPartition::putLocal(const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
  LOG(WARNING)<< "Store delegate RDD can not put data.";
}

bool StoreDelegateRddPartition::empty() const {
  const StoreDataMap& data = getData();
  if (!data) {
    LOG(ERROR)<< "store " << storeName << " in partition " << partition << " is not found";
    return true;
  }

  return data->size() == 0;
}

size_t StoreDelegateRddPartition::size() const {
  const StoreDataMap& data = getData();
  if (!data) {
    LOG(ERROR)<< "store " << storeName << " in partition " << partition << " is not found";
    return 0;
  }
  return data->size();
}

void StoreDelegateRddPartition::foreach(RddEntryFunc fn) const {
  const StoreDataMap& data = getData();
  if (!data) {
    LOG(ERROR)<< "store " << storeName << " in partition " << partition << " is not found";
    return;
  }

  data->foreach([&fn] (const StoreKey<Message>& key, const StoreValue<Message>& value) {
    fn(key, value.get());
  });
}

void StoreDelegateRddPartition::foreachGroup(RddGroupEntryFunc fn) const {
  const StoreDataMap& data = getData();
  if (!data) {
    LOG(ERROR)<< "store " << storeName << " in partition " << partition << " is not found";
    return;
  }

  data->foreach([&fn] (const StoreKey<Message>& key, const StoreValue<Message>& value) {
    vector<shared_ptr<Message>> values;
    values.push_back(value.get());
    fn(key, values);
  });
}

void StoreDelegateRddPartition::handleDestory(const idgs::actor::ActorMessagePtr& msg) {
  delete this;
}

} // namespace rdd
} // namespace idgs 
