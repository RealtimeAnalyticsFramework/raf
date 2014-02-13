
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <memory>
#include <vector>
#include "idgs/actor/rpc_framework.h"
#include "idgs/rdd/pb/rdd_svc.pb.h"

namespace idgs {
namespace rdd {
namespace action {

/// Action information of RDD
/// To save the information of RDD action.
class RddAction {
public:

  /// @brief Constructor
  RddAction();

  /// @brief Destructor
  virtual ~RddAction();

  /// @brief  Get the action ID.
  /// @return Action ID.
  const std::string& getActionId() const;

  /// @brief  Get the action operation name.
  /// @return Action operation name.
  const std::string& getActionOpName() const;

  /// @brief  Get the action message.
  /// @return Action message.
  const idgs::actor::ActorMessagePtr& getMessage() const;

  /// @brief  Get the result code of action.
  /// @return Result code of action.
  const pb::RddResultCode& getActionResultCode() const;

  /// @brief  Get the state of action.
  /// @return State of action.
  const idgs::rdd::pb::RddState getState() const;

  /// @brief  Get the state of given partition.
  /// @param  partition Partition of store.
  /// @return State of given partition.
  const idgs::rdd::pb::RddState& getPartitionState(const uint32_t& partition) const;

  /// @brief  Get the result of action.
  /// @return Result of action.
  const std::vector<std::vector<std::string>>& getActionResult() const;

  /// @brief  Set action ID.
  /// @param  actionid Action ID.
  void setActionId(const std::string& actionid);

  /// @brief  Set the action operation name.
  /// @param  actionOpName Action operation name.
  void setActionOpName(const std::string& actionOpName);

  /// @brief  Set the action message.
  /// @param  message Action message.
  void setMessage(idgs::actor::ActorMessagePtr message);

  /// @brief  Set the result code of action.
  /// @param  actionResultCode Result code of action.
  void setActionResultCode(const pb::RddResultCode& actionResultCode);

  /// @brief  Set the state of the given partition.
  /// @param  partition Partition of store.
  /// @param  state     State of parititon.
  void setPartitionState(const uint32_t& partition, const idgs::rdd::pb::RddState& state);

  /// @brief  Add the result of the given partition.
  /// @param  partition Partition of store.
  /// @param  result    Result of each partition.
  void addActionResult(const uint32_t& partition, const std::string& result);

private:
  std::string actionId;
  idgs::rdd::pb::RddState state;
  idgs::actor::ActorMessagePtr message;
  std::vector<idgs::rdd::pb::RddState> partitionState;
  std::vector<std::vector<std::string>> actionResult;
  pb::RddResultCode code;
  std::string actionOpName;
};

typedef std::shared_ptr<RddAction> RddActionPtr;

} // namespace action
} // namespace rdd
} // namespace idgs
