/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/actor/actor_message.h"
#include "idgs/store/store_config_wrapper.h"

namespace idgs {
namespace store {

enum ListenerResultCode {
  LRC_CONTINUE,
  LRC_BREAK,
  LRC_END,
  LRC_ERROR
};

/// Listener to store, possible usage: persist, sync to backup members, secondary index ...
/// extends sample class Listener : public StoreListener, public idgs::util::CloneEnabler<Listener, StoreListener>
/// must register listener by singleton<idgs::store::DataStore>::getInstance().registerStoreListener() before store module initialize
class StoreListener: public virtual idgs::util::Cloneable<StoreListener> {
public:

  StoreListener();
  virtual ~StoreListener();

  /// used by factory class.
  virtual const std::string& getName() const = 0;

  /// the description of current listener
  virtual const std::string& getDescription() const = 0;

  virtual idgs::ResultCode init(std::map<std::string, std::string> props) = 0;

  void setStoreConfig(std::shared_ptr<StoreConfigWrapper> config) {
    storeConfig = config;
  }

  void setListenerIndex(const uint32_t& index) {
    listenerIndex = index;
  }

  /// @brief  send message to next listener manager
  /// @param  msg  actor message with OpName : OP_INSERT, payload[InsertRequest], attachment{(STORE_ATTACH_LISTENER, StoreListenerRequest), (key, PbMessagePtr), (value, PbMessagePtr)}
  /// @return result code of listener
  virtual ListenerResultCode insert(const idgs::actor::ActorMessagePtr& msg) {
    return LRC_CONTINUE;
  }

  /// @brief  send message to next listener manager
  /// @param  msg  actor message with OpName : OP_GET, payload[GetRequest], attachment{(STORE_ATTACH_LISTENER, StoreListenerRequest), (key, PbMessagePtr), (value, PbMessagePtr)}
  /// @return result code of listener
  virtual ListenerResultCode get(const idgs::actor::ActorMessagePtr& msg) {
    return LRC_CONTINUE;
  }

  /// @brief  send message to next listener manager
  /// @param  msg  actor message with OpName : OP_Update, payload[UpdateRequest], attachment{(STORE_ATTACH_LISTENER, StoreListenerRequest), (key, PbMessagePtr), (value, PbMessagePtr)}
  /// @return result code of listener
  virtual ListenerResultCode update(const idgs::actor::ActorMessagePtr& msg) {
    return LRC_CONTINUE;
  }

  /// @brief  send message to next listener manager
  /// @param  msg  actor message with OpName : OP_DELETE, payload[RemoveRequest], attachment{(STORE_ATTACH_LISTENER, StoreListenerRequest), (key, PbMessagePtr), (value, PbMessagePtr)}
  /// @return result code of listener
  virtual ListenerResultCode remove(const idgs::actor::ActorMessagePtr& msg) {
    return LRC_CONTINUE;
  }

  /// @brief  send message to next listener manager
  /// @param  msg  actor message with OpName : OP_TRUNCATE, payload[DataClearRequest], attachment{(STORE_ATTACH_LISTENER, StoreListenerRequest)}
  /// @return result code of listener
  virtual ListenerResultCode truncate(const idgs::actor::ActorMessagePtr& msg) {
    return LRC_CONTINUE;
  }

protected:
  /// @brief  send message to next listener manager when LRC_CONTINUE
  /// @param  msg            actor message from listener manager
  /// @param  destMemberId   member id of next listener
  void sendNextListener(const idgs::actor::ActorMessagePtr& msg, const uint32_t& destMemberId);

  /// @brief  send message to next listener manager when LRC_BREAK
  /// @param  msg            actor message from listener manager
  /// @param  destMemberId   member id of next listener
  void sendBreakBranch(const idgs::actor::ActorMessagePtr& msg, const uint32_t& destMemberId);

  /// @brief  send message to next listener manager
  /// @param  msg    actor message from listener manager
  /// @param  key    to get key of data
  /// @param  value  to get value of data
  /// @return whether success
  bool getAttachmentData(const idgs::actor::ActorMessagePtr& msg, idgs::actor::PbMessagePtr& key, idgs::actor::PbMessagePtr& value);

  const std::shared_ptr<idgs::store::StoreConfigWrapper>& getStoreConfig() const {
    return storeConfig;
  }

private:
  std::shared_ptr<idgs::store::StoreConfigWrapper> storeConfig;
  uint32_t listenerIndex;
};

} /* namespace store */
} /* namespace idgs */
