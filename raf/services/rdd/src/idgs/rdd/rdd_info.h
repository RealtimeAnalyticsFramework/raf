
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <vector>
#include "idgs/rdd/pb/rdd_svc.pb.h"
#include "idgs/rdd/pb/rdd_internal.pb.h"

namespace idgs {
namespace rdd {

enum PartitionState {
  P_INIT = 0,           // when has instance of partition
  P_CREATE = 1,         // when partition process done
  P_PREPARED = 2,       // when partition is prepared to do transform
  P_TRANSFORM_DONE = 3, // when partition transform has already done
  P_READY = 4,          // when RDD transform done
  P_ERROR = 5
};

/// Information about RDD partition actor.
/// To access some necessary information of RDD partition actor.
class PartitionInfo {
public:

  /// @brief Constructor
  PartitionInfo();

  /// @brief Destructor
  virtual ~PartitionInfo();

  /// @brief  Set RDD partition actor ID.
  /// @param  actorId RDD partition actor ID.
  void setActorId(const idgs::pb::ActorId& actorId);

  /// @brief  Set the state of RDD partition actor.
  /// @param  state The state of RDD partition actor.
  void setState(const PartitionState& state);

  /// @brief   Get RDD partition actor ID.
  /// @return  RDD partition actor ID.
  const idgs::pb::ActorId& getActorId() const;

  /// @brief   Get the state of RDD partition actor.
  /// @return  The state of RDD partition actor.
  const PartitionState& getState() const;

private:
  idgs::pb::ActorId actorId;
  PartitionState state;
};

/// Information about RDD actor.
/// To access some necessary information of RDD actor.
class RddInfo {
public:

  /// @brief Constructor
  RddInfo();

  /// @brief Destructor
  ~RddInfo();

  /// @brief  Set RDD actor ID.
  /// @param  actorId RDD actor ID.
  void setActorId(const int32_t& memberId, const std::string& actorId);

  /// @brief  Set the state of RDD actor.
  /// @param  state The state of RDD actor.
  void setState(const pb::RddState& state);

  /// @brief  Add a partition information.
  /// @param  partition Partition of store.
  /// @param  actorId   Actor ID(include member ID and actor ID) of RDD partition actor.
  /// @param  state     The state of RDD partition actor, default is INIT.
  void addPartitionInfo(const uint32_t& partition, const idgs::pb::ActorId& actorId, const PartitionState& state =
      P_INIT);

  /// @brief  Update state of RDD partition actor.
  /// @param  partition Partition of store.
  /// @param  state     The state of RDD partition actor, default is INIT
  void setPartitionState(const uint32_t& partition, const PartitionState& state);

  /// @brief   Get RDD actor ID.
  /// @return  RDD actor ID.
  const idgs::pb::ActorId& getActorId() const;

  /// @brief   Get the state of RDD actor.
  /// @return  The state of RDD actor.
  const pb::RddState& getState() const;

  /// @brief  Get the state of the given RDD partition actor.
  /// @param  partition Partition of store.
  /// @return The state of RDD partition actor.
  const PartitionState& getPartitionState(const uint32_t& partition) const;

  /// @brief  Get all RDD partition actor information.
  /// @return The information of all RDD partition actor.
  const std::vector<PartitionInfo>& getPartitionInfo() const;

  /// @brief  Whether the given actor ID is current actor.
  /// @param  actorId The given actor ID.
  /// @return True or false.
  bool isTheRdd(const idgs::pb::ActorId& actorId);

  const std::string& getRddName() const {
    return rddName;
  }

  void setRddName(const std::string& rddName) {
    this->rddName = rddName;
  }

private:
  idgs::pb::ActorId actorId;
  pb::RddState state;
  std::vector<PartitionInfo> partitionInfo;
  std::string rddName;
};

} /* namespace rdd */
} /* namespace idgs */
