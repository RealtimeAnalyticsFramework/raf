
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/rdd/base_rdd_actor.h"



namespace idgs {
namespace rdd {

/// The stateful actor of store delegate RDD.
/// Always be leaf RDD, connect to database to get data.
class PairStoreDelegateRddActor: public idgs::rdd::BaseRddActor {
public:

  /// @brief Constructor
  PairStoreDelegateRddActor(const std::string& rddname);

  /// @brief Destructor
  virtual ~PairStoreDelegateRddActor();

  /// @brief  Generate descriptor for StoreDelegateRddActor.
  /// @return The descriptor for StoreDelegateRddActor.
  static idgs::actor::ActorDescriptorPtr generateActorDescriptor();

  /// @brief  Get the descriptor for StoreDelegateRddActor.
  /// @return The descriptor for StoreDelegateRddActor.
  const idgs::actor::ActorDescriptorPtr& getDescriptor() const override;

  const std::string& getActorName() const override {
    static std::string actorName = PAIR_STORE_DELEGATE_RDD_ACTOR;
    return actorName;
  }

  const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override;

private:
  static idgs::actor::ActorDescriptorPtr descriptor;

  void handleRddCreate(const idgs::actor::ActorMessagePtr& msg);
};

} // namespace rdd
} // namespace idgs 
