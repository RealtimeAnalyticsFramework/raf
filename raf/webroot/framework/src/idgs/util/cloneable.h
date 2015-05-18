
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

namespace idgs {
namespace util {

template<typename BASE>
class Cloneable {
public:
  virtual ~Cloneable() {
  }

  virtual BASE* clone() const = 0;
};

template<typename TYPE, typename BASE = TYPE>
class CloneEnabler: public virtual Cloneable<BASE> {
public:
  virtual ~CloneEnabler() {
  }

  virtual BASE* clone() const override {
    return new TYPE();
  }
};

template<typename TYPE, typename BASE = TYPE>
class CopyConstructEnabler: public virtual Cloneable<BASE> {
public:
  virtual ~CopyConstructEnabler() {
  }

  virtual BASE* clone() const override {
    return new TYPE(reinterpret_cast<const TYPE&>(*this));
  }
};

template<typename TYPE, typename BASE = TYPE>
class SingletonEnabler: public virtual Cloneable<BASE> {
public:
  virtual ~SingletonEnabler() {
  }

  virtual BASE* clone() const override {
    return this;
  }
};

} // namespace util
} // namespace idgs
