
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/actor/actor_message.h"
#include "idgs/rdd/base_rdd_partition.h"
#include "idgs/rdd/pb/rdd_svc.pb.h"
#include "protobuf/pbvariant.h"

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
  /// @param  msg     The message of action request from client.
  /// @param  input   Input RDD for each partition.
  /// @param  output  Output RDD, to save action result, one or more values.
  /// @return Status code of result.
  virtual pb::RddResultCode action(const idgs::actor::ActorMessagePtr& actionRequest, const idgs::rdd::BaseRddPartition* input,
      std::vector<protobuf::PbVariant>& output) = 0;

  /// @brief  Aggregate all action result of each partition, and calculate total action result.
  /// @param  msg     Action message.
  /// @param  input   Action result of each partition.
  /// @return Status code of result.
  virtual pb::RddResultCode aggregate(const idgs::actor::ActorMessagePtr& actionRequest, idgs::actor::ActorMessagePtr& actionResponse,
      const std::vector<std::vector<std::string>>& input) = 0;
};

} // namespace action
} // namespace rdd
} // namespace idgs 
