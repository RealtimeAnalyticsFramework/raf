
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include <gtest/gtest.h>


TEST(scheduler_test, dummy) {
  int i = 0;
  EXPECT_EQ(i, 0);

}

// TEST(scheduler_test, scheduler_test) {
//  ScheduledMessageService& service = ::idgs::util::singleton<RpcFramework>::getInstance().getScheduler();
//
//  // Test if thread is started
//  ResultCode res = service.start();
//  DLOG(INFO) << "result is " << getErrorDescription(res);
//  ASSERT_EQ(RC_SUCCESS,res);
//
//  res = service.start();
//  DLOG(INFO) << "result is " << getErrorDescription(res);
//  ASSERT_EQ(RC_SCHEDULER_ALREADY_STARTED,res);
//
//  // Test for 2000ms queue
//  std::shared_ptr<ActorMessage> msg1(new ActorMessage());
//  std::shared_ptr<ScheduledFuture> future1 = service.schedule(msg1,3000);
//  DLOG(INFO) << "get dispatch time " << future1->getDispachTime();
//
//  std::shared_ptr<ActorMessage> msg2(new ActorMessage());
//  std::shared_ptr<ScheduledFuture> future2 = service.schedule(msg2,3000);
//  DLOG(INFO) << "get dispatch time " << future2->getDispachTime();
//
//  DLOG(INFO) << "countTimeOutTask for 3000ms " << service.countTimeOutTask(3000);
//
//  // Test for 4000ms queue
//  std::shared_ptr<ActorMessage> msg3(new ActorMessage());
//  std::shared_ptr<ScheduledFuture> future3 = service.schedule(msg3,6000);
//  DLOG(INFO) << "get dispatch time " << future3->getDispachTime();
//
//  std::shared_ptr<ActorMessage> msg4(new ActorMessage());
//  std::shared_ptr<ScheduledFuture> future4 = service.schedule(msg4,6000);
//  DLOG(INFO) << "get dispatch time " << future4->getDispachTime();
//
//  std::shared_ptr<ActorMessage> msg5(new ActorMessage());
//  std::shared_ptr<ScheduledFuture> future5 = service.schedule(msg5,6000);
//  DLOG(INFO) << "get dispatch time " << future5->getDispachTime();
//
//  DLOG(INFO) << "countTimeOutTask for 6000ms " << service.countTimeOutTask(6000);
//
//  ASSERT_EQ(2,service.countTimeOutTask(3000));
//  ASSERT_EQ(3,service.countTimeOutTask(6000));
//
//  ::sleep(4);
//
//  res = service.stop();
//  DLOG(INFO) << "result is " << getErrorDescription(res);
//  ASSERT_EQ(RC_SUCCESS,res);
//
//
//  res = service.stop();
//  DLOG(INFO) << "result is " << getErrorDescription(res);
//  ASSERT_EQ(RC_SCHEDULER_ALREADY_STOPPED,res);
//}
