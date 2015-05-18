
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $
#include "expression_context.h"


namespace idgs {
namespace expr {

ExpressionContext::ExpressionContext():
  key(NULL), value(NULL), outkey(NULL), outvalue(NULL), viarialbes(NULL) {
}

ExpressionContext::~ExpressionContext() {
  if (viarialbes) {
    delete viarialbes;
    viarialbes = NULL;
  }
}


__thread ExpressionContext* threadlocal_expression_context = NULL;

///
/// get expression context in thread local storage
///
ExpressionContext& getTlExpressionContext() {
  if (!threadlocal_expression_context) {
    threadlocal_expression_context = new ExpressionContext();
  }
  return *threadlocal_expression_context;
}


} // namespace expr
} // namespace idgs 
