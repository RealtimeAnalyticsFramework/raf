
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/application.h"

#include "idgs/actor/thread_model_stl.h"

using namespace std;

namespace idgs {
namespace actor {
StlThreadModel::~StlThreadModel() {
  function_footprint();
  shutdown();
}

/// @todo: if the capacity is zero, set the threadNumber as the core number
int StlThreadModel::initialize(const int capacity) {
  function_footprint();
  threadNumber = capacity;

  for (int i = 0; i < threadNumber; i++) {
    receivers.push_back(std::move(ActorWorker(i)));
  }

  DVLOG(2) << "start thread number : " << threadNumber;
  return RC_SUCCESS;
}

int StlThreadModel::start() {
  function_footprint();
  vector<ActorWorker>::iterator iter = receivers.begin();
  while (iter != receivers.end()) {
    (*iter).start();
    ++iter;
  }

  return RC_SUCCESS;
}

int StlThreadModel::shutdown() {
  if (receivers.empty()) {
    return RC_SUCCESS;
  }
  LOG(INFO)<< "terminating threads: " << receivers.size();

  auto end = receivers.end();
  for (auto iter = receivers.begin(); iter != end; ++iter) {
    iter->markFinish();
  }
  for (auto iter = receivers.begin(); iter != end; ++iter) {
    idgs_application()->getActorMessageQueue()->notifyAll();
  }
  for (auto iter = receivers.begin(); iter != end; ++iter) {
    iter->join();
  }
  receivers.clear();
  return RC_SUCCESS;
}

}
}
