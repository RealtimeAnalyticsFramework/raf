
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/rdd/action/action.h"

namespace idgs {
namespace rdd {
namespace action {

/// Action of RDD.
/// To handle action which would sum one field or a expression.
class SumAction: public Action {
public:

  /// @brief Constructor
  SumAction();

  /// @brief Destructor
  virtual ~SumAction();

  /// Copy constructor, called by containers.
  /// This should be singleton, copy constructor should be deleted.
  SumAction(const SumAction& other) = delete;
  SumAction(SumAction&& other) = delete;
  SumAction& operator()(const SumAction& other) = delete;
  SumAction& operator()(SumAction&& other) = delete;

  /// @brief  Handle action and save result.
  /// @param  msg     The message of action request from client.
  /// @param  input   Input RDD for each partition.
  /// @param  output  Output RDD, to save action result, one or more values.
  /// @return Status code of result.
  pb::RddResultCode action(const idgs::actor::ActorMessagePtr& msg, const idgs::rdd::BaseRddPartition* input,
      std::vector<protobuf::PbVariant>& output);

  /// @brief  Aggregate all action result of each partition, and calculate total action result.
  /// @param  msg     Action message.
  /// @param  input   Action result of each partition.
  /// @return Status code of result.
  pb::RddResultCode aggregate(const idgs::actor::ActorMessagePtr& actionRequest, idgs::actor::ActorMessagePtr& actionResponse, const std::vector<std::vector<std::string>>& input);
};

} /* namespace op */
} /* namespace rdd */
} /* namespace idgs */
