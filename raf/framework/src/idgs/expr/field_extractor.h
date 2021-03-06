
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once



#include "idgs/expr/expression.h"


namespace idgs {
namespace expr {

/// The target of extract.
enum FieldExtractTarget {
  T_KEY = 1, T_VALUE = 2
};

/// A struct of field witch is parsed from given protobuf message.
struct ParsedField {
  /// field path and name
  std::string path;
  /// message descriptor
  google::protobuf::Descriptor const* descriptor;
  // field descritptor
  google::protobuf::FieldDescriptor const* fieldDescriptor;
  /// message reflection
  google::protobuf::Reflection const* reflection;
};

class FieldExtractor: public NullaryExpression, public idgs::util::CloneEnabler<FieldExtractor, Expression> {
public:

  /// @brief Constructor
  FieldExtractor();

  /// @brief Destructor
  virtual ~FieldExtractor();

  const google::protobuf::FieldDescriptor::Type& getFieldType() const;

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const;

  virtual bool parse(const idgs::pb::Expr& entryExp, const idgs::actor::PbMessagePtr& key,
      const idgs::actor::PbMessagePtr& value);

  virtual const std::string& name() {
    static std::string name("FIELD");
    return name;
  }

private:
  std::string path;
  /// @todo change to google::protobuf::FieldDescriptor::CppType
  google::protobuf::FieldDescriptor::Type fieldType;
  std::vector<ParsedField> parsedFields;
  FieldExtractTarget target;

};

} // namespace expr
} // namespace idgs 
