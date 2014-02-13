
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once


#include "protobuf/pbvariant.h"
#include "idgs/actor/actor_message.h"

namespace idgs {
namespace rdd {
class BaseRddPartition;
}

namespace expr {

/// The context of expression.
/// To access the context of expression, e.g. KEY or Value.
class ExpressionContext {
public:
  ExpressionContext();
  ~ExpressionContext();

public:
  /// @brief  set KEY and VALUE of the entry.
  /// @param  key     Key of data.
  /// @param  value   Value of data.
  void setKeyValue(const idgs::actor::PbMessagePtr* key, const idgs::actor::PbMessagePtr* value) {
    this->key = const_cast<idgs::actor::PbMessagePtr*>(key);
    this->value = const_cast<idgs::actor::PbMessagePtr*>(value);
  }

  void setKey(const idgs::actor::PbMessagePtr* key) {
    this->key = const_cast<idgs::actor::PbMessagePtr*>(key);
  }

  void setValue(const idgs::actor::PbMessagePtr* value) {
    this->value = const_cast<idgs::actor::PbMessagePtr*>(value);
  }

  inline idgs::actor::PbMessagePtr* getKey() {
    return key;
  }

  inline idgs::actor::PbMessagePtr* getValue() {
    return value;
  }

  void setVariable(int index, const protobuf::PbVariant& v) {
    if (index >= viarialbes.size()) {
      viarialbes.resize(index + 1);
    }
    viarialbes[index] = v;
  }

  const protobuf::PbVariant& getVariable(int index) {
    if (index >= viarialbes.size()) {
      viarialbes.resize(index + 1);
    }
    return viarialbes[index];
  }

  const idgs::rdd::BaseRddPartition* getInPartition() const {
    return inPartition;
  }
  void setInPartition(idgs::rdd::BaseRddPartition* p) {
    inPartition = p;
  }
  idgs::rdd::BaseRddPartition* getOutPartition() {
    return outPartition;
  }
  void setOutPartition(idgs::rdd::BaseRddPartition* p) {
    outPartition = p;
  }

private:
  idgs::actor::PbMessagePtr* key;
  idgs::actor::PbMessagePtr* value;

  idgs::rdd::BaseRddPartition* inPartition;
  idgs::rdd::BaseRddPartition* outPartition;

  std::vector<protobuf::PbVariant> viarialbes;
};

} // namespace expr
} // namespace idgs 
