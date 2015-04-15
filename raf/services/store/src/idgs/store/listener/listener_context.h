/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/

#pragma once

#include "idgs/actor/actor_message.h"
#include "idgs/store/pb/store_service.pb.h"

namespace idgs {
namespace store {

class ListenerContext {
public:
  ListenerContext();
  virtual ~ListenerContext();

  inline const idgs::actor::ActorMessagePtr& getMessage() const {
    return * msg;
  }

  void setMessage(const idgs::actor::ActorMessagePtr* message) {
    msg = message;
  }

  void setKey(const idgs::actor::PbMessagePtr* key) {
    this->key = const_cast<idgs::actor::PbMessagePtr*>(key);
  }

  void setValue(const idgs::actor::PbMessagePtr* value) {
    this->value = const_cast<idgs::actor::PbMessagePtr*>(value);
  }

  void setKeyValue(const idgs::actor::PbMessagePtr* key, const idgs::actor::PbMessagePtr* value) {
    setKey(key);
    setValue(value);
  }

  inline const idgs::actor::PbMessagePtr* getKey() const {
    return key;
  }

  inline const idgs::actor::PbMessagePtr* getValue() const {
    return value;
  }

  void setRawValue(const idgs::actor::PbMessagePtr* value) {
    this->rawValue = const_cast<idgs::actor::PbMessagePtr*>(value);
  }

  inline const idgs::actor::PbMessagePtr* getRawValue() const {
    return rawValue;
  }

  void setLastValue(const idgs::actor::PbMessagePtr* lastValue) {
    this->lastValue = const_cast<idgs::actor::PbMessagePtr*>(lastValue);
  }

  inline idgs::actor::PbMessagePtr* getLastValue() {
    return lastValue;
  }

  void setVersionValue(const idgs::actor::PbMessagePtr* lastValue) {
    this->versionValue = const_cast<idgs::actor::PbMessagePtr*>(lastValue);
  }

  inline idgs::actor::PbMessagePtr* getVersionValue() {
    return versionValue;
  }

  inline const int32_t& getPartitionId() const {
    return partitionId;
  }

  void setPartitionId(const uint32_t& partitionid) {
    partitionId = partitionid;
  }

  inline const int32_t& getPrmaryMemberId() const {
    return priMemberId;
  }

  void setPrmaryMemberId(const uint32_t& primaryMemberId) {
    priMemberId = primaryMemberId;
  }

  inline const int32_t& getListenerIndex() const {
    return listenerIndex;
  }

  void setListenerIndex(const int32_t& index) {
    listenerIndex = index;
  }

  inline const uint32_t& getOptions() const {
    return options;
  }

  void setOptions(const uint32_t& storeOptions) {
    options = storeOptions;
  }

  inline const uint32_t& getVersion() const {
    return version;
  }

  void setVersion(const int32_t& dataVersion) {
    version = dataVersion;
  }

  inline const pb::StoreResultCode& getResultCode() const {
    return code;
  }

  void setResultCode(const pb::StoreResultCode& resultCode) {
    code = resultCode;
  }

private:
  const idgs::actor::ActorMessagePtr* msg;

  idgs::actor::PbMessagePtr* key;
  idgs::actor::PbMessagePtr* value;
  idgs::actor::PbMessagePtr* rawValue;
  idgs::actor::PbMessagePtr* lastValue;
  idgs::actor::PbMessagePtr* versionValue;

  int32_t partitionId;
  int32_t priMemberId;
  int32_t listenerIndex;
  uint32_t options;
  uint32_t version;

  pb::StoreResultCode code;

};

} /* namespace store */
} /* namespace idgs */
