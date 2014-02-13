
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once
#include <map>
#include <list>
#include <string>
#include <memory>

#include <tbb/spin_rw_mutex.h>
#include <google/protobuf/message.h>

#include "idgs/result_code.h"
#include "idgs/store/store_ptr.h"
#include "idgs/store/store_config_wrappers.h"
#include "idgs/store/store.h"
#include "data_map.h"

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

  /// Copy constructor, called by containers.
  /// This should be singleton, copy constructor should be deleted.
  DataStore(const DataStore& other) = delete;
  DataStore(DataStore&& other) = delete;
  DataStore& operator()(const DataStore& other) = delete;
  DataStore& operator()(DataStore&& other) = delete;

  /// @brief  Initialize data store.
  /// @param  configFilePath The path of config file.
  /// @return Status code of result.
  /// @code
  /// // Example
  /// ResultCode status = ::idgs::util::singleton<DataStore>::getInstance().initialize("store_config.xml");
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
  /// ResultCode status = ::idgs::util::singleton<DataStore>::getInstance().start();
  /// if (status != RC_CONFIG_FILE_NOT_FOUND) {
  ///   cerr << "Error." << endl;
  /// }
  /// @endcode
  ResultCode start();

  /// @brief  Stop datastore service.
  /// @return Status code of result.
  /// @code
  /// // Example
  /// ResultCode status = ::idgs::util::singleton<DataStore>::getInstance().stop();
  /// if (status != RC_CONFIG_FILE_NOT_FOUND) {
  ///   cerr << "Error." << endl;
  /// }
  /// @endcode
  ResultCode stop();

  /// @brief  Load store config info by name of store.
  /// @param  storeName  The name of store.
  /// @param  storeConfigWrapper Return value of store config info.
  /// @return Status code of result.
  /// @code
  /// // Example
  /// StoreConfigWrapper storeConfigWrapper;
  /// ResultCode status = ::idgs::util::singleton<DataStore>::getInstance().loadStoreConfig("Employee", storeConfigWrapper);
  /// if (status == RC_STORE_NOT_FOUND) {
  ///   cerr << "Store not found." << endl;
  /// } else if (status == RC_SUCCESS) {
  ///   cerr << "Load success." << endl;
  /// }
  /// @endcode
  ResultCode loadStoreConfig(const std::string& storeName, std::shared_ptr<StoreConfigWrapper>& storeConfigWrapper);

  ResultCode registerStoreConfig(const std::shared_ptr<StoreConfigWrapper>& storeConfigWrapper);

  /// @todo remove this method
  /// @brief  Insert data to store.
  /// @param  storeName  The name of store.
  /// @param  key  The key of data to store.
  /// @param  value  The value of data to store.
  /// @return Status code of result.
  /// @code
  /// // Example
  /// Message * key, value; // = new protobuf message
  /// ResultCode status = ::idgs::util::singleton<DataStore>::getInstance().insertData("Employee", key, value);
  /// if (status == RC_KEY_IS_NULL) {
  ///   cerr << "Insert data failed, key is null." << endl;
  /// } else if (status == RC_VALUE_IS_NULL) {
  ///   cerr << "Insert data failed, value is null." << endl;
  /// } else if (status == RC_STORE_NOT_FOUND) {
  ///   cerr << "Store not found." << endl;
  /// } else if (status == RC_SUCCESS) {
  ///   cerr << "Insert data success." << endl;
  /// }
  /// delete key;
  /// delete value;
  /// @endcode
  ResultCode insertData(const std::string& storeName, const StoreKey<google::protobuf::Message>& key,
      StoreValue<google::protobuf::Message>& value, PartitionStatus* ps = NULL);

  /// @todo remove this method
  /// @brief  Get data by key from the store.
  /// @param  storeName  The name of store.
  /// @param  key  The key of data.
  /// @param  value  Return value of data from store.
  /// @return Status code of result.
  /// @code
  /// // Example
  /// Message * key, value; // = new protobuf message
  /// ResultCode status = ::idgs::util::singleton<DataStore>::getInstance().getData("Employee", key, value);
  /// if (status == RC_KEY_IS_NULL) {
  ///   cerr << "Get data failed, key is null." << endl;
  /// } else if (status == RC_VALUE_IS_NULL) {
  ///   cerr << "Get data failed, value is null." << endl;
  /// } else if (status == RC_STORE_NOT_FOUND) {
  ///   cerr << "Store not found." << endl;
  /// } else if (status == RC_DATA_NOT_FOUND) {
  ///   cerr << "Data not found." << endl;
  /// } else if (status == RC_SUCCESS) {
  ///   cerr << "Get data success." << endl;
  /// }
  /// delete key;
  /// delete value;
  /// @endcode
  ResultCode getData(const std::string& storeName, const StoreKey<google::protobuf::Message>& key,
      StoreValue<google::protobuf::Message>& value, PartitionStatus* ps = NULL);

  /// @todo remove this method
  /// @brief  Update data to store.
  /// @param  storeName  The name of store.
  /// @param  key  The key of data to store.
  /// @param  value  The value of data to store.
  /// @return Status code of result.
  /// @code
  /// // Example
  /// Message * key, value; // = new protobuf message
  /// ResultCode status = ::idgs::util::singleton<DataStore>::getInstance().updateData("Employee", key, value);
  /// if (status == RC_KEY_IS_NULL) {
  ///   cerr << "Update data failed, key is null." << endl;
  /// } else if (status == RC_VALUE_IS_NULL) {
  ///   cerr << "Update data failed, value is null." << endl;
  /// } else if (status == RC_STORE_NOT_FOUND) {
  ///   cerr << "Store not found." << endl;
  /// } else if (status == RC_SUCCESS) {
  ///   cerr << "Update data success." << endl;
  /// }
  /// delete key;
  /// delete value;
  /// @endcode
  ResultCode updateData(const std::string& storeName, const StoreKey<google::protobuf::Message>& key,
      StoreValue<google::protobuf::Message>& value, PartitionStatus* ps = NULL);

  /// @todo remove this method
  /// @brief  Remove data by key from store.
  /// @param  storeName  The name of store.
  /// @param  key  The key of data.
  /// @param  value  Return value of removed data from store.
  /// @return Status code of result.
  /// @code
  /// // Example
  /// Message * key; // = new protobuf message
  /// ResultCode status = ::idgs::util::singleton<DataStore>::getInstance().removeData("Employee", key);
  /// if (status == RC_KEY_IS_NULL) {
  ///   cerr << "Remove data failed, key is null." << endl;
  /// } else if (status == RC_STORE_NOT_FOUND) {
  ///   cerr << "Store not found." << endl;
  /// } else if (status == RC_DATA_NOT_FOUND) {
  ///   cerr << "Data not found." << endl;
  /// } else if (status == RC_SUCCESS) {
  ///   cerr << "Remove data success." << endl;
  /// }
  /// delete key;
  /// @endcode
  ResultCode removeData(const std::string& storeName, const StoreKey<google::protobuf::Message>& key,
      StoreValue<google::protobuf::Message>& value, PartitionStatus* ps = NULL);

  /// @brief  Get the data size of the given store.
  /// @param  storeName  The name of store.
  /// @param  size  Size of the given store.
  /// @return Status code of result.
  /// @code
  /// // Example
  /// uint32_t size;
  /// ResultCode status = ::idgs::util::singleton<DataStore>::getInstance().scanData("Employee", size);
  /// if (status == RC_STORE_NOT_FOUND) {
  ///   cerr << "Store not found." << endl;
  /// } else if (status == RC_SUCCESS) {
  ///   cerr << "Size of store is " << size << "." << endl;
  /// }
  /// @endcode
  ResultCode storeSize(const std::string& storeName, size_t& size);

  /// @brief  Get the data size of the given store.
  /// @param  storeName  The name of store.
  /// @param  partition  The partition where the data accessed.
  /// @param  size  Size of the given store.
  /// @return Status code of result.
  /// @code
  /// // Example
  /// uint32_t size;
  /// ResultCode status = ::idgs::util::singleton<DataStore>::getInstance().scanData("Employee", size);
  /// if (status == RC_STORE_NOT_FOUND) {
  ///   cerr << "Store not found." << endl;
  /// } else if (status == RC_SUCCESS) {
  ///   cerr << "Size of store is " << size << "." << endl;
  /// }
  /// @endcode
  ResultCode storeSize(const std::string& storeName, const uint32_t& partition, size_t& size);

  /// @brief  When parititon changed, rebalnace data of store.
  /// @param  partition  Witch partition changed.
  /// @param  localMemberId  Member id of local.
  /// @param  curMemberId  Member id before changed.
  /// @param  newMemberId  Member id after changed.
  /// @return Status code of result.
  /// @code
  /// // Example
  /// ResultCode status = ::idgs::util::singleton<DataStore>::getInstance().migrateData(0, 1, 2, 1);
  /// if (status != RC_SUCCESS) {
  ///   cerr << "Error in migrateData." << endl;
  /// }
  /// @endcode
  ResultCode migrateData(const uint32_t& partition, const int32_t& localMemberId, const int32_t& curMemberId,
      const int32_t& newMemberId);

  /// @brief  Load data store config file.
  /// @param  configFilePath  Path of data store config file.
  /// @return Status code of result.
  ResultCode loadCfgFile(const std::string & configFilePath);

  /// @brief  Initialize data store.
  /// @return Status code of result.
  ResultCode initializeDataStore();

  /// @brief  Register actor of data store to actor framework.
  /// @return Status code of result.
  ResultCode registerActor();

  /// @brief  Get all name of stores.
  /// @return The name of stores.
  std::vector<std::string> getAllStoreNames();

  /// @brief  Register actor of data store to actor framework.
  /// @return Status code of result.
  ResultCode syncStore(const std::string& storeName, std::shared_ptr<idgs::store::pb::SyncStore>& store);

  /// @brief  Scan data with store name and paritition.
  /// @param  storeName  Name of store.
  /// @param  partition  Parition of store.
  /// @param  dataMap    Returned data.
  /// @return Status code of result.
  ResultCode scanPartitionData(const std::string& storeName, const uint32_t& partition,
      std::shared_ptr<StoreMap>& dataMap);

  /// @brief  Clear all data with the given partition of store in memory.
  /// @param  storeName  Name of store.
  /// @param  partition  Parition of store.
  /// @return Status code of result.
  ResultCode clearData(const std::string& storeName, const uint32_t& partition);

  /// @brief  Whether service is initialized.
  /// @return True of false of service initialized.
  bool isInit();

  StoreConfigWrappers& getStoreConfigWrappers() {
    return storeConfigWrappers;
  }

  std::shared_ptr<Store>& getStore(const std::string& name);

  ResultCode registerStoreListener(StoreListener* listener);

private:
  bool isInited;

  StoreConfigWrappers storeConfigWrappers;
  std::map<std::string, std::shared_ptr<Store>> storeMap;
  tbb::spin_rw_mutex mutex;

};
// class DataStore

}// namespace store
} // namespace idgs
