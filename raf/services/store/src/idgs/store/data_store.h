
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/store/schema/store_schema_wrapper.h"

namespace idgs {
namespace store {

/// Data store interface class. <br>
/// Internal interface to cluster and rpc <br>
class DataStore {
public:

  /// @brief Constructor
  DataStore();

  /// @brief Destructor
  virtual ~DataStore();

  DataStore(const DataStore& other) = delete;
  DataStore(DataStore&& other) = delete;
  DataStore& operator()(const DataStore& other) = delete;
  DataStore& operator()(DataStore&& other) = delete;

  /// @brief  Initialize data store.
  /// @param  configFilePath The path of config file.
  /// @return Status code of result.
  /// @code
  /// // Example
  /// ResultCode status = idgs_store_module()->initialize("store_config.xml");
  /// if (status == RC_CONFIG_FILE_NOT_FOUND) {
  ///   cerr << "Config file not found." << endl;
  /// } else if (status == RC_SUCCESS) {
  ///   cerr << "Initialize success." << endl;
  /// }
  /// @endcode
  ResultCode initialize(const std::string& configFilePath);

  /// @brief  Start datastore service.
  /// @return Status code of result.
  /// @code
  /// // Example
  /// ResultCode status = idgs_store_module()->start();
  /// if (status != RC_CONFIG_FILE_NOT_FOUND) {
  ///   cerr << "Error." << endl;
  /// }
  /// @endcode
  ResultCode start();

  /// @brief  Stop datastore service.
  /// @return Status code of result.
  /// @code
  /// // Example
  /// ResultCode status = idgs_store_module()->stop();
  /// if (status != RC_CONFIG_FILE_NOT_FOUND) {
  ///   cerr << "Error." << endl;
  /// }
  /// @endcode
  ResultCode stop();

  /// @brief  Load data store config file.
  /// @param  configFilePath  Path of data store config file.
  /// @return Status code of result.
  ResultCode loadCfgFile(const std::string& configFilePath);

  ResultCode createDataStore(const std::string& content, bool start = false);

  ResultCode createDataStore(const idgs::store::pb::DataStoreConfig& config, bool start = false);

  ResultCode dropSchema(const std::string& schemaName);

  ResultCode dropStore(const std::string& schemaName, const std::string& storeName);

  /// @brief  Initialize data store.
  /// @return Status code of result.
  ResultCode startDataStore();

  /// @brief  Whether service is initialized.
  /// @return True of false of service initialized.
  bool isInit();

  StorePtr getStore(const std::string& storeName);

  StorePtr getStore(const std::string& schemaName, const std::string& storeName);

  ResultCode getStores(std::vector<StorePtr>& stores);

  ResultCode getStores(const std::string& schemaName, std::vector<StorePtr>& stores);

  void registerStoreListener(StoreListener* listener);

  void unregisterStoreListener(StoreListener* listener);

private:
  bool isInited;

  std::map<std::string, StoreSchemaWrapperPtr> schemaMap;
  tbb::spin_rw_mutex mutex;

};
// class DataStore

}// namespace store
} // namespace idgs
