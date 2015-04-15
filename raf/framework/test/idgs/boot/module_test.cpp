
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include <gtest/gtest.h>
#include <dlfcn.h>

#include "idgs/actor/rpc_framework.h"




TEST(module, load) {
  LOG(INFO)<< "dlopen module store";
  void* handle  = dlopen("dist/lib/libidgsdatastore.so", RTLD_NOW | RTLD_GLOBAL);
  ASSERT_NE((void*)NULL, handle);

//  LOG(INFO)<< "dlsym module store";
//  fn_idgs_module module = (fn_idgs_module)dlsym(handle, "idgs_module");
//  ASSERT_NE((void*)NULL, (void*)module);
//
//  LOG(INFO)<< "module store: get callback functions";
//  idgs_module_fn_t* mod_fn = (*module)();
//  ASSERT_NE((void*)NULL, (void*)mod_fn);
//
//  mod_fn->init("here is the config file path", NULL);
//  mod_fn->start();
//  mod_fn->stop();

  LOG(INFO)<< "module store: close";
  google::protobuf::ShutdownProtobufLibrary();
  ASSERT_EQ(0, dlclose(handle));

}
