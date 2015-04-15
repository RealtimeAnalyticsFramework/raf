
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/rdd/pair_rdd_partition.h"


namespace idgs {
namespace rdd {
namespace transform {

/// Transformer of RDD interface class.
/// To handle transformer of each partition and save the result.
class Transformer {
public:
  Transformer();
  virtual ~Transformer();

  virtual const std::string& getName() = 0;

  /// @brief  Handle transform and process result.
  ///         The source of data is the whole data of input.
  ///         use input->foreach() or input->foreachGroup() to loop data.
  /// @param  ctx     The context of transformer.
  /// @param  input   Input RDD of each partition.
  /// @param  output  Output RDD of each partition.
  /// @return Status code of result.
  virtual idgs::rdd::pb::RddResultCode transform(TransformerContext* ctx, const idgs::rdd::BaseRddPartition* input, idgs::rdd::PairRddPartition* output);

  /// @brief  Handle transform one data and process result.
  ///         The data is in the context.
  ///         use ctx->getExpressionContext()->getKey() and ctx->getExpressionContext()->getValue() to get data.
  /// @param  ctx     The context of transformer.
  /// @param  input   Input RDD of each partition.
  /// @param  output  Output RDD of each partition.
  /// @return Status code of result.
  virtual idgs::rdd::pb::RddResultCode transform(TransformerContext* ctx, idgs::rdd::PairRddPartition* output);

  /// @brief  After repartition some RDD have to aggregate.
  /// @param  ctx     The context of transformer.
  /// @param  output  A target RDD of each partition.
  /// @return Status code of result.
  virtual idgs::rdd::pb::RddResultCode aggregate(TransformerContext* ctx, idgs::rdd::PairRddPartition* output);
};
// class Transformer

}// namespace transform

typedef std::shared_ptr<idgs::rdd::transform::Transformer> TransformerPtr;
typedef idgs::util::resource_manager<TransformerPtr> TransformerMgr;
} // namespace rdd
} // namespace idgs 
