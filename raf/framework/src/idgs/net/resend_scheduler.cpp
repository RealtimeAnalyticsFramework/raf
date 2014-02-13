
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/

#include "resend_scheduler.h"
#include "idgs/actor/rpc_framework.h"

namespace idgs {
namespace net {

ResendScheduler::ResendScheduler() {
  // TODO Auto-generated constructor stub

}

ResendScheduler::~ResendScheduler() {
  // TODO Auto-generated destructor stub
}

void ResendScheduler::scheduleResend(const idgs::actor::ActorMessagePtr& msg) {
  static idgs::actor::ScheduledMessageService& service = ::idgs::util::singleton<idgs::actor::RpcFramework>::getInstance().getScheduler();
  std::shared_ptr<idgs::actor::ScheduledFuture> future = service.schedule(msg, 10);

  outReceipts.insert(std::make_pair(msg->getRpcMessage()->message_id(), future));
}

void ResendScheduler::cancelResendTimer(uint64_t msgId) {
  TimerMap::const_accessor a;
  if (outReceipts.find(a, msgId)) {
    a->second->cancel();
    outReceipts.erase(msgId);
  }
}


} /* namespace net */
} /* namespace idgs */
