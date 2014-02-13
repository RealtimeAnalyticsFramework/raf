/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/

#pragma once

#include <memory>

#include "idgs/rdd/pb/rdd_common.pb.h"
#include "idgs/pb/rpc_message.pb.h"

namespace idgs {
namespace rdd {

class RddMetaInfo {
public:
  const idgs::pb::ActorId& getActorId() const {
    return actorId;
  }

  void setActorId(const int32_t& memberId, const std::string& actorId) {
    this->actorId.set_actor_id(actorId);
    this->actorId.set_member_id(memberId);
  }

  void setActorId(const idgs::pb::ActorId& rddActorId) {
    this->actorId = rddActorId;
  }

  const std::string& getRddName() const {
    return rddName;
  }

  void setRddName(const std::string& rddName) {
    this->rddName = rddName;
  }

private:
  idgs::pb::ActorId actorId;
  std::string rddName;
};

/// Information about RDD snap shot, excluding some dynamic information
/// comparing to RddInfo.
class RddSnapshot {
public:

  /// @brief Constructor
  RddSnapshot(){}

  /// @brief Destructor
  ~RddSnapshot(){}

  /// @brief  Add depending RDD.
  /// @param  rddActorId RDD actor ID.
  void addDependingRdd(const idgs::pb::ActorId& rddActorId, const std::string& rddName="");

  std::vector<idgs::rdd::RddMetaInfo>& getDependingRdd();

  const idgs::rdd::RddMetaInfo& getSelfInfo() const;

  void setSelfInfo(const idgs::rdd::RddMetaInfo& selfInfo);

  void setSelfInfo(const idgs::pb::ActorId& rddActorId, const std::string& rddName="") ;

  void setSelfInfo(const int32_t& memberId, const std::string& actorId, const std::string& rddName="") ;

  idgs::rdd::pb::RddResultCode save();

  static std::shared_ptr<RddSnapshot> restore(const std::string& rddName, pb::RddResultCode& code);

private:
  idgs::rdd::RddMetaInfo selfInfo;
  /// the memeber id and actor id with rdd state of depending rdds
  std::vector<idgs::rdd::RddMetaInfo> dependingRdds;
};


} // namespace rdd 
} // namespace idgs 
