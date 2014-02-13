
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once
#include "idgs/idgs_module.h"


namespace idgs {
namespace admin {
/// @fixme all module level singleton should be an attribute of the class below.

struct AdminModule : public idgs::Module {
  virtual ~AdminModule();

  virtual int init(const char* config_path, idgs::Application* theApp) override;
  virtual int start() override;
  virtual int stop() override;
  idgs::Application* app;
};

AdminModule*  idgs_admin_module(void);
} // namespace admin
} // namespace idgs
