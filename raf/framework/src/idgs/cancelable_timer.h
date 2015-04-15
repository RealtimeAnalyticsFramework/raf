
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once
#include "glog/logging.h"
#include <thread>
#include <condition_variable>

namespace idgs {

  /// cancelable timer, which is useful to unit test.
  class cancelable_timer {
  public:

    template <class Function> cancelable_timer(int seconds, Function func):timerThread(NULL){
      LOG(INFO) << "Schedule a timer:" << seconds;

      // auto handle = std::async(std::launch::async, taskFunc);
      timerThread = new std::thread([this, seconds, &func]() {
        DVLOG(5) << "timer thread enter, timeout:" << seconds;
        // wait in timer thread
        std::unique_lock<std::mutex> lk(this->lock);
        auto waitResult = this->cv.wait_for(lk, std::chrono::seconds(seconds));
        if (waitResult == std::cv_status::no_timeout) {
          // caller cancel
          DVLOG(5) << "timer canceled.";
        } else {
          // timeout
          DVLOG(5) << "timeout.";
          func();
        }
      });
    }

    void cancel() {
      LOG(INFO) << "Timer is canceled";
      // unlock in calling thread
      cv.notify_all();
      timerThread->join();
      delete timerThread;
      timerThread = NULL;
    }


    ~cancelable_timer() {
      if(timerThread) {
        cancel();
      }
    }
  private:
    std::mutex lock;
    std::condition_variable cv;

    std::thread* timerThread;
  };

#define TEST_TIMEOUT(timeout) \
  idgs::cancelable_timer __the_timer_for_unit_test(timeout, \
    [this](){ \
      LOG(ERROR) << "Timeout for test " << __FILE__ << ":" << __LINE__ << " case: " << typeid(*this).name(); \
      exit(1);\
  })
} // namespace idgs
