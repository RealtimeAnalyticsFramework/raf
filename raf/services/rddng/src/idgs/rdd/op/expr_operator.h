
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/rdd/rdd_local.h"

namespace idgs {
namespace rdd {
namespace op {

enum ReUseType {
  REUSE_NONE = 0,
  REUSE_KEY = 1,
  REUSE_VALUE = 2
};

class OutMessageInfo {
public:

  OutMessageInfo();
  virtual ~OutMessageInfo();

  void setTemplateMessage(const google::protobuf::Message* templateMessage);
  void setReuse(const ReUseType& reuse);
  const ReUseType& getReuse() const;
  bool addField(const std::string& fieldName, idgs::expr::Expression* srcExpr);
  void fillMessage(idgs::actor::PbMessagePtr& output, idgs::expr::ExpressionContext* ctx) const;
  const std::vector<const google::protobuf::FieldDescriptor*>& getFields() const;
  const std::vector<idgs::expr::Expression*>& getExpressions() const;
  void destroy();

private:
  const google::protobuf::Message* templateMessage;
  const google::protobuf::Reflection* reflection;
  std::vector<const google::protobuf::FieldDescriptor*> outFields;
  std::vector<idgs::expr::Expression*> srcExprs;
  ReUseType reuse;
};

struct OutMessagePair {
  OutMessageInfo key;
  OutMessageInfo value;
};

class ExprMapOperator: public RddOperator, public idgs::util::CloneEnabler<ExprMapOperator, RddOperator> {
public:
  ExprMapOperator();
  ~ExprMapOperator();
public:
  RddOperator* clone() const {
    return new ExprMapOperator;
  }

  virtual const std::string& getName() const override {
    static std::string name = "EXPR_MAP_OPERATOR";
    return name;
  }

  virtual bool parse(const pb::InRddInfo& inRddInfo, const pb::OutRddInfo& outRddInfo,
      RddLocal* inRddLocal, RddLocal* outRddLocal) override;

  virtual bool evaluate(idgs::expr::ExpressionContext* ctx) override;

public:
  OutMessagePair outMsg;
  idgs::expr::Expression* filterExpr = NULL;

private:
};

} // namespace op
} // namespace rdd
} // namespace idgs

