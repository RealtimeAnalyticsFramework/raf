
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "count_action.h"
#include "idgs/util/utillity.h"
#include "idgs/rdd/rdd_const.h"
#include "idgs/rdd/pb/rdd_action.pb.h"
#include "idgs/expr/expression_factory.h"

using namespace std;
using namespace protobuf;
using namespace idgs::actor;
using namespace idgs::rdd::pb;
using namespace idgs::expr;

namespace idgs {
namespace rdd {
namespace action {

CountAction::CountAction() {
}

CountAction::~CountAction() {
}

RddResultCode CountAction::action(const idgs::actor::ActorMessagePtr& msg, const BaseRddPartition* input,
    std::vector<PbVariant>& output) {
  ActionRequest* payload = dynamic_cast<ActionRequest*>(msg->getPayload().get());
  if (!payload->has_filter()) { /// like count(*) or count(field)
    VLOG(5) << "count(*)";
    output.push_back(input->size());
    return RRC_SUCCESS;
  } /// end like count(*) or count(field)

  VLOG(5) << "count(*) where";
  Expression* filter_expr = NULL;
  auto rc = ExpressionFactory::build(&filter_expr, payload->filter(), input->getKeyTemplate(),
      input->getValueTemplate());
  if (idgs::RC_SUCCESS != rc) {
    LOG(ERROR)<< "RDD \"" << input->getRddName() << " partition[" << input->getPartition() << "]" << "parse filter expression error, caused by " << getErrorDescription(rc);
    return RRC_NOT_SUPPORT;
  }
  ExpressionContext ctx;
  size_t count = 0;
  input->foreach([filter_expr, &count, &ctx] (const PbMessagePtr& key, const PbMessagePtr& value) {
    ctx.setKeyValue(&key, &value);
    bool flag = (bool) filter_expr->evaluate(&ctx);
    if(flag) {
      ++count;
    }
  });
  output.push_back(count);
  return RRC_SUCCESS;
}

RddResultCode CountAction::aggregate(const idgs::actor::ActorMessagePtr& actionRequest, idgs::actor::ActorMessagePtr& actionResponse, const vector<vector<string>>& input) {
  size_t size = 0;
  for (size_t partition = 0; partition < input.size(); ++partition) {
    size_t pSize = 0;
    ResultCode code = sys::convert<size_t>(input[partition][0], pSize);
    if (code != RC_SUCCESS) {
      LOG(ERROR)<< "cannot convert \"" << input[partition][0] << "\" to a number.";
      return RRC_DATA_CONVERT_ERROR;
    }

    size += pSize;

    DVLOG(2) << "partition : " << partition << " data size : " << pSize;
  }

  DVLOG(2) << "total size : " << size;

  shared_ptr<CountActionResult> response(new CountActionResult);
  response->set_size(size);

  actionResponse->setAttachment(ACTION_RESULT, response);

  return RRC_SUCCESS;
}

} // namespace action
} // namespace rdd
} // namespace idgs 
