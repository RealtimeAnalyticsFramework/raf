/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#pragma once

#include "loader.h"
#include "idgs/actor/stateless_actor.h"
#include "idgs/cancelable_timer.h"

namespace idgs {
namespace client {
class InClusterLoader: public Loader, public idgs::actor::StatelessActor {
public:
  InClusterLoader();
  ~InClusterLoader();
  idgs::ResultCode startMember();
  idgs::ResultCode stopMember();
  idgs::ResultCode runMember();
  idgs::ResultCode init(LoaderSettings* settings);
  void import();
  const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override;

private:
  idgs::actor::ActorMessagePtr genInsertActorMsg(const std::string& store_name, const std::string& line,
      idgs::ResultCode* rc, uint32_t option = 0);
  static ::idgs::actor::ActorDescriptorPtr generateActorDescriptor();
  idgs::ResultCode sendRequest();
  void handleInsertResponse(const idgs::actor::ActorMessagePtr& msg);
  const static std::string ACOTR_ID;
  std::atomic_int activeRequest;
  std::atomic_int doneRecords;
  idgs::cancelable_timer* cancelTimer = NULL;
};
}
}
