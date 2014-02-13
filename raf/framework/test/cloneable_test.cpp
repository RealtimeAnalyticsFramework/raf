
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__)
#include "idgs_gch.h"
#endif // GNUC_ $
#include <gtest/gtest.h>
#include "idgs/idgslogging.h"
#include "idgs/util/cloneable.h"

namespace {
class CloneTest: public virtual idgs::util::Cloneable<CloneTest>{
public:
  virtual void uname() const {
    LOG(INFO) << "CloneTest";
  }
};

class CloneTest1 : public CloneTest, public idgs::util::CloneEnabler<CloneTest1, CloneTest> {
public:
  virtual void uname() const {
    LOG(INFO) << "CloneTest1";
  }

};

class CloneTest2 : public CloneTest, public idgs::util::CloneEnabler<CloneTest2, CloneTest> {
  virtual void uname() const override {
    LOG(INFO) << "CloneTest2";
  }
};
}

TEST(singleton, cloneable) {
  CloneTest* p;

  CloneTest1 o1;
  p = o1.clone();
  p->uname();
  delete p;

  CloneTest2 o2;
  p = o2.clone();
  p->uname();
  delete p;
}
