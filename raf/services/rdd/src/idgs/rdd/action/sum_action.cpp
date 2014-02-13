
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $

#include "sum_action.h"
#include "idgs/util/utillity.h"
#include "idgs/rdd/rdd_const.h"
#include "idgs/expr/expression_factory.h"
#include "idgs/rdd/pb/rdd_action.pb.h"

using namespace std;
using namespace protobuf;
using namespace idgs::actor;
using namespace idgs::expr;
using namespace idgs::rdd::pb;

namespace idgs {
namespace rdd {
namespace action {

SumAction::SumAction() {

}

SumAction::~SumAction() {

}

RddResultCode SumAction::action(const idgs::actor::ActorMessagePtr& msg, const BaseRddPartition* input,
    std::vector<PbVariant>& output) {
  ActionRequest* payload = dynamic_cast<ActionRequest*>(msg->getPayload().get());
  if (!payload->has_expression()) {
    LOG(ERROR)<< "RDD \"" << input->getRddName() << " partition[" << input->getPartition() << "]" << "sum action error, lack sum's field";
    return RRC_INVALID_ACTION_PARAM;
  }
  Expression* field_expr = NULL;
  auto rc = ExpressionFactory::build(&field_expr, payload->expression(), input->getKeyTemplate(),
      input->getValueTemplate());
  if (idgs::RC_SUCCESS != rc) {
    LOG(ERROR)<< "RDD \"" << input->getRddName() << " partition[" << input->getPartition() << "]" << "parse sum's field expression error, caused by " << idgs::getErrorDescription(rc);
    return RRC_NOT_SUPPORT;
  }
  double sum = 0;
  if (payload->has_filter()) { /// with filter
    Expression* filter_expr = NULL;
    rc = ExpressionFactory::build(&filter_expr, payload->filter(), input->getKeyTemplate(), input->getValueTemplate());
    if (idgs::RC_SUCCESS != rc) {
      LOG(ERROR)<< "RDD \"" << input->getRddName() << " partition[" << input->getPartition() << "]" << "parse filter expression error, caused by " << idgs::getErrorDescription(rc);
      return RRC_NOT_SUPPORT;
    }
    ExpressionContext ctx;
    input->foreach([filter_expr, field_expr, &sum, &ctx] (const PbMessagePtr& key, const PbMessagePtr& value) {
      ctx.setKeyValue(&key, &value);
      bool flag = (bool) filter_expr->evaluate(&ctx); /// filter expression's value is true or false
        if(flag) { /// meet filter condition
          PbVariant field_value = field_expr->evaluate(&ctx);
          sum += (double) field_value;
        }
      });
    output.push_back(sum);
  } /// end with filter
  else { /// without filter
    ExpressionContext ctx;
    input->foreach([field_expr, &sum, &ctx] (const PbMessagePtr& key, const PbMessagePtr& value) {
      ctx.setKeyValue(&key, &value);
      PbVariant field_value = field_expr->evaluate(&ctx);
      sum += (double) field_value;
    });
    output.push_back(sum);
  } /// end without filter
  return RRC_SUCCESS;
}

RddResultCode SumAction::aggregate(const idgs::actor::ActorMessagePtr& actionRequest, idgs::actor::ActorMessagePtr& actionResponse, const vector<vector<string>>& input) {
  double sum = 0;
  for (size_t partition = 0; partition < input.size(); ++partition) {
    double pSize = 0;
    ResultCode code = sys::convert<double>(input[partition][0], pSize);
    if (code != RC_SUCCESS) {
      LOG(ERROR)<< "cannot convert \"" << input[partition][0] << "\" to a number.";
      return RRC_DATA_CONVERT_ERROR;
    }

    sum += pSize;
  }

  shared_ptr<SumActionResult> response(new SumActionResult);
  response->set_total(sum);

  actionResponse->setAttachment(ACTION_RESULT, response);

  return RRC_SUCCESS;
}

} /* namespace op */
} /* namespace rdd */
} /* namespace idgs */
