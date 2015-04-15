/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $
#include "tpch_Q6_action.h"
#include "idgs/util/utillity.h"
#include "idgs/rdd/pb/rdd_action.pb.h"

using namespace std;
using namespace protobuf;
using namespace idgs::actor;
using namespace idgs::rdd;
using namespace idgs::rdd::action;
using namespace idgs::rdd::pb;

namespace idgs {
namespace tpc {
namespace action {

TpchQ6Action::TpchQ6Action() {
}

TpchQ6Action::~TpchQ6Action() {
}

RddResultCode TpchQ6Action::action(ActionContext* ctx, const BaseRddPartition* input) {
  double sum = 0;

  if (!input->empty()) {
    auto valueTemplate = input->getValueTemplate();
    auto descriptor = valueTemplate->GetDescriptor();
    auto reflection = valueTemplate->GetReflection();

    auto priceField = descriptor->FindFieldByName("l_extendedprice");
    auto discountField = descriptor->FindFieldByName("l_discount");

    input->foreach([&sum, reflection, priceField, discountField] (const PbMessagePtr& key, const PbMessagePtr& value) {
      double price = 0, discount = 0;
      price = reflection->GetDouble(* value, priceField);
      discount = reflection->GetDouble(* value, discountField);

      sum += price * discount;
    });
  }

  PbVariant var;
  var = sum;
  ctx->addPartitionResult(var);

  return RRC_SUCCESS;
}

RddResultCode TpchQ6Action::aggregate(ActionContext* ctx) {
  double sum = 0;
  auto& input = ctx->getAggregateResult();
  for (size_t partition = 0; partition < input.size(); ++partition) {
    double pSize = 0;
    ResultCode code = sys::convert<double>(input[partition][0], pSize);
    if (code != RC_SUCCESS) {
      LOG(ERROR)<< "cannot convert \"" << input[partition][0] << "\" to a number.";
      return RRC_DATA_CONVERT_ERROR;
    }

    sum += pSize;
  }

  shared_ptr<SumActionResult> result = make_shared<SumActionResult>();
  result->set_total(sum);

  ctx->setActionResult(result);

  return RRC_SUCCESS;
}

} /* namespace op */
} /* namespace rdd */
} /* namespace idgs */
