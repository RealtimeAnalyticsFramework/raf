
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once


#include "idgs/rdd/op/expr_operator.h"
#include "idgs/rdd/pb/rdd_transform.pb.h"

namespace idgs {
namespace rdd {
namespace op {

class JoinOperator: public RddOperator, public idgs::util::CloneEnabler<JoinOperator, RddOperator> {
public:
  JoinOperator();
  ~JoinOperator();

public:
  JoinOperator* clone() const {
    return new JoinOperator;
  }

  virtual const std::string& getName() const override {
    static std::string name = "JOIN_OPERATOR";
    return name;
  }

  virtual bool parse(const pb::InRddInfo& inRddInfo, const pb::OutRddInfo& outRddInfo,
      RddLocal* inRddLocal, RddLocal* outRddLocal) override;

  virtual bool evaluate(idgs::expr::ExpressionContext* ctx) override;

public:
  ExprMapOperator mapOperator;
  pb::JoinType joinType;

};

} // namespace op
} // namespace rdd
} // namespace idgs

