/*
Copyright (c) <2012>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/

#include <gtest/gtest.h>
#include <thread>
#include "idgs/idgslogging.h"


template <typename T>
class ThreadLocal {
public:
  T value = 0;
};

#define LOOP 10


__thread int tl_value = 0;
__thread std::string* tl_string = NULL;

std::string& getTlString() {
  if (!tl_string) {
    tl_string = new std::string();
  }
  return *tl_string;
}

TEST(threadlocal, test) {

  int i = 0;
  std::thread t1([i](){
    for(int j = 0; j < LOOP; ++j) {
      getTlString() = "thread 1";
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      LOG(INFO) << "Thread: " << i << ", value: " << tl_value++ << ", string: " << getTlString();
    }
  });

  ++i;
  std::thread t2([i](){
    for(int j = 0; j < LOOP; ++j) {
      getTlString() = "thread 2";
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      LOG(INFO) << "Thread: " << i << ", value: " << tl_value++ << ", string: " << getTlString();
    }
  });

  ++i;
  std::thread t3([i](){
    for(int j = 0; j < LOOP; ++j) {
      getTlString() = "thread 3";
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      LOG(INFO) << "Thread: " << i << ", value: " << tl_value++ << ", string: " << getTlString();
    }
  });

  t1.join();
  t2.join();
  t3.join();

}


