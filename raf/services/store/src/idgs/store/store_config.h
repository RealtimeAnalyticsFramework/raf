
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

struct StoreOption {
  uint32_t partitionId;
  uint32_t memberId;

  uint64_t version;
};

/// Wrapper class. <br>
/// Wrap the config of store from config file. <br>
class StoreConfig : public std::enable_shared_from_this<StoreConfig>{
public:

  /// @brief Constructor
  StoreConfig();

  /// @brief Destructor
  virtual ~StoreConfig();

  /// @brief  Put the store config into wrapper.
  /// @param  storeConfig The config of store.
  ResultCode setStoreConfig(const ::idgs::store::pb::StoreConfig& storeConfig);

  /// @brief  Get the config of store.
  /// @return Config of store.
  const ::idgs::store::pb::StoreConfig& getStoreConfig() const;

  /// @brief  Add the listener config and wrap it.
  /// @param  listenerConfig The listener config to wrap and store.
  /// @return Status code of result.
  ResultCode addListenerConfig(const idgs::store::pb::ListenerConfig& listenerConfig);

  ResultCode calculatePartitionInfo(const std::shared_ptr<google::protobuf::Message>& message, StoreOption* ps);

  const std::vector<StoreListener*>& getStoreListener() const;

  void addStoreListener(const StoreListener* listener);

  ResultCode buildStoreListener();

  void setMessageTemplate(const google::protobuf::Message* key, const google::protobuf::Message* value);

  ResultCode parseKey(const protobuf::SerdesMode& mode, const std::string& buff, std::shared_ptr<google::protobuf::Message>& message);

  ResultCode parseValue(const protobuf::SerdesMode& mode, const std::string& buff, std::shared_ptr<google::protobuf::Message>& message);

  std::shared_ptr<google::protobuf::Message> newKey();

  std::shared_ptr<google::protobuf::Message> newValue();

  const google::protobuf::Message* getKeyTemplate() const;

  const google::protobuf::Message* getValueTemplate() const;

  void setSchema(const std::string& schemaName);

  const std::string& getSchema() const;

private:
  ::idgs::store::pb::StoreConfig storeConfig;
  std::string schema;

  google::protobuf::Message* keyTemplate = NULL;
  google::protobuf::Message* valueTemplate = NULL;

  std::vector<StoreListener*> listeners;
  idgs::expr::Expression* partitioner = NULL;
};
// class StoreConfigWrapper

typedef std::shared_ptr<StoreConfig> StoreConfigWrapperPtr;

} // namespace store
} // namespace idgs
