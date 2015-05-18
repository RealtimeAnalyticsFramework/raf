
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once
#include "idgs/actor/actor_message.h"

namespace idgs {
namespace actor {
typedef unsigned long TimerType;

///
/// receipt of scheduler service,
/// actor may keep reference to this object to cancel it.
///
struct ScheduledFuture {
public:
  /// construct an instance, called by scheduler service.
  ///
  ScheduledFuture(std::shared_ptr<ActorMessage> msg, TimerType time) :
      actorMsg(msg), timeToDispatch(time), canceled(false) {
  }

  /// default constructor, called by containers.
  ScheduledFuture() :
      timeToDispatch(-1), canceled(false) {
  }
  ;

  /// copy constructor, called by containers.
  ScheduledFuture(const ScheduledFuture& other) = default;

  /// move constructor, called by containers.
  ScheduledFuture(ScheduledFuture&& other) = default;

  /// copy assignment, called by containers.
  ScheduledFuture& operator =(const ScheduledFuture& other) = default;

  /// move assignment, called by containers.
  ScheduledFuture& operator =(ScheduledFuture&& other) = default;

  /// deconstructor
  ~ScheduledFuture() = default;

  /// cancel this timer
  void cancel();

  /// check whether this timer is canceled.
  /// @return whether the timer is canceled.
  bool isCanceled() const {
    return canceled;
  }
  ;

  /// get actor message
  /// @return reference to actor message.
  ActorMessagePtr& getActorMessage() {
    return actorMsg;
  }
  ;

  /// get the time to dispatch the message.
  /// @return the expired time of the timer, in ms.
  unsigned long getDispachTime() const {
    return timeToDispatch;
  }
  ;

private:
  std::shared_ptr<ActorMessage> actorMsg;
  TimerType timeToDispatch;
  bool canceled;
};

} // namespace  rpc {
} // namespace idgs
