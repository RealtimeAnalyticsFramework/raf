
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "base_rdd_partition.h"

#include "idgs/rdd/rdd_local.h"

namespace idgs {
namespace rdd {

BaseRddPartition::BaseRddPartition() : partition(0), rddLocal(NULL) {
}

BaseRddPartition::~BaseRddPartition() {
}

const std::string& BaseRddPartition::getRddName() const {
  return rddName;
}

const std::string& BaseRddPartition::getPartitionName() const {
  return partitionName;
}

uint32_t BaseRddPartition::getPartition() const {
  return partition;
}

void BaseRddPartition::setRddLocal(const std::shared_ptr<RddLocal>& rddlocal) {
  rddLocal = rddlocal;
}

const std::shared_ptr<RddLocal>& BaseRddPartition::getRddLocal() const {
  return rddLocal;
}

const idgs::actor::PbMessagePtr& BaseRddPartition::getKeyTemplate() const {
  return rddLocal->getKeyTemplate();
}

const idgs::actor::PbMessagePtr& BaseRddPartition::getValueTemplate() const {
  return rddLocal->getValueTemplate();
}

void BaseRddPartition::transform() {
  auto& downstreamPartition = rddLocal->getDownstreamPartition(partition);
  if (!downstreamPartition.empty()) {
    pb::RddResultCode code = pb::RRC_SUCCESS;
    for (int32_t i = 0; i < downstreamPartition.size(); ++ i) {
      auto& rddlocal = downstreamPartition[i]->getRddLocal();
      auto& ctx = transformerContexts.at(i);

      if (!rddlocal->isUpstreamSync()) {
        auto upstreamPartition = rddlocal->getUpstreamPartition(partition);
        if (upstreamPartition[0] == this) {
          if (ctx.getParamRdds().empty()) {
            std::vector<BaseRddPartition*> params;
            for (int32_t i = 1; i < upstreamPartition.size(); ++ i) {
              params.push_back(upstreamPartition[i]);
            }
            ctx.setParamRdds(params);
          }
        } else {
          continue;
        }
      }

      code = rddlocal->getTransformer()->transform(&ctx, this, downstreamPartition[i]);
      if (code != idgs::rdd::pb::RRC_SUCCESS) {
        LOG(ERROR) << getPartitionName() << " transform error, caused by " << idgs::rdd::pb::RddResultCode_Name(code);
      }
    }
  }
}

void BaseRddPartition::transform(const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) {
  if (!key.get()) {
    LOG(ERROR)<< getPartitionName() << " transform data, key is null";
    return;
  }

  if(!value) {
    LOG(ERROR)<< getPartitionName() << " transform data, value is null";
    return;
  }

  auto& downstreamPartition = rddLocal->getDownstreamPartition(partition);
  if (!downstreamPartition.empty()) {
    idgs::rdd::pb::RddResultCode code = idgs::rdd::pb::RRC_SUCCESS;
    for (int32_t i = 0; i < downstreamPartition.size(); ++ i) {
      auto& rddlocal = downstreamPartition[i]->getRddLocal();
      auto& ctx = transformerContexts.at(i);

      idgs::actor::PbMessagePtr outkey, outvalue;
      ctx.getExpressionContext()->setKeyValue(&key, &value);
      ctx.getExpressionContext()->setOutputKeyValue(&outkey, &outvalue);

      if (!rddlocal->isUpstreamSync()) {
        auto upstreamPartition = rddlocal->getUpstreamPartition(partition);
        if (upstreamPartition[0] == this) {
          if (ctx.getParamRdds().empty()) {
            std::vector<BaseRddPartition*> params;
            for (int32_t i = 1; i < upstreamPartition.size(); ++ i) {
              params.push_back(upstreamPartition[i]);
            }
            ctx.setParamRdds(params);
          }
        } else {
          continue;
        }
      }

      code = rddlocal->getTransformer()->transform(&ctx, downstreamPartition[i]);
      if (code != idgs::rdd::pb::RRC_SUCCESS) {
        LOG(ERROR) << getPartitionName() << " transform error, caused by " << idgs::rdd::pb::RddResultCode_Name(code);
      }
    }
  }
}

void BaseRddPartition::addTransformerContext(const transform::TransformerContext& ctx) {
  transformerContexts.push_back(ctx);
}

void BaseRddPartition::setAggrTransformerContext(transform::TransformerContext& ctx) {
  aggrCtx = ctx;
}

} // namespace rdd
} // namespace idgs 
