
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__)
#include "idgs_gch.h"
#endif // GNUC_ $

#include "data_store.h"

#include "idgs/application.h"
#include "idgs/store/config_parser.h"
#include "idgs/store/listener/store_listener_factory.h"
#include "idgs/util/utillity.h"


namespace idgs {
namespace store {

// class DataStore
DataStore::DataStore() {
  isInited = false;
}

DataStore::~DataStore() {
  function_footprint();
}

ResultCode DataStore::initialize(const std::string& configFilePath) {
  DVLOG(1) << "Load data store configuration: " << configFilePath;
  auto status = loadCfgFile(configFilePath);
  if (status != RC_SUCCESS) {
    LOG(ERROR)<< "Failed to load data store config. Error code : " << status << ", message : " << getErrorDescription(status) << ".";
    return status;
  }

  isInited = true;
  return RC_SUCCESS;
}

ResultCode DataStore::start() {
  ResultCode status = startDataStore();
  if (status != RC_SUCCESS) {
    DVLOG(2) << "Failed to initialize store of data. caused by " << getErrorDescription(status);
    return status;
  }

  return RC_SUCCESS;
}

ResultCode DataStore::stop() {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);

  schemaMap.clear();
  return RC_SUCCESS;
}

ResultCode DataStore::loadCfgFile(const std::string& configFilePath) {
  pb::DataStoreConfig config;
  auto code = StoreConfigParser::parseStoreConfigFromFile(configFilePath, &config);
  if (code != RC_SUCCESS) {
    return code;
  }

  return createDataStore(config);
}

ResultCode DataStore::createDataStore(const std::string& content, bool start) {
  pb::DataStoreConfig config;
  auto code = StoreConfigParser::parseStoreConfigFromString(content, &config);
  if (code != RC_SUCCESS) {
    return code;
  }

  return createDataStore(config);
}

ResultCode DataStore::createDataStore(const pb::DataStoreConfig& dataStoreConfig, bool start) {
  pb::DataStoreConfig config(dataStoreConfig);
  auto code = StoreConfigParser::parseStoreConfig(&config);
  if (code != RC_SUCCESS) {
    return RC_PARSE_CONFIG_ERROR;
  }

  auto localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();

  auto schemas = config.schemas();
  auto it = schemas.begin();
  for (; it != schemas.end(); ++ it) {
    tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
    auto sit = schemaMap.find(it->schema_name());
    if (sit == schemaMap.end()) {
      schemaMap.insert(std::pair<std::string, StoreSchemaPtr>(it->schema_name(), std::make_shared<StoreSchema>(it->schema_name())));
    }

    auto& schema = schemaMap[it->schema_name()];
    auto& helper = schema->getMessageHelper();

    if (it->has_proto_filename()) {
      const std::string& filename = it->proto_filename();
      DVLOG(2) << "register protobuf file : " << filename;
      do {
        code = helper.registerDynamicMessage(filename);
        if (code == RC_SUCCESS) {
          break;
        }

        std::string idgsHome = "..";
        if (getenv("IDGS_HOME")) {
          idgsHome = getenv("IDGS_HOME");
        }

        std::string path = idgsHome + "/" + filename;
        code = helper.registerDynamicMessage(path);
        if (code != RC_SUCCESS) {
          LOG(ERROR) << "error when register protobuf file : " << path << " casused by " << getErrorDescription(code);
          return code;
        }
      } while (false);
    }

    if (it->has_proto_content()) {
      const std::string& content = it->proto_content();
      std::string filename = "/tmp/idgs/proto/dynamic_proto_file_m" + std::to_string(localMemberId) + ".proto";
      sys::saveFile(filename, content);
      DVLOG(2) << "register protobuf with content : " << content;
      code = helper.registerDynamicMessage(filename);
      if (code != RC_SUCCESS) {
        LOG(ERROR) << "error when register protobuf with content : " << content << " casused by " << getErrorDescription(code);
        remove(filename.c_str());
        return code;
      }

      remove(filename.c_str());
    }

    auto storeIt = it->store_config().begin();
    for (; storeIt != it->store_config().end(); ++ storeIt) {
      std::string keyType = storeIt->key_type();
      if (!helper.isMessageRegistered(keyType)) {
        LOG(ERROR)<< "store " << storeIt->name() << ", key type " << keyType << " is not register to system.";
        return RC_KEY_TYPE_NOT_REGISTERED;
      }

      std::string valueType = storeIt->value_type();
      if (!helper.isMessageRegistered(valueType)) {
        LOG(ERROR)<< "store " << storeIt->name() << ", value type " << valueType << " is not register to system.";
        return RC_VALUE_TYPE_NOT_REGISTERED;
      }

      StoreConfigWrapperPtr storeConfigWrapper = std::make_shared<StoreConfig>();
      storeConfigWrapper->setSchema(it->schema_name());

      auto key = helper.getPbPrototype(keyType);
      auto value = helper.getPbPrototype(valueType);
      storeConfigWrapper->setMessageTemplate(key, value);

      code = storeConfigWrapper->setStoreConfig(* storeIt);
      if (code != RC_SUCCESS) {
        return code;
      }

      code = schema->addStore(storeConfigWrapper);
      if (code != RC_SUCCESS) {
        return code;
      }

      if (start) {
        code = schema->getStore(storeIt->name())->initialize();
        if (code != RC_SUCCESS) {
          return code;
        }
      }
    }
  }

  return code;
}

ResultCode DataStore::dropSchema(const std::string& schemaName) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  auto it = schemaMap.find(schemaName);
  if (it == schemaMap.end()) {
    return RC_SCHEMA_NOT_FOUND;
  }

