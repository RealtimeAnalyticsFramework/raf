
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "group_transformer.h"

#include "idgs/cluster/cluster_framework.h"

using namespace std;
using namespace idgs::actor;
using namespace idgs::rdd::pb;
using namespace idgs::expr;
using namespace protobuf;

#if !defined(REPARTITION_BATCH)
#define REPARTITION_BATCH 100
#endif // !defined(REPARTITION_BATCH)
namespace idgs {
namespace rdd {
namespace transform {

GroupTransformer::GroupTransformer() {
}

GroupTransformer::~GroupTransformer() {
}

RddResultCode GroupTransformer::transform(const ActorMessagePtr& msg, const vector<BaseRddPartition*>& input,
    RddPartition* output) {
  if (input.size() != 1) {
    LOG(ERROR)<< "group transformer '" << output->getRddName() << "' with partition " << output->getPartition() << " must have one input RDD, caused by code " << RRC_INVALID_RDD_INPUT;
    return RRC_INVALID_RDD_INPUT;
  }

  if (input[0]->empty()) {
    DVLOG(3) << "group transformer '" << output->getRddName() << "' with partition " << output->getPartition() << " have no data.";
    return RRC_SUCCESS;
  }

  RddResultCode code = RRC_SUCCESS;
  ExpressionContext ctx;
  auto& outMsg = output->getOutMessage(0);
  auto filterExpr = output->getFilterExpression(0);

  std::multimap<PbMessagePtr, PbMessagePtr, idgs::store::less> localCache;

  input[0]->foreach([this, output, outMsg, filterExpr, &code, &ctx, &localCache] (const PbMessagePtr& key, const PbMessagePtr& value) {
        ctx.setKeyValue(&key, &value);
        try {
          if (filterExpr) {
            PbVariant var = filterExpr->evaluate(&ctx);
            if (!(bool) var) {
              return;
            }
          }
        } catch (RddResultCode& err) {
          LOG(ERROR) << "evaluate error, caused by code " << err;
          code = err;
        }

        PbMessagePtr outKey, outValue;
        outMsg.key.fillMessage(outKey, ctx);
        outMsg.value.fillMessage(outValue, ctx);
#if (REPARTITION_BATCH <= 1)
        std::vector<PbMessagePtr> values;
        values.push_back(outValue);
        output->put(outKey, values);
#else // (REPARTITION_BATCH == 0)
        localCache.insert(std::pair<PbMessagePtr, PbMessagePtr>(outKey, outValue));

        static idgs::pb::ClusterConfig* clusterConfig = ::idgs::util::singleton<idgs::cluster::ClusterFramework>::getInstance().getClusterConfig();

        if(localCache.size() > clusterConfig->repartition_batch()) {
          this->putData(output, localCache);
        }
#endif // (REPARTITION_BATCH == 0)
      });
#if (REPARTITION_BATCH > 1)
  putData(output, localCache);
#endif
  return code;
}

void GroupTransformer::putData(RddPartition* output,
    std::multimap<PbMessagePtr, PbMessagePtr, idgs::store::less>& localCache) {
  if (!localCache.empty()) {
    PbMessagePtr key;
    std::vector<PbMessagePtr> values;
    for (auto it = localCache.begin(); it != localCache.end(); ++it) {
      if (idgs::store::equals_to()(const_cast<PbMessagePtr&>(it->first), key)) {
        values.push_back(it->second);
      } else {
        if (!values.empty()) {
          output->put(key, values);
          values.clear();
        }
        values.clear();
        key = it->first;
        values.push_back(it->second);
      }
    }
    if (!values.empty()) {
      output->put(key, values);
      values.clear();
    }

    localCache.clear();
  }
}

} // namespace transform
} // namespace rdd
} // namespace idgs 
