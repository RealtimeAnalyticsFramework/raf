
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/rdd/action/action_context.h"
#include "idgs/rdd/base_rdd_partition.h"


namespace idgs {
namespace rdd {
namespace action {

/// Action of RDD interface class.
/// To handle action of each partition and aggregate each partition result to calculate total result.
class Action {
public:

  /// @brief Constructor
  Action();

  /// @brief Destructor
  virtual ~Action();

  /// @brief  Handle action and save result.
  ///         use ctx->addPartitionResult(result) to set result of each partition.
  /// @param  ctx     The context of action.
  /// @param  input   Input RDD for each partition.
  /// @return Status code of result.
  virtual pb::RddResultCode action(ActionContext* ctx, const idgs::rdd::BaseRddPartition* input) = 0;

  /// @brief  Aggregate all action result of each partition, and calculate total action result.
  ///         use ctx->getAggregateResult() to get action result from each partition.
  ///         use ctx->setActionResult(result) to set final result.
  /// @param  ctx     The context of action.
  /// @return Status code of result.
  virtual pb::RddResultCode aggregate(ActionContext* ctx) = 0;

};

} // namespace action

typedef std::shared_ptr<idgs::rdd::action::Action> ActionPtr;
typedef idgs::util::resource_manager<ActionPtr> ActionMgr;

} // namespace rdd
} // namespace idgs 
