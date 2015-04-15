/*
 Copyright (c) <2012>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#include "actor_stub.h"
#include "idgs/client/client_pool.h"

namespace idgs {
namespace client {

ActorStub::ActorStub() {
}

ActorStub::~ActorStub() {
  if (stateful) {
    destroy();
  }
}

void ActorStub::destroy() {
  idgs::ResultCode code;
  auto client = getTcpClientPool().getTcpClient(code);
  if (code != RC_SUCCESS) {
    LOG(ERROR) << "Error in get client, cause by " << getErrorDescription(code);
  }
  ClientActorMessagePtr clientActorMsg = std::make_shared<ClientActorMessage>();
  clientActorMsg->setOperationName(idgs::actor::OP_DESTROY);
  clientActorMsg->setChannel(idgs::pb::TC_TCP);
  clientActorMsg->setDestActorId(actorId.actor_id());
  clientActorMsg->setDestMemberId(actorId.member_id());
  clientActorMsg->setSourceActorId("client_actor_id");
  clientActorMsg->setSourceMemberId(idgs::pb::CLIENT_MEMBER);

  code = client->send(clientActorMsg);
}

} // namespace client
} // namespace idgs
