
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "idgs/actor/scheduler.h"


#include "idgs/util/utillity.h"
#include "idgs/actor/actor_message_queue.h"

using namespace std;

namespace idgs {
namespace actor {
ScheduledMessageService::~ScheduledMessageService() {
  function_footprint();
}

std::shared_ptr<ScheduledFuture> ScheduledMessageService::schedule(std::shared_ptr<ActorMessage> msg,
    unsigned long delay) {
  DLOG_IF(FATAL, (msg->getDestMemberId() == idgs::pb::UNKNOWN_MEMBER || msg->getSourceMemberId() == idgs::pb::UNKNOWN_MEMBER))
                                                                                                                                  << "Unknown member: "
                                                                                                                                  << msg->toString();

  std::shared_ptr<ScheduledFuture> result(new ScheduledFuture(msg, sys::getCurrentTime() + delay));

  PendingFuturesType& pFuture = tasks[delay];
  if (!pFuture.get()) {
    pFuture.reset(new PendingFutures());
  }

  pFuture->queue.push(result);
  return result;
}

///
/// @todo only ScheduledFuture::timeToDispatch < now should be count?
int ScheduledMessageService::countTimeOutTask(TimerType delay) {
  int count = 0;
  if (tasks.count(delay) != 0) {
    MapType::iterator it = tasks.find(delay);
    count = (it->second->queue).unsafe_size();
  }

  return count;
}

ResultCode ScheduledMessageService::start() {
  ResultCode res = RC_ERROR;
  if (m_is_started.compare_and_swap(ACTIVE, IDLE) == IDLE) {
    DVLOG(1) << "starting ScheduledMessageService";
    m_pthread = new thread(bind(&ScheduledMessageService::run, this));
    res = RC_SUCCESS;
  } else {
    DLOG(WARNING)<< "ScheduledMessageService is already started";
    res = RC_SCHEDULER_ALREADY_STARTED;
  }
  return res;
}

/// @todo specified exception should be caught.
void ScheduledMessageService::run() {
  DVLOG(2) << "ScheduledMessageService is Running";
  set_thread_name("idgs_sched");

  while (m_is_started.load() == ACTIVE) {
    unsigned long now = sys::getCurrentTime();
    try {
      //DVLOG(2) << "current time is " << now;
      MapType::iterator it;
      for (it = tasks.begin(); it != tasks.end(); ++it) {
        fire(now, it->second);
      }
    } catch (std::exception &e) {
      LOG(ERROR)<< e.what();
    } catch(...) {
      catchUnknownException();
    }

    std::this_thread::sleep_for(dura);
  }
  DVLOG(2) << "ScheduledMessageService is out of running";
}

void ScheduledMessageService::fire(unsigned long now, PendingFuturesType pendingFutures) {
  if (pendingFutures.get() == NULL) {
    return;
  }
  if (pendingFutures->leaveBehindFutrue.get() != NULL) {

    if (pendingFutures->leaveBehindFutrue->isCanceled()) {
      pendingFutures->leaveBehindFutrue.reset();
    } else {
      if (now < pendingFutures->leaveBehindFutrue->getDispachTime()) {
        // Since the leave behind future is not time out, the timer in the queue
        // is also not time out. continue check next queue;
        return;
      } else {
        //DVLOG(2) << "leave behind timer " << pendingFutures->leaveBehindFutrue->getDispachTime()
        //   <<":" << pendingFutures->leaveBehindFutrue.get() << " is fired ";
        idgs::actor::relayTimerMessage(pendingFutures->leaveBehindFutrue->getActorMessage());

        pendingFutures->leaveBehindFutrue.reset();
      }
    }
  }

  std::shared_ptr<ScheduledFuture> future(NULL);
  while (pendingFutures->queue.try_pop(future)) {
    if (future->isCanceled()) {
      // the timer is canceled
      DVLOG(2) << "timer " << future->getDispachTime() << ":" << future.get() << "  is canceled";
      continue;
    }

    if (now < future->getDispachTime()) {
      // the timer should not be timer out, so does to the next other timer in the queue
      // but since this timer is popped out from the queue, we should record it as the
      // leave behind timer.
      pendingFutures->leaveBehindFutrue = future;
      DVLOG(2) << "timer " << future->getDispachTime() << " become leave behind";
      //future.reset();
      break;
    }

    idgs::actor::relayTimerMessage(future->getActorMessage());
    DVLOG(2) << "timer " << future->getDispachTime() << " is fired ";
  }
}

ResultCode ScheduledMessageService::stop() {
  ResultCode res = RC_ERROR;
  if (m_is_started.compare_and_swap(IDLE, ACTIVE) == ACTIVE) {
    m_pthread->join();
    DVLOG(2) << "ScheduledMessageService is terminated ";
    delete m_pthread;
    m_pthread = NULL;
    res = RC_SUCCESS;
  } else {
    DVLOG(2) << "ScheduledMessageService is already terminated ";
    res = RC_SCHEDULER_ALREADY_STOPPED;
  }
  return res;
}
} // namespace  rpc {
} // namespace idgs
