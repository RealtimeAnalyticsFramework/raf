
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/idgslogging.h"

namespace idgs {
  class Application;

  struct Module {
    virtual ~Module() {
      function_footprint();
    }

    virtual int init(const char* config_path, idgs::Application* theApp) = 0;
    virtual int start() = 0;
    virtual int stop() = 0;
  };
}

#if defined(__cplusplus)
extern "C" {
#endif // defined(__cplusplus)

  typedef idgs::Module* (* fn_get_idgs_module)(void);
  typedef void (* fn_release_idgs_module)(idgs::Module*);

  ///
  /// all modules must implement this function
  /// get pointer to module
  ///
  idgs::Module*  get_idgs_module(void);

  ///
  /// all modules must implement this function
  /// release module
  ///
  void  release_idgs_module(idgs::Module* mod);

#if defined(__cplusplus)
}
#endif // defined(__cplusplus)
