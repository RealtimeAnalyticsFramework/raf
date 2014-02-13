
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "scheduled_future.h"
#include <tbb/concurrent_queue.h>
#include <thread>

namespace idgs {
namespace actor {

typedef tbb::concurrent_queue<std::shared_ptr<ScheduledFuture>> QueueType;

class PendingFutures {
  friend class ScheduledMessageService;
public:
  PendingFutures() :
      leaveBehindFutrue(NULL) {
  }

  /// copy constructor, called by containers.
  PendingFutures(const PendingFutures& other) = default;

  /// deconstructor
  ~PendingFutures() = default;

private:
  QueueType queue;
  std::shared_ptr<ScheduledFuture> leaveBehindFutrue;
};

class ScheduledMessageService {
public:
  ScheduledMessageService(long _dura = 10) :
      m_pthread(NULL), dura(_dura) {
    m_is_started.store(IDLE);
  }
  /// copy constructor, called by containers.
  /// this should be singleton, copy constructor should be deleted.
  ScheduledMessageService(const ScheduledMessageService& other) = delete;
  ScheduledMessageService(ScheduledMessageService&& other) = delete;
  ScheduledMessageService& operator()(const ScheduledMessageService& other) = delete;
  ScheduledMessageService& operator()(ScheduledMessageService&& other) = delete;

  ~ScheduledMessageService();

  std::shared_ptr<ScheduledFuture> schedule(std::shared_ptr<ActorMessage> msg, TimerType delay);

  int countTimeOutTask(TimerType key);

  inline void setDuration(long _dura) {
    dura = std::chrono::milliseconds(_dura);
  }

  ResultCode stop();

  ResultCode start();

private:
  enum State {
    IDLE, ACTIVE
  };

  typedef std::shared_ptr<PendingFutures> PendingFuturesType;
  typedef std::map<TimerType, PendingFuturesType> MapType;

  void run();
  static void fire(TimerType now, PendingFuturesType pendingFutures);

  MapType tasks;
  //std::shared_ptr<std::thread> m_pthread;
  std::thread *m_pthread;

  tbb::atomic<State> m_is_started;
  std::chrono::milliseconds dura;
};

} // namespace  rpc {
} // namespace idgs
