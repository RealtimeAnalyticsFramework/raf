/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $
#include "ssb_Q1_action.h"
#include "idgs/util/utillity.h"
#include "idgs/rdd/rdd_const.h"
#include "idgs/rdd/pb/rdd_action.pb.h"

using namespace std;
using namespace protobuf;
using namespace idgs::actor;
using namespace idgs::rdd;
using namespace idgs::rdd::pb;

namespace idgs {
namespace tpc {
namespace action {

SsbQ1_1Action::SsbQ1_1Action() {
}

SsbQ1_1Action::~SsbQ1_1Action() {
}

RddResultCode SsbQ1_1Action::action(const idgs::actor::ActorMessagePtr& msg, const BaseRddPartition* input,
    std::vector<PbVariant>& output) {
  double sum = 0;

  if (!input->empty()) {
    auto valueTemplate = input->getValueTemplate();
    auto descriptor = valueTemplate->GetDescriptor();
    auto reflection = valueTemplate->GetReflection();

    auto priceField = descriptor->FindFieldByName("lo_extendedprice");
    auto discountField = descriptor->FindFieldByName("lo_discount");

    input->foreach([&sum, reflection, priceField, discountField] (const PbMessagePtr& key, const PbMessagePtr& value) {
      double price = 0, discount = 0;
      price = reflection->GetDouble(* value, priceField);
      discount = reflection->GetDouble(* value, discountField);

      sum += price * discount;
    });
  }

  PbVariant var;
  var = sum;
  output.push_back(var);

  return RRC_SUCCESS;
}

RddResultCode SsbQ1_1Action::aggregate(const idgs::actor::ActorMessagePtr& actionRequest, idgs::actor::ActorMessagePtr& actionResponse, const vector<vector<string>>& input) {
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