  it->second->drop();
  schemaMap.erase(it);

  return RC_SUCCESS;
}

ResultCode DataStore::dropStore(const std::string& schemaName, const std::string& storeName) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, true);
  auto it = schemaMap.find(schemaName);
  if (it == schemaMap.end()) {
    return RC_SCHEMA_NOT_FOUND;
  }

  return it->second->removeStore(storeName);
}

ResultCode DataStore::startDataStore() {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
  auto it = schemaMap.begin();
  for (; it != schemaMap.end(); ++ it) {
    auto& stores = it->second->getStores();
    for (auto& en : stores) {
      auto& storeConfigWrapper = en.second->getStoreConfig();

      auto code = storeConfigWrapper->buildStoreListener();
      if (code != RC_SUCCESS) {
        return code;
      }

      en.second->initialize();
    }
  }

  return RC_SUCCESS;
}

bool DataStore::isInit() {
  return isInited;
}

StorePtr DataStore::getStore(const std::string& storeName) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
  StorePtr result;
  for (auto it = schemaMap.begin(); it != schemaMap.end(); ++ it) {
    auto store = it->second->getStore(storeName);
    if (store) {
      if (result) {
        LOG(ERROR) << "There are more than ONE stores named " << storeName;
      } else {
        result = store;
      }
    }
  }

  return result;
}

StorePtr DataStore::getStore(const std::string& schemaName, const std::string& storeName) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
  auto it = schemaMap.find(schemaName);
  if (it == schemaMap.end()) {
    LOG(ERROR) << "No schema named " << schemaName;
    StorePtr nullStore;
    return nullStore;
  }

  return it->second->getStore(storeName);
}

ResultCode DataStore::getStores(std::vector<StorePtr>& stores) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);

  for (auto it = schemaMap.begin(); it != schemaMap.end(); ++ it) {
    auto store = it->second->getStores();
    for (auto& en : store) {
      stores.push_back(en.second);
    }
  }

  return RC_SUCCESS;
}

ResultCode DataStore::getStores(const std::string& schemaName, std::vector<StorePtr>& stores) {
  tbb::spin_rw_mutex::scoped_lock lock(mutex, false);
  auto it = schemaMap.find(schemaName);
  if (it == schemaMap.end()) {
    return RC_SCHEMA_NOT_FOUND;
  }

  auto store = it->second->getStores();
  for (auto& en : store) {
    stores.push_back(en.second);
  }

  return RC_SUCCESS;
}

void DataStore::registerStoreListener(StoreListener* listener) {
  StoreListenerFactory::registerStoreListener(listener);
}

void DataStore::unregisterStoreListener(StoreListener* listener) {
  StoreListenerFactory::unregisterStoreListener(listener->getName());
}

} //namespace store
} // namespace idgs
