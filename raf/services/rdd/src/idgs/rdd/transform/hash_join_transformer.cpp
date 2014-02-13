
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "hash_join_transformer.h"
#include "idgs/rdd/pb/rdd_transform.pb.h"

using namespace idgs::actor;
using namespace idgs::rdd::pb;
using namespace idgs::expr;
using namespace protobuf;
using namespace google::protobuf;

namespace idgs {
namespace rdd {
namespace transform {

HashJoinTransformer::HashJoinTransformer() {
}

HashJoinTransformer::~HashJoinTransformer() {
}

RddResultCode HashJoinTransformer::transform(const ActorMessagePtr& msg, const vector<BaseRddPartition*>& input,
    RddPartition* output) {
  if (input.size() != 2) {
    LOG(ERROR)<< "hash join transformer '" << output->getRddName() << "' with partition " << output->getPartition() << " must have two input RDD, caused by code " << RRC_INVALID_RDD_INPUT;
    return RRC_INVALID_RDD_INPUT;
  }

  JoinRequest join;
  if (!msg->parseAttachment(JOIN_PARAM, &join)) {
    LOG(ERROR) << "The parameter of join transformer is invalid.";
    return RRC_INVALID_TRANSFORMER_PARAM;
  }

  RddResultCode code = RRC_SUCCESS;
  switch (join.type()) {
    case INNER_JOIN: {
      code = handleInnerJoin(input, output);
      break;
    }
    case LEFT_JOIN: {
      code = handleLeftJoin(input, output);
      break;
    }
    case OUTER_JOIN: {
      code = handleOuterJoin(input, output);
      break;
    }
    default: {
      code = RRC_JOIN_TYPE_NOT_FOUND;
    }
  }

  return code;
}

pb::RddResultCode HashJoinTransformer::handleLeftJoin(const std::vector<BaseRddPartition*>& input,
    RddPartition* output) {
  BaseRddPartition* input0 = input[0];
  BaseRddPartition* input1 = input[1];

  RddResultCode code = RRC_SUCCESS;
  ExpressionContext ctx;
  auto& outMsg0 = output->getOutMessage(0);
  auto& outMsg1 = output->getOutMessage(1);
  auto filterExpr0 = output->getFilterExpression(0);
  auto filterExpr1 = output->getFilterExpression(1);

  input0->foreachGroup(
      [output, input1, filterExpr0, filterExpr1, outMsg0, outMsg1, &code, &ctx] (const PbMessagePtr& key0, const vector<PbMessagePtr>& value0) {
        auto value1 = input1->getValue(key0);
        auto it0 = value0.begin();
        for (; it0 != value0.end(); ++ it0) {
          ctx.setKeyValue(&key0, &(*it0));
          try {
            if (filterExpr0) {
              PbVariant var = filterExpr0->evaluate(&ctx);
              if (!(bool) var) {
                continue;
              }
            }
          } catch (RddResultCode& err) {
            LOG(ERROR) << "evaluate error, caused by code " << err;
            code = err;
          }

          PbMessagePtr outkey, outvalue;
          outMsg0.key.fillMessage(outkey, ctx);
          outMsg0.value.fillMessage(outvalue, ctx);

          if (!value1.empty()) {
            auto it1 = value1.begin();
            for (; it1 != value1.end(); ++ it1) {
              ctx.setKeyValue(&key0, &(*it1));

              try {
                if (filterExpr1) {
                  PbVariant var = filterExpr1->evaluate(&ctx);
                  if (!(bool) var) {
                    continue;
                  }
                }
              } catch (RddResultCode& err) {
                LOG(ERROR) << "evaluate error, caused by code " << err;
                code = err;
              }

              outMsg1.value.fillMessage(outvalue, ctx);

              output->putLocal(outkey, outvalue);
            }
          } else {
            output->putLocal(outkey, outvalue);
          }
        }
      });

  return code;
}

pb::RddResultCode HashJoinTransformer::handleInnerJoin(const std::vector<BaseRddPartition*>& input,
    RddPartition* output) {
  BaseRddPartition* input0 = input[0];
  BaseRddPartition* input1 = input[1];

  RddResultCode code = RRC_SUCCESS;
  ExpressionContext ctx;
  auto filterExpr0 = output->getFilterExpression(0);
  auto filterExpr1 = output->getFilterExpression(1);
  auto& outMsg0 = output->getOutMessage(0);
  auto& outMsg1 = output->getOutMessage(1);

  input0->foreachGroup(
      [this, output, input1, filterExpr0, filterExpr1, outMsg0, outMsg1, &code, &ctx] (const PbMessagePtr& key0, const vector<PbMessagePtr>& value0) {
        auto value1 = input1->getValue(key0);
        if (!value1.empty()) {
          auto it0 = value0.begin();
          for (; it0 != value0.end(); ++ it0) {
            ctx.setKeyValue(&key0, &(*it0));
            try {
              if (filterExpr0) {
                PbVariant var = filterExpr0->evaluate(&ctx);
                if (!(bool) var) {
                  continue;
                }
              }
            } catch (RddResultCode& err) {
              LOG(ERROR) << "evaluate error, caused by code " << err;
              code = err;
            }

            PbMessagePtr outkey, outvalue;
            outMsg0.key.fillMessage(outkey, ctx);
            outMsg0.value.fillMessage(outvalue, ctx);

            auto it1 = value1.begin();
            for (; it1 != value1.end(); ++ it1) {
              ctx.setKeyValue(&key0, &(*it1));
              try {
                if (filterExpr1) {
                  PbVariant var = filterExpr1->evaluate(&ctx);
                  if (!(bool) var) {
                    continue;
                  }
                }
              } catch (RddResultCode& err) {
                LOG(ERROR) << "evaluate error, caused by code " << err;
                code = err;
              }

              outMsg1.value.fillMessage(outvalue, ctx);
              output->putLocal(outkey, outvalue);
            }
          }
        }
      });

  return code;
}

pb::RddResultCode HashJoinTransformer::handleOuterJoin(const std::vector<BaseRddPartition*>& input,
    RddPartition* output) {
  RddResultCode code = handleLeftJoin(input, output);

  if (code == RRC_SUCCESS) {
    BaseRddPartition* input0 = input[0];
    BaseRddPartition* input1 = input[1];

    auto filterExpr1 = output->getFilterExpression(1);
    auto& outMsg1 = output->getOutMessage(1);

    ExpressionContext ctx;
    input1->foreachGroup(
        [this, output, input0, filterExpr1, outMsg1, &code, &ctx] (const PbMessagePtr& key, const vector<PbMessagePtr>& value) {
          if (!input0->containKey(key)) {
            auto it = value.begin();
            for (; it != value.end(); ++ it) {
              ctx.setKeyValue(&key, &(*it));
              try {
                if (filterExpr1) {
                  PbVariant var = filterExpr1->evaluate(&ctx);
                  if (!(bool) var) {
                    continue;
                  }
                }
              } catch (RddResultCode& err) {
                LOG(ERROR) << "evaluate error, caused by code " << err;
                code = err;
              }

              PbMessagePtr outkey, outvalue;
              outMsg1.key.fillMessage(outkey, ctx);
              outMsg1.value.fillMessage(outvalue, ctx);

              output->putLocal(outkey, outvalue);
            }
          }
        });
  }

  return code;
}

} // namespace transform
} // namespace rdd
} // namespace idgs 
