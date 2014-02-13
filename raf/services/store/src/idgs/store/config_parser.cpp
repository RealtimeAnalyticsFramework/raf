
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "config_parser.h"

#include "idgs/util/utillity.h"

#include "protobuf/message_helper.h"

using namespace std;
using namespace protobuf;
using namespace idgs::store::pb;

namespace idgs {
namespace store {

// class ConfigParser
StoreConfigParser::StoreConfigParser() {
}

StoreConfigParser::~StoreConfigParser() {
  function_footprint();
}

ResultCode StoreConfigParser::parseStoreConfig(const string& configFilePath, StoreConfigWrappers& storeConfigWrappers) {
  DVLOG(2) << "start parse config file: " << configFilePath;
  DataStoreConfig dataStoreConfig;

  ResultCode resultCode = JsonMessage().parseJsonFromFile(&dataStoreConfig, configFilePath);

  if (resultCode != RC_SUCCESS) {
    LOG(ERROR)<< "parse store config error, caused by " << getErrorDescription(resultCode);
    return resultCode;
  }

  auto it = dataStoreConfig.schemas().begin();
  int32_t i = 0;
  for (; it != dataStoreConfig.schemas().end(); ++ it) {
    if (it->has_proto_filename()) {
      const string& filename = it->proto_filename();
      DVLOG(2) << "register protobuf file : " << filename;
      auto code = idgs::util::singleton<MessageHelper>::getInstance().registerDynamicMessage(filename);
      if (code != RC_SUCCESS) {
        LOG(ERROR) << "error when register protobuf file : " << filename << " casused by " << getErrorDescription(code);
        return code;
      }
    }

    if (it->has_proto_content()) {
      const string& content = it->proto_content();
      auto pos = configFilePath.find_last_of("/");
      string dir = configFilePath.substr(0, pos);
      string filename = dir + "dynamic_proto_file.proto";
      sys::saveFile(filename, content);
      DVLOG(2) << "register protobuf with content : " << content;
      auto code = idgs::util::singleton<MessageHelper>::getInstance().registerDynamicMessage(filename);
      if (code != RC_SUCCESS) {
        LOG(ERROR) << "error when register protobuf with content : " << content << " casused by " << getErrorDescription(code);
        remove(filename.c_str());
        return code;
      }

      remove(filename.c_str());
    }

    auto storeIt = it->store_config().begin();
    for (; storeIt != it->store_config().end(); ++ storeIt) {
      string keyType = storeIt->key_type();
      if (!::idgs::util::singleton<MessageHelper>::getInstance().isMessageRegistered(keyType)) {
        LOG(ERROR)<< "store " << storeIt->name() << ", key type " << keyType << " is not register to system.";
        return RC_KEY_TYPE_NOT_REGISTERED;
      }

      string valueType = storeIt->value_type();
      if (!::idgs::util::singleton<MessageHelper>::getInstance().isMessageRegistered(valueType)) {
        LOG(ERROR)<< "store " << storeIt->name() << ", value type " << valueType << " is not register to system.";
        return RC_VALUE_TYPE_NOT_REGISTERED;
      }

      std::shared_ptr<StoreConfigWrapper> storeConfigWrapper = std::make_shared<StoreConfigWrapper>();

      storeConfigWrapper->setStoreConfig(* storeIt);
      storeConfigWrappers.addStoreConfig(storeConfigWrapper);
    }
  }

  return RC_SUCCESS;
}

// class ConfigParser

}// namespace store
} // namespace idgs
