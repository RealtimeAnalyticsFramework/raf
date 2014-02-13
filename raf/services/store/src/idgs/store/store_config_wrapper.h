
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/expr/expression_factory.h"
#include "idgs/store/pb/store_config.pb.h"

namespace idgs {
namespace store {
class StoreListener;

struct PartitionStatus {
  uint32_t partitionId;
  uint32_t memberId;
};

/// Wrapper class. <br>
/// Wrap the config of store from config file. <br>
class StoreConfigWrapper : public std::enable_shared_from_this<StoreConfigWrapper>{
public:

  /// @brief Constructor
  StoreConfigWrapper();

  /// @brief Destructor
  virtual ~StoreConfigWrapper();

  /// @brief  Put the store config into wrapper.
  /// @param  storeConfig The config of store.
  void setStoreConfig(const ::idgs::store::pb::StoreConfig& storeConfig);

  /// @brief  Get the config of store.
  /// @return Config of store.
  const ::idgs::store::pb::StoreConfig& getStoreConfig() const;

  /// @brief  Get the listener config of the store with listener name.
  /// @param  listenerName The name of listener.
  /// @param  listenerConfig The return value of listener config.
  /// @return Status code of result.
//  ResultCode getListenerConfig(const std::string& listenerName, idgs::store::pb::ListenerConfig& listenerConfig);

  /// @brief  Get the parameter of listener with listener name and parameter name.
  /// @param  listenerName The name of listener.
  /// @param  paramName The name of listener parameter.
  /// @param  paramValue The return value of parameter.
  /// @return Status code of result.
//  ResultCode getListenerParam(const std::string& listenerName, const std::string& paramName, std::string& paramValue);

  /// @brief  Add the listener config and wrap it.
  /// @param  listenerConfig The listener config to wrap and store.
  /// @return Status code of result.
  ResultCode addListenerConfig(const idgs::store::pb::ListenerConfig& listenerConfig);

  ResultCode parseKey(const std::string& buff, std::shared_ptr<google::protobuf::Message>& message);
  ResultCode parseValue(const std::string& buff, std::shared_ptr<google::protobuf::Message>& message);
  ResultCode calculatePartitionStatus(const std::shared_ptr<google::protobuf::Message>& message, PartitionStatus* ps);

  const std::vector<StoreListener*>& getStoreListener() const;

private:
//  std::map<std::string, std::map<std::string, std::string>> paramMap;
//  std::map<std::string, idgs::store::pb::ListenerConfig> listenerMap;

  std::vector<StoreListener*> listeners;

  ::idgs::store::pb::StoreConfig storeConfig;

  google::protobuf::Message* keyTemplate = NULL;

  google::protobuf::Message* valueTemplate = NULL;

  idgs::expr::Expression* partitioner = NULL;
};
// class StoreConfigWrapper

}// namespace store
} // namespace idgs
