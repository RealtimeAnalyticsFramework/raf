/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/actor/stateless_actor.h"
#include "idgs/store/data_store.h"
#include "idgs/store/datastore_const.h"
#include "idgs/store/listener/store_listener.h"

namespace idgs {
namespace store {

class ListenerManager: public idgs::actor::StatelessActor {
public:

  /// @brief  Construction.
  /// @param  actorId Actor id
  ListenerManager(const std::string& actorId);

  /// @brief  Destruction.
  virtual ~ListenerManager();

  /// @brief  Generate descriptor for DataStoreStatelessActor.
  /// @return The descriptor for DataStoreStatelessActor
  static idgs::actor::ActorDescriptorPtr generateActorDescriptor();

  const std::string& getActorName() const override {
    static std::string actorName = LISTENER_MANAGER;
    return actorName;
  }

  const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override;

  void setDataStore(DataStore* dataStore);

private:
  DataStore* datastore;

private:

  void handleListenerInsert(const idgs::actor::ActorMessagePtr& msg);
  void handleListenerUpdate(const idgs::actor::ActorMessagePtr& msg);
  void handleListenerGet(const idgs::actor::ActorMessagePtr& msg);
  void handleListenerDelete(const idgs::actor::ActorMessagePtr& msg);
  void handleListenerTruncate(const idgs::actor::ActorMessagePtr& msg);

  void sendResponse(const idgs::actor::ActorMessagePtr& msg, const std::string& opName, const idgs::actor::PbMessagePtr& payload, ListenerContext& ctx);

  pb::StoreResultCode parseKeyValue(const idgs::actor::ActorMessagePtr& msg, const StoreConfigWrapperPtr& config,
      idgs::actor::PbMessagePtr& key, idgs::actor::PbMessagePtr& value, idgs::actor::PbMessagePtr& rawValue);

  template<typename Request, typename Response>
  void handleListener(const idgs::actor::ActorMessagePtr& msg, const std::string& responseOpName) {
    ListenerContext ctx;
    Request* request = dynamic_cast<Request*>(msg->getPayload().get());
    std::shared_ptr<Response> response = std::make_shared<Response>();

    const std::string& storeName = request->store_name();
    auto store = datastore->getStore(storeName);
    if (!store) {
      LOG(ERROR) << "Store " << storeName << " is not found.";
      ctx.setResultCode(pb::SRC_STORE_NOT_FOUND);
      response->set_result_code(pb::SRC_STORE_NOT_FOUND);
      sendResponse(msg, responseOpName, response, ctx);
      return;
    }

    pb::StoreListenerInfo listenerInfo;
    if (!msg->parseAttachment(STORE_ATTACH_LISTENER, &listenerInfo)) {
      LOG(ERROR) << "Failed to parse store listener info.";
      ctx.setResultCode(pb::SRC_INVALID_LISTENER_INFO);
      response->set_result_code(pb::SRC_INVALID_LISTENER_INFO);
      sendResponse(msg, responseOpName, response, ctx);
      return;
    }

    std::shared_ptr<StoreConfig> config = store->getStoreConfig();
    idgs::actor::PbMessagePtr key, value, rawValue;
    auto src = parseKeyValue(msg, config, key, value, rawValue);
    if (src != pb::SRC_SUCCESS) {
      ctx.setResultCode(src);
      response->set_result_code(src);
      sendResponse(msg, responseOpName, response, ctx);
      return;
    }

    ctx.setListenerIndex(listenerInfo.listener_index());
    ctx.setResultCode(listenerInfo.result_code());
    ctx.setMessage(&msg);
    ctx.setKeyValue(&key, &value);
    ctx.setRawValue(&rawValue);
    ctx.setPrmaryMemberId(listenerInfo.primary_member_id());

    if (request->has_partition_id() && request->partition_id() != -1) {
      ctx.setPartitionId(request->partition_id());
    } else {
      StoreOption ps;
      config->calculatePartitionInfo(* ctx.getKey(), &ps);
      ctx.setPartitionId(ps.partitionId);
    }

    if (request->has_options()) {
      ctx.setOptions(request->options());
    }

    if (request->has_version()) {
      ctx.setVersion(request->version());
    }

    auto& listeners = config->getStoreListener();
    int32_t index = listenerInfo.has_listener_index() ? listenerInfo.listener_index() : 0;
    std::string opName = msg->getOperationName();
    for (int32_t i = index; i < listeners.size(); ++ i) {
      ListenerResultCode code = LRC_CONTINUE;
      if (opName == OP_INTERNAL_INSERT) {
        code = listeners[i]->insert(&ctx);
      } else if (opName == OP_INTERNAL_UPDATE) {
        code = listeners[i]->update(&ctx);
      } else if (opName == OP_INTERNAL_GET) {
        code = listeners[i]->get(&ctx);
      } else if (opName == OP_INTERNAL_DELETE) {
        code = listeners[i]->remove(&ctx);
      } else if (opName == OP_INTERNAL_TRUNCATE) {
        code = listeners[i]->truncate(&ctx);
      } else {
        LOG(ERROR) << "store listener is not support operator " << opName;
        continue;
      }

      switch (code) {
        case LRC_CONTINUE:
          continue;
        case LRC_BREAK:
          return;
        case LRC_END:
          break;
        case LRC_ERROR:
          LOG(ERROR) << "error when execute listener " << listeners[i]->getName() << ", index " << i;
          break;
      }
    }

    response->set_result_code(ctx.getResultCode());
    sendResponse(msg, responseOpName, response, ctx);
  }

};

} /* namespace store */
} /* namespace idgs */
