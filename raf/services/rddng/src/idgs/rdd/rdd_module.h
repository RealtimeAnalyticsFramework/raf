
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/idgs_module.h"
#include "idgs/rdd/action/action.h"
#include "idgs/rdd/transform/transformer.h"
#include "idgs/rdd/rdd_member_event_listener.h"

namespace idgs {
namespace rdd {
class RddServiceActor;
class RddInternalServiceActor;

struct RddModule: public idgs::Module {
public:
  RddModule();
  virtual ~RddModule();

  virtual int init(const char* config_path, idgs::Application* theApp) override;
  virtual int start() override;
  virtual int stop() override;
public:
  ActionMgr* getActionManager() {
    return actionManager;
  }

  TransformerMgr* getTransformManager() {
    return transformManager;
  }

  RddOperatorMgr* getRddOperatorManager() {
    return operatorManager;
  }

  protobuf::MessageHelper* getMessageHelper() {
    return messageHelper;
  }

  void resetMessageHelper() {
    if (messageHelper) {
      delete messageHelper;
    }
    messageHelper = new protobuf::MessageHelper;
  }

public:
  idgs::Application* app = NULL;
  ActionMgr *actionManager = NULL;
  TransformerMgr *transformManager = NULL;
  RddOperatorMgr *operatorManager = NULL;

  RddServiceActor*         serviceActor = NULL;
  RddInternalServiceActor* internalServiceActor = NULL;

  RddMemberEventListener* memberEventListener = NULL;

  protobuf::MessageHelper* messageHelper = NULL;
};

RddModule* idgs_rdd_module(void);


} // namespace rdd
} // namespace idgs
