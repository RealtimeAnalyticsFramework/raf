
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include <gtest/gtest.h>
#include <stdexcept>
#include "idgs/idgslogging.h"

#define __noinline __attribute__((noinline))

namespace bt{
void f1();
void f2() ;
void f3() ;
void foo(int i);
void bar(int i) ;
void baz(int i) ;
}

TEST(backtrace, case1) {
  bt::baz(1);
}

TEST(backtrace, case2) {
  bt::baz(2);
}
TEST(backtrace, case3) {
  bt::baz(3);
}
TEST(backtrace, case4) {
  LOG(INFO) << idgs::util::stacktrace();
}
TEST(backtrace, case5) {
  LOG(INFO) << idgs::util::stacktrace(0);
}
//TEST(backtrace, case6) {
//  LOG(FATAL) << "hello";
//}

TEST(backtrace, case7) {
  LOG(INFO) << typeid(*this).name();
  LOG(INFO) << idgs::util::demangle(typeid(*this).name());
}

namespace bt{
__noinline void baz(int i)  {
  bar(i);
}
__noinline void bar(int i)  {
  foo(i);
}
__noinline void foo(int i) {
  switch(i) {
  case 1:
    f1();
    break;
  case 2:
    f2();
    break;
  case 3:
    f3();
    break;
  default:
    break;
  }
}

__noinline void f1() {
  try {
    throw int(42);
  } catch (int i) {
    LOG(INFO) << "Int Exception: " << i;
  } catch (std::exception& e) {
    LOG(INFO) << "Exception: " << e.what();
  } catch (...) {
    catchUnknownException();
  }
}

__noinline void f2() {
  try {
    throw std::runtime_error("runtime");
  } catch (std::exception& e) {
    LOG(INFO) << "Exception: " << e.what();
  } catch (...) {
    catchUnknownException();
  }
}

__noinline void f3() {
  try {
    throw std::runtime_error("runtime");
  } catch (...) {
    catchUnknownException();
  }
}

}

