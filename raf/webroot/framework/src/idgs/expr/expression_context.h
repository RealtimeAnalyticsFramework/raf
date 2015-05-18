
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "protobuf/pbvariant.h"
#include "idgs/actor/actor_message.h"

namespace idgs {
namespace expr {

/// The context of expression.
/// To access the context of expression, e.g. KEY or Value.
class ExpressionContext {
private:

public:
  ExpressionContext();
  ~ExpressionContext();
  friend ExpressionContext& getTlExpressionContext();

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

  void setOutputKeyValue(idgs::actor::PbMessagePtr* key, idgs::actor::PbMessagePtr* value) {
    outkey = key;
    outvalue = value;
  }

  inline idgs::actor::PbMessagePtr* getOutputKey() {
    return outkey;
  }

  inline idgs::actor::PbMessagePtr* getOutputValue() {
    return outvalue;
  }

  void setVariable(int index, const protobuf::PbVariant& v) {
    if (!viarialbes) {
      viarialbes = new std::vector<protobuf::PbVariant>;
    }
    if (index >= viarialbes->size()) {
      viarialbes->resize(index + 1);
    }
    (*viarialbes)[index] = v;
  }

  const protobuf::PbVariant& getVariable(int index) {
    if (!viarialbes) {
      viarialbes = new std::vector<protobuf::PbVariant>;
    }
    if (index >= viarialbes->size()) {
      viarialbes->resize(index + 1);
    }
    return (*viarialbes)[index];
  }

  void reset() {
    key = NULL;
    value = NULL;
    outkey = NULL;
    outvalue = NULL;
    if (viarialbes) {
      viarialbes->clear();
    }
  }

private:
  idgs::actor::PbMessagePtr* key;
  idgs::actor::PbMessagePtr* value;

  idgs::actor::PbMessagePtr* outkey;
  idgs::actor::PbMessagePtr* outvalue;

  std::vector<protobuf::PbVariant> *viarialbes;
};

///
/// get expression context in thread local storage
///
ExpressionContext& getTlExpressionContext();

} // namespace expr
} // namespace idgs 
