
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/actor/rpc_framework.h"

#include "idgs/store/datastore_const.h"
#include "idgs/store/store.h"

namespace idgs {
namespace store {

/// Stateless actor of data store
/// Data store actor will register to actor framework.
class StoreServiceActor: public idgs::actor::StatelessActor {

public:

  /// @brief  Construction.
  /// @param  actorId Actor id
  StoreServiceActor(const std::string& actorId);

  /// @brief  Destruction.
  virtual ~StoreServiceActor();

  /// @brief  Generate descriptor for DataStoreStatelessActor.
  /// @return The descriptor for DataStoreStatelessActor
  static idgs::actor::ActorDescriptorPtr generateActorDescriptor();

  const std::string& getActorName() const override {
    static std::string actorName = ACTORID_STORE_SERVCIE;
    return actorName;
  }

  const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override;

private:
  void handleInsertStore(const idgs::actor::ActorMessagePtr& msg);
  void handleLocalInsert(const idgs::actor::ActorMessagePtr& msg);
  void handleGetStore(const idgs::actor::ActorMessagePtr& msg);
  void handleLocalGet(const idgs::actor::ActorMessagePtr& msg);
  void handleUpdateStore(const idgs::actor::ActorMessagePtr& msg);
  void handleLocalUpdate(const idgs::actor::ActorMessagePtr& msg);
  void handleDeleteStore(const idgs::actor::ActorMessagePtr& msg);
  void handleLocalDelete(const idgs::actor::ActorMessagePtr& msg);
  void handleLocalCount(const idgs::actor::ActorMessagePtr& msg);
  void handleCountStore(const idgs::actor::ActorMessagePtr& msg);
  void handleTruncateStore(const idgs::actor::ActorMessagePtr& msg);
  void handleLocalTruncate(const idgs::actor::ActorMessagePtr& msg);

  void sendResponse(const idgs::actor::ActorMessagePtr& msg, const std::string& opName,
      const idgs::actor::PbMessagePtr& payload, const idgs::actor::PbMessagePtr& returnValue = idgs::actor::PbMessagePtr());

  void addToRedoLog(StorePtr& store, const int32_t& partition, const std::string& opName, const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value);
  void processReplicatedStore(const idgs::actor::ActorMessagePtr& msg);
  idgs::store::pb::StoreResultCode getResultCode(const ResultCode& status);

  void sendStoreListener(const idgs::actor::ActorMessagePtr& msg, pb::StoreResultCode code = pb::SRC_SUCCESS);

  template<typename Request>
  ResultCode calcStorePartition(Request* request, const idgs::actor::PbMessagePtr& key, StoreConfigWrapper* storeConfig) {
    if (request->has_partition_id() && request->partition_id() != -1) {
      return RC_SUCCESS;
    } else {
      PartitionInfo ps;
      ResultCode rc = storeConfig->calculatePartitionInfo(key, &ps);
      request->set_partition_id(ps.partitionId);
      return rc;
    }
  }
};

} // namespace store
} // namespace idgs
