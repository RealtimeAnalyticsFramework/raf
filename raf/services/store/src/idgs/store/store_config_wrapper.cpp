
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/store/store_config_wrapper.h"

#include "idgs/application.h"

#include "idgs/store/listener/store_listener_factory.h"

#include "protobuf/hash_code.h"

using namespace idgs::store::pb;

namespace idgs {
namespace store {

// class StoreConfigWrapper
StoreConfigWrapper::StoreConfigWrapper() : keyTemplate(NULL), valueTemplate(NULL), partitioner(NULL) {
}

StoreConfigWrapper::~StoreConfigWrapper() {
  keyTemplate = NULL;

  valueTemplate = NULL;

  if(partitioner) {
    delete partitioner;
    partitioner = NULL;
  }

  for (int32_t i = 0; i < listeners.size(); ++ i) {
    auto listener = listeners[i];
    if (listener != NULL) {
      delete listener;
    }
  }

  listeners.clear();
}

ResultCode StoreConfigWrapper::setStoreConfig(const ::idgs::store::pb::StoreConfig& storeConfig) {
  if (!keyTemplate) {
    return RC_INVALID_KEY;
  }

  if (!valueTemplate) {
    return RC_INVALID_VALUE;
  }

  this->storeConfig.CopyFrom(storeConfig);

  if(storeConfig.has_partitioner()) {
    std::shared_ptr<google::protobuf::Message> tempKey(keyTemplate->New());
    std::shared_ptr<google::protobuf::Message> tempValue(valueTemplate->New());
    ResultCode code = idgs::expr::ExpressionFactory::build(&partitioner, storeConfig.partitioner(), tempKey, tempValue);
    if (code != RC_SUCCESS) {
      LOG(ERROR) << "failed to parse store partitioner: " << storeConfig.partitioner().DebugString();
      // LOG(ERROR) << idgs::util::stacktrace();
      return code;
    }
  }

  return RC_SUCCESS;
}

ResultCode StoreConfigWrapper::buildStoreListener() {
  for (int32_t i = 0; i < storeConfig.listener_configs_size(); ++i) {
    ResultCode code = addListenerConfig(storeConfig.listener_configs(i));
    if (code != RC_SUCCESS) {
      return code;
    }
  }

  return RC_SUCCESS;
}

const ::idgs::store::pb::StoreConfig& StoreConfigWrapper::getStoreConfig() const {
  return storeConfig;
}

ResultCode StoreConfigWrapper::addListenerConfig(const ListenerConfig& listenerConfig) {
  auto self = shared_from_this();
  std::map<std::string, std::string> props;
  for (int32_t i = 0; i < listenerConfig.params_size(); ++i) {
    props[listenerConfig.params(i).name()] = listenerConfig.params(i).value();
  }

  StoreListener* listener = NULL;
  auto code = StoreListenerFactory::build(listenerConfig.name(), self, &listener, props);
  if (code != RC_SUCCESS) {
    return code;
  }

  listener->setListenerIndex(listeners.size());
  listeners.push_back(listener);

  return RC_SUCCESS;
}

const std::vector<StoreListener*>& StoreConfigWrapper::getStoreListener() const {
  return listeners;
}

void StoreConfigWrapper::addStoreListener(const StoreListener* listener) {
  listeners.push_back(const_cast<StoreListener*>(listener));
}

void StoreConfigWrapper::setMessageTemplate(const google::protobuf::Message* key, const google::protobuf::Message* value) {
  keyTemplate = const_cast<google::protobuf::Message*>(key);
  valueTemplate = const_cast<google::protobuf::Message*>(value);
}

ResultCode StoreConfigWrapper::parseKey(const protobuf::SerdesMode& mode, const std::string& buff, std::shared_ptr<google::protobuf::Message>& message) {
  message.reset(keyTemplate->New());
  auto r = protobuf::ProtoSerdesHelper::deserialize(mode, buff, message.get());
  if (r) {
    return RC_OK;
  } else {
    return RC_INVALID_KEY;
  }
}

ResultCode StoreConfigWrapper::parseValue(const protobuf::SerdesMode& mode, const std::string& buff, std::shared_ptr<google::protobuf::Message>& message) {
  message.reset(valueTemplate->New());
  auto r = protobuf::ProtoSerdesHelper::deserialize(mode, buff, message.get());
  if (r) {
    return RC_OK;
  } else {
    return RC_INVALID_VALUE;
  }
}

std::shared_ptr<google::protobuf::Message> StoreConfigWrapper::newKey() {
  std::shared_ptr<google::protobuf::Message> message;
  message.reset(keyTemplate->New());
  return message;
}

std::shared_ptr<google::protobuf::Message> StoreConfigWrapper::newValue() {
  std::shared_ptr<google::protobuf::Message> message;
  message.reset(valueTemplate->New());
  return message;
}

const google::protobuf::Message* StoreConfigWrapper::getKeyTemplate() const {
  return keyTemplate;
}

const google::protobuf::Message* StoreConfigWrapper::getValueTemplate() const {
  return valueTemplate;
}

void StoreConfigWrapper::setSchema(const std::string& schemaName) {
  schema = schemaName;
}

const std::string& StoreConfigWrapper::getSchema() const {
  return schema;
}

ResultCode StoreConfigWrapper::calculatePartitionInfo(const std::shared_ptr<google::protobuf::Message>& key, PartitionInfo* ps) {
  size_t hash;
  if(unlikely(!ps)) {
    return RC_ERROR;
  }
  if (partitioner) {
    idgs::expr::ExpressionContext& ctx = idgs::expr::getTlExpressionContext();
    ctx.setKey(&key);
    hash = partitioner->evaluate(&ctx).hashcode();
  } else {
    hash = protobuf::HashCode::hashcode(key.get());
  }

  // calculate partition ID
  static auto cf = idgs_application()->getClusterFramework();
  int32_t partSize = cf->getPartitionCount();
  ps->partitionId = (hash % partSize);

  // member Id is only available in run time.
  auto& table = cf->getPartitionManager()->getPartitionTable();
  if(likely(table.size() > ps->partitionId)) {
    idgs::cluster::PartitionWrapper* partition = cf->getPartitionManager()->getPartition(ps->partitionId);
    if(likely(partition->getReplicaCount() > 0)) {
      ps->memberId = partition->getPrimaryMemberId();
      return RC_OK;
    }
  }
  ps->memberId = 0;
  LOG(WARNING) << "PartitionManager is not initialized";

  return RC_OK;

}


}// namespace store
} // namespace idgs
