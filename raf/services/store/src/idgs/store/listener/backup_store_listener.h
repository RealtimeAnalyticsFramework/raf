/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intela�?s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/store/listener/store_listener.h"
#include "idgs/store/datastore_const.h"
#include "idgs/store/store.h"

namespace idgs {
namespace store {

class BackupStoreListener: public StoreListener, public idgs::util::CloneEnabler<BackupStoreListener, StoreListener> {
public:

  BackupStoreListener();
  ~BackupStoreListener();

  const std::string& getName() const override {
    static std::string name = BACKUP_STORE_LISTENER;
    return name;
  }

  const std::string& getDescription() const override {
    static std::string description = "Backup store listener";
    return description;
  }

  idgs::ResultCode init(std::map<std::string, std::string> props) override {
    return RC_SUCCESS;
  }

  ListenerResultCode insert(ListenerContext* ctx) override;
  ListenerResultCode update(ListenerContext* ctx) override;
  ListenerResultCode remove(ListenerContext* ctx) override;

private:
  void addToRedoLog(StorePtr& store, const int32_t& partition, const std::string& opName, const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value);
  idgs::store::pb::StoreResultCode getResultCode(const ResultCode& status);

};

} /* namespace store */
} /* namespace idgs */
