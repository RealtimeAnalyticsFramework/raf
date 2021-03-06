
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "transformer.h"

namespace idgs {
namespace rdd {
namespace transform {

/// Transformer of RDD.
/// To handle transform which would do join of two or more stores.
class HashJoinTransformer: public Transformer {
public:
  HashJoinTransformer();
  virtual ~HashJoinTransformer();

  virtual const std::string& getName() override {
    static std::string name(HASH_JOIN_TRANSFORMER);
    return name;
  }


  HashJoinTransformer(const HashJoinTransformer& other) = delete;
  HashJoinTransformer(HashJoinTransformer&& other) = delete;
  HashJoinTransformer& operator()(const HashJoinTransformer& other) = delete;
  HashJoinTransformer& operator()(HashJoinTransformer&& other) = delete;

  virtual pb::RddResultCode transform(TransformerContext* ctx, const BaseRddPartition* input, PairRddPartition* output) override;
  virtual pb::RddResultCode transform(TransformerContext* ctx, PairRddPartition* output) override;

private:
  void handleTwoRddInnerOrLeftJoin(TransformerContext* ctx, PairRddPartition* output);
  void handleMoreRddInnerOrLeftJoin(TransformerContext* ctx, PairRddPartition* output);

  void handleTwoRddOuterJoin(TransformerContext* ctx, const BaseRddPartition* input, PairRddPartition* output);
  void handleMoreRddOuterJoin(TransformerContext* ctx, const BaseRddPartition* input, PairRddPartition* output);

};

} // namespace transform
} // namespace rdd
} // namespace idgs 
