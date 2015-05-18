
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include <gtest/gtest.h>
#include "idgs/util/singleton.h"
#include "idgs/idgslogging.h"

using namespace std;

namespace {
  struct SingletonTest {
    int value;
  };

  struct Interface {
    virtual void sayHello() = 0;
  };

  struct Implementation : public Interface {
    virtual void sayHello() {
      LOG(INFO) << "hello world. " << endl;
    }
  };

};


TEST(singleton, singleton) {
  ::idgs::util::singleton<SingletonTest>::getInstance().value = 1;
  ASSERT_EQ(::idgs::util::singleton<SingletonTest>::getInstance().value, 1);
}

TEST(singleton, pointer) {
  ::idgs::util::singleton<Interface*>::getInstance() = new Implementation();
  ::idgs::util::singleton<Interface*>::getInstance()->sayHello();
}
