
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once


#include "idgs/application.h"

namespace idgs {
namespace store {

class DataStore;

class DataMigraionListener;
class DataSyncListener;
class BackupStoreListener;

class StoreServiceActor;
class ListenerManager;
class StoreSchemaActor;
class MigrationTargetActor;
class MigrationSourceActor;
class SyncTargetActor;
class SyncSourceActor;

struct StoreModule: public idgs::Module {
public:
  StoreModule();
  virtual ~StoreModule();

  virtual int init(const char* config_path, idgs::Application* theApp);
  virtual int start();
  virtual int stop();

public:
  DataStore* getDataStore() {
    return datastore;
  }

private:
  idgs::Application* app;
  DataStore* datastore;

  DataMigraionListener* partChangeListener;
  DataSyncListener* memberJoinedListener;
  BackupStoreListener* backupStoreListener;

  StoreServiceActor* storeActor;
  ListenerManager* listenerManager;
  StoreSchemaActor* schemaActor;
  MigrationTargetActor* migrationTargetActor;
  MigrationSourceActor* migrationSourceActor;
  SyncTargetActor* syncTargetActor;
  SyncSourceActor* syncSourceActor;

};

/// entry point of this module
StoreModule* idgs_store_module(void);
} // namespace store
} // namespace idgs


