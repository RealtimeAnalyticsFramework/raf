
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/store/store_config_wrapper.h"
#include "protobuf/message_helper.h"
#include "protobuf/hash_code.h"
#include "idgs/cluster/cluster_framework.h"
#include "idgs/store/store_listener_factory.h"

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
}

void StoreConfigWrapper::setStoreConfig(const ::idgs::store::pb::StoreConfig& storeConfig) {
  this->storeConfig.CopyFrom(storeConfig);

  for (int32_t i = 0; i < storeConfig.listener_configs_size(); ++i) {
    addListenerConfig(storeConfig.listener_configs(i));
  }

  keyTemplate = const_cast<google::protobuf::Message*>(idgs::util::singleton<protobuf::MessageHelper>::getInstance().getPbPrototype(storeConfig.key_type()));
  valueTemplate = const_cast<google::protobuf::Message*>(idgs::util::singleton<protobuf::MessageHelper>::getInstance().getPbPrototype(storeConfig.value_type()));

  if(storeConfig.has_partitioner()) {
    std::shared_ptr<google::protobuf::Message> tempKey(keyTemplate->New());
    std::shared_ptr<google::protobuf::Message> tempValue(valueTemplate->New());
    auto rc = idgs::expr::ExpressionFactory::build(&partitioner, storeConfig.partitioner(), tempKey, tempValue);
    if (rc != RC_OK) {
      LOG(ERROR) << "failed to parse store partitioner: " << storeConfig.partitioner().DebugString();
      // LOG(ERROR) << idgs::util::stacktrace();
    }
  }
}

const ::idgs::store::pb::StoreConfig& StoreConfigWrapper::getStoreConfig() const {
  return storeConfig;
}

//ResultCode StoreConfigWrapper::getListenerConfig(const string& listenerName, ListenerConfig& listenerConfig) {
//  if (listenerMap.find(listenerName) == listenerMap.end()) {
//    return RC_LISTENER_CONFIG_NOT_FOUND;
//  }
//
//  listenerConfig = listenerMap[listenerName];
//  return RC_SUCCESS;
//}
//
//ResultCode StoreConfigWrapper::getListenerParam(const string& listenerName, const string& paramName,
//    string& paramValue) {
//  if (paramMap.find(listenerName) == paramMap.end()) {
//    return RC_LISTENER_CONFIG_NOT_FOUND;
//  }
//
//  if (paramMap[listenerName].find(paramName) == paramMap[listenerName].end()) {
//    return RC_LISTENER_PARAM_NOT_FOUND;
//  }
//
//  paramValue = paramMap[listenerName][paramName];
//  return RC_SUCCESS;
//}
//
ResultCode StoreConfigWrapper::addListenerConfig(const ListenerConfig& listenerConfig) {
//  listenerMap[listenerConfig.name()] = listenerConfig;

  std::map<std::string, std::string> props;
  for (int32_t i = 0; i < listenerConfig.params_size(); ++i) {
//    paramMap[listenerConfig.name()][listenerConfig.params(i).name()] = listenerConfig.params(i).value();
    props[listenerConfig.params(i).name()] = listenerConfig.params(i).value();
  }

  StoreListener* listener = NULL;
  auto code = StoreListenerFactory::build(listenerConfig.name(), shared_from_this(), &listener, props);
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

ResultCode StoreConfigWrapper::parseKey(const std::string& buff, std::shared_ptr<google::protobuf::Message>& message) {
  message.reset(keyTemplate->New());
  auto r = protobuf::ProtoSerdes<0>::deserialize(buff, message.get());
  if (r) {
    return RC_OK;
  } else {
    return RC_INVALID_KEY;
  }
}

ResultCode StoreConfigWrapper::parseValue(const std::string& buff, std::shared_ptr<google::protobuf::Message>& message) {
  message.reset(valueTemplate->New());
  auto r = protobuf::ProtoSerdes<0>::deserialize(buff, message.get());
  if (r) {
    return RC_OK;
  } else {
    return RC_INVALID_KEY;
  }
}
ResultCode StoreConfigWrapper::calculatePartitionStatus(const std::shared_ptr<google::protobuf::Message>& key, PartitionStatus* ps) {
  size_t hash;
  if(unlikely(!ps)) {
    return RC_ERROR;
  }
  if (partitioner) {
    idgs::expr::ExpressionContext ctx;
    ctx.setKey(&key);
    hash = partitioner->evaluate(&ctx).hashcode();
  } else {
    hash = protobuf::HashCode::hashcode(key.get());
  }

  // calculate partition ID
  static auto& cf = ::idgs::util::singleton<idgs::cluster::ClusterFramework>::getInstance();
  int32_t partSize = cf.getPartitionCount();
  ps->partitionId = (hash % partSize);

  // member Id is only available in run time.
  auto& table = cf.getPartitionManager()->getPartitionTable();
  if(likely(table.size() > ps->partitionId)) {
    idgs::cluster::PartitionWrapper* partition = cf.getPartitionManager()->getPartition(ps->partitionId);
    if(likely(partition->getMemberCount() > 0)) {
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
