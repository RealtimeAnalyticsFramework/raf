
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/actor/actor_manager.h"
#include "idgs/actor/thread_model_stl.h"
#include "idgs/net/network_model_asio.h"
#include "idgs/actor/scheduler.h"

namespace idgs {
namespace actor {

template<class NETWORKMODEL, class THREADMODEL = StlThreadModel, class ACTORFRAME = ActorManager>
class BasicRpcFramework {
public:
  BasicRpcFramework() = default;
  BasicRpcFramework(const BasicRpcFramework&) = delete;
  BasicRpcFramework(BasicRpcFramework&&) = delete;
  BasicRpcFramework& operator =(const BasicRpcFramework&) = delete;
  BasicRpcFramework& operator =(BasicRpcFramework&&) = delete;

  ~BasicRpcFramework() {
    function_footprint();
  }

  idgs::ResultCode initialize() {
    return idgs::RC_SUCCESS;
  }

  inline void shutdown() {
    threadModel.shutdown();
    networkModel.shutdown();

    DVLOG(2) << "RPC Framework is shutdown";
  }

  THREADMODEL* getThreadModel() {
    return &threadModel;
  }

  ACTORFRAME* getActorManager() {
    return &actorFramework;
  }

  NETWORKMODEL* getNetwork() {
    return &networkModel;
  }

  ScheduledMessageService& getScheduler() {
    return service;
  }

private:
  THREADMODEL threadModel;
  NETWORKMODEL networkModel;
  ACTORFRAME actorFramework;
  ScheduledMessageService service;
};
typedef BasicRpcFramework<idgs::net::NetworkModelAsio, StlThreadModel, ActorManager> RpcFramework;

int sendMessage(ActorMessagePtr msg);
int postMessage(ActorMessagePtr msg);

} // namespace actor
} // namespace idgs
