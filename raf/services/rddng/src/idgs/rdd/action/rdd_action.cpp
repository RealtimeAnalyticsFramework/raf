
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "rdd_action.h"

#include "idgs/application.h"

using namespace std;
using namespace idgs::rdd::pb;

namespace idgs {
namespace rdd {
namespace action {

RddAction::RddAction() : state(idgs::rdd::pb::INIT), code(RRC_SUCCESS) {
  uint32_t partitionSize = idgs_application()->getClusterFramework()->getPartitionCount();
  partitionState.resize(partitionSize);
  actionResult.resize(partitionSize);
}

RddAction::~RddAction() {
}

const string& RddAction::getActionId() const {
  return actionId;
}

const string& RddAction::getActionOpName() const {
  return actionOpName;
}

const idgs::actor::ActorMessagePtr& RddAction::getMessage() const {
  return message;
}

const pb::RddResultCode& RddAction::getActionResultCode() const {
  return code;
}

RddState RddAction::getState() const {
  return state;
}

const RddState& RddAction::getPartitionState(const uint32_t& partition) const {
  return partitionState[partition];
}

const vector<vector<string>>& RddAction::getActionResult() const {
  return actionResult;
}

void RddAction::setActionId(const string& actionid) {
  actionId = actionid;
}

void RddAction::setActionOpName(const string& actionOpName) {
  this->actionOpName = actionOpName;
}

void RddAction::setMessage(idgs::actor::ActorMessagePtr message) {
  this->message = message;
}

void RddAction::setActionResultCode(const pb::RddResultCode& actionResultCode) {
  code = actionResultCode;
}

void RddAction::setPartitionState(const uint32_t& partition, const RddState& state) {
  partitionState[partition] = state;
  for (int32_t i = 0; i < partitionState.size(); ++i) {
    if (partitionState[i] != READY) {
      return;
    }
  }

  this->state = READY;
}

void RddAction::addActionResult(const uint32_t& partition, const string& result) {
  actionResult[partition].push_back(result);
}

} // namespace action
} // namespace rdd
} // namespace idgs
