
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "hash_join_transformer.h"

#include <queue>
#include "idgs/rdd/op/join_operator.h"

namespace idgs {
namespace rdd {
namespace transform {

struct KeyValuePair {
public:
  KeyValuePair() {
  }
  KeyValuePair(idgs::actor::PbMessagePtr key, idgs::actor::PbMessagePtr value) {
    this->key = key;
    this->value = value;
  }
  idgs::actor::PbMessagePtr key;
  idgs::actor::PbMessagePtr value;
};

HashJoinTransformer::HashJoinTransformer() {
}

HashJoinTransformer::~HashJoinTransformer() {
}

pb::RddResultCode HashJoinTransformer::transform(TransformerContext* ctx, const BaseRddPartition* input, PairRddPartition* output) {
  op::JoinOperator* joinOp = dynamic_cast<op::JoinOperator*>(ctx->getRddOperator());
  if (joinOp->joinType == pb::OUTER_JOIN) {
    if (ctx->getParamRdds().size() == 1) {
      handleTwoRddOuterJoin(ctx, input, output);
    } else {
      handleMoreRddOuterJoin(ctx, input, output);
    }
  } else {
    input->foreach([this, &ctx, &output] (const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
      idgs::actor::PbMessagePtr outkey, outvalue;
      ctx->getExpressionContext()->setKeyValue(&key, &value);
      ctx->getExpressionContext()->setOutputKeyValue(&outkey, &outvalue);
      if (ctx->getParamRdds().size() == 1) {
        handleTwoRddInnerOrLeftJoin(ctx, output);
      } else {
        handleMoreRddInnerOrLeftJoin(ctx, output);
      }
    });
  }

  return pb::RRC_SUCCESS;
}

pb::RddResultCode HashJoinTransformer::transform(TransformerContext* ctx, PairRddPartition* output) {
  op::JoinOperator* joinOp = dynamic_cast<op::JoinOperator*>(ctx->getRddOperator());
  if (joinOp->joinType == pb::OUTER_JOIN) {
    return pb::RRC_NOT_SUPPORT;
  } else {
    if (ctx->getParamRdds().size() == 1) {
      handleTwoRddInnerOrLeftJoin(ctx, output);
    } else {
      handleMoreRddInnerOrLeftJoin(ctx, output);
    }

    return pb::RRC_SUCCESS;
  }
}

void HashJoinTransformer::handleTwoRddInnerOrLeftJoin(TransformerContext* ctx, PairRddPartition* output) {
  op::JoinOperator* op = dynamic_cast<op::JoinOperator*>(ctx->getRddOperator());
  auto exprCtx = ctx->getExpressionContext();
  if (op->evaluate(exprCtx)) {
    auto& key = * exprCtx->getKey();
    auto& params = ctx->getParamRdds();

    auto& outkey = * exprCtx->getOutputKey();
    auto& outvalue = * exprCtx->getOutputValue();

    auto values = params[0]->getValue(key);
    if (values.empty()) {
      if (op->joinType == pb::INNER_JOIN) {
        return;
      } else {
        output->processRow(outkey, outvalue);
      }
    } else if (values.size() == 1) {
      exprCtx->setKeyValue(&key, &values.at(0));
      if (op->paramOperators.at(0)->evaluate(exprCtx)) {
        output->processRow(outkey, outvalue);
      }
    } else {
      auto it = values.begin();
      for (; it != values.end(); ++ it) {
        idgs::actor::PbMessagePtr newkey, newvalue;
        newkey.reset(outkey->New());
        newkey->CopyFrom(* outkey);
        newvalue.reset(outvalue->New());
        newvalue->CopyFrom(* outvalue);

        exprCtx->setKeyValue(&key, &(* it));
        exprCtx->setOutputKeyValue(&newkey, &newvalue);

        if (op->paramOperators.at(0)->evaluate(exprCtx)) {
          output->processRow(newkey, newvalue);
        }
      }
    }
  }
}

void HashJoinTransformer::handleMoreRddInnerOrLeftJoin(TransformerContext* ctx, PairRddPartition* output) {
  op::JoinOperator* op = dynamic_cast<op::JoinOperator*>(ctx->getRddOperator());
  auto exprCtx = ctx->getExpressionContext();
  if (op->evaluate(exprCtx)) {
    auto& key = * exprCtx->getKey();
    auto& params = ctx->getParamRdds();

    auto& outkey = * exprCtx->getOutputKey();
    auto& outvalue = * exprCtx->getOutputValue();

    std::queue<KeyValuePair> outputPairs;
    outputPairs.push(KeyValuePair(outkey, outvalue));

    for (int32_t i = 0; i < params.size(); ++ i) {
      if (outputPairs.empty()) {
        return;
      }

      auto values = params[i]->getValue(key);
      if (values.empty()) {
        if (op->joinType == pb::INNER_JOIN) {
          return;
        } else {
          continue;
        }
      } else if (values.size() == 1) {
        auto size = outputPairs.size();
        for (size_t j = 0; j < size; ++ j) {
          KeyValuePair pair = outputPairs.front();
          outputPairs.pop();

          exprCtx->setKeyValue(&key, &values.at(0));
          exprCtx->setOutputKeyValue(&pair.key, &pair.value);

          if (op->paramOperators.at(i)->evaluate(exprCtx)) {
            outputPairs.push(KeyValuePair(outkey, outvalue));
          }
        }
      } else {
        auto size = outputPairs.size();
        for (size_t j = 0; j < size; ++ j) {
          KeyValuePair pair = outputPairs.front();
          outputPairs.pop();
          auto it = values.begin();
          for (; it != values.end(); ++ it) {
            idgs::actor::PbMessagePtr newkey, newvalue;
            newkey.reset(pair.key->New());
            newkey->CopyFrom(* pair.key);
            newvalue.reset(pair.value->New());
            newvalue->CopyFrom(* pair.value);

            exprCtx->setKeyValue(&key, &(* it));
            exprCtx->setOutputKeyValue(&newkey, &newvalue);

            if (op->paramOperators.at(i)->evaluate(exprCtx)) {
              outputPairs.push(KeyValuePair(newkey, newvalue));
            }
          }
        }
      }
    }

    while (!outputPairs.empty()) {
      KeyValuePair pair = outputPairs.front();
      outputPairs.pop();
      output->processRow(pair.key, pair.value);
    }
  }
}

void HashJoinTransformer::handleTwoRddOuterJoin(TransformerContext* ctx, const BaseRddPartition* input, PairRddPartition* output) {
  input->foreach([this, &ctx, &output] (const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
    idgs::actor::PbMessagePtr outkey, outvalue;
    ctx->getExpressionContext()->setKeyValue(&key, &value);
    ctx->getExpressionContext()->setOutputKeyValue(&outkey, &outvalue);
    handleTwoRddInnerOrLeftJoin(ctx, output);
  });

  auto exprCtx = ctx->getExpressionContext();
  auto& params = ctx->getParamRdds();
  op::JoinOperator* op = dynamic_cast<op::JoinOperator*>(ctx->getRddOperator());
  auto rddOp = op->paramOperators.at(0);

  if (!params.at(0)->empty()) {
    params.at(0)->foreachGroup([this, &exprCtx, &rddOp, output] (const idgs::actor::PbMessagePtr& key, const std::vector<idgs::actor::PbMessagePtr>& values) {
      if (output->containKey(key)) {
        return;
      }

      auto vit = values.begin();
      for (; vit != values.end(); ++ vit) {
        idgs::actor::PbMessagePtr outkey, outvalue;
        exprCtx->setKeyValue(&key, &(* vit));
        exprCtx->setOutputKeyValue(&outkey, &outvalue);

        if (rddOp->evaluate(exprCtx)) {
          output->processRow(outkey, outvalue);
        }
      }
    });
  }
}

void HashJoinTransformer::handleMoreRddOuterJoin(TransformerContext* ctx, const BaseRddPartition* input, PairRddPartition* output) {
  input->foreach([this, &ctx, &output] (const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
    idgs::actor::PbMessagePtr outkey, outvalue;
    ctx->getExpressionContext()->setKeyValue(&key, &value);
    ctx->getExpressionContext()->setOutputKeyValue(&outkey, &outvalue);
    handleMoreRddInnerOrLeftJoin(ctx, output);
  });

  auto exprCtx = ctx->getExpressionContext();
  auto& params = ctx->getParamRdds();
  op::JoinOperator* op = dynamic_cast<op::JoinOperator*>(ctx->getRddOperator());
  OrderedRddDataMap tmpMap;

  for (int32_t i = 0; i < params.size(); ++ i) {
    auto rddOp = op->paramOperators.at(i);
    if (!params.at(i)->empty()) {
      params.at(i)->foreachGroup([this, &exprCtx, &rddOp, output, &tmpMap] (const idgs::actor::PbMessagePtr& key, const std::vector<idgs::actor::PbMessagePtr>& values) {
        if (output->containKey(key)) {
          return;
        }

        auto tmpit = tmpMap.find(key);
        auto vit = values.begin();
        if (tmpit == tmpMap.end()) {
          for (; vit != values.end(); ++ vit) {
            idgs::actor::PbMessagePtr outkey, outvalue;
            exprCtx->setKeyValue(&key, &(* vit));
            exprCtx->setOutputKeyValue(&outkey, &outvalue);

            if (rddOp->evaluate(exprCtx)) {
              tmpMap[outkey].push_back(outvalue);
            }
          }
        } else {
          auto tmpkey = tmpit->first;
          auto tmpvalues = tmpit->second;
          tmpMap.erase(tmpit);
          auto tmpvit = tmpvalues.begin();
          for (; tmpvit != tmpvalues.end(); ++ tmpvit) {
            if (values.size() == 1) {
              idgs::actor::PbMessagePtr outvalue = tmpvalues.at(0);

              exprCtx->setKeyValue(&key, &values.at(0));
              exprCtx->setOutputKeyValue(&tmpkey, &outvalue);

              if (rddOp->evaluate(exprCtx)) {
                tmpMap[tmpkey].push_back(outvalue);
              }
            } else {
              for (; vit != values.end(); ++ vit) {
                idgs::actor::PbMessagePtr outkey, outvalue;
                outkey.reset(tmpkey->New());
                outkey->CopyFrom(* tmpkey);
                outvalue.reset((* tmpvit)->New());
                outvalue->CopyFrom(* (* tmpvit));

                exprCtx->setKeyValue(&key, &(* vit));
                exprCtx->setOutputKeyValue(&outkey, &outvalue);

                if (rddOp->evaluate(exprCtx)) {
                  tmpMap[outkey].push_back(outvalue);
                }
              }
            }
          }
        }
      });
    }
  }

  if (!tmpMap.empty()) {
    auto it = tmpMap.begin();
    for (; it != tmpMap.end(); ++ it) {
      auto vit = it->second.begin();
      for (; vit != it->second.end(); ++ vit);
      output->processRow(it->first, (* vit));
    }
  }
}

} // namespace transform
} // namespace rdd
} // namespace idgs 
