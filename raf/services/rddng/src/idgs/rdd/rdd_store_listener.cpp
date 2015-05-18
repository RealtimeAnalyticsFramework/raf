/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#include "rdd_store_listener.h"


#include "idgs/actor/rpc_framework.h"

#include "idgs/rdd/pb/rdd_internal.pb.h"

namespace idgs {
namespace rdd {

RddStoreListener::RddStoreListener() : rddLocal(NULL) {
}

RddStoreListener::~RddStoreListener() {
}

idgs::store::ListenerResultCode RddStoreListener::insert(idgs::store::ListenerContext* context) {
  if (!rddLocal) {
    LOG(ERROR) << "RDD is not found.";
    return idgs::store::LRC_CONTINUE;
  }

  auto& key = * context->getKey();
  auto& value = * context->getValue();

  if (rddLocal->isReplicatedRdd()) {
    auto partitions = rddLocal->getLocalPartitions();
    auto it = partitions.begin();
    for (; it != partitions.end(); ++ it) {
      auto partitionActor = rddLocal->getPartitionActor(* it);

      idgs::actor::ActorMessagePtr message = std::make_shared<idgs::actor::ActorMessage>();
      message->setOperationName(RDD_STORE_LISTENER);
      message->setSourceActorId(partitionActor.actor_id());
      message->setSourceMemberId(partitionActor.member_id());
      message->setDestActorId(partitionActor.actor_id());
      message->setDestMemberId(partitionActor.member_id());
      message->setPayload(std::make_shared<pb::RddRequest>());
      message->setAttachment(RE_PARTITION_KEY, key);
      message->setAttachment(RE_PARTITION_VALUE, value);
      idgs::actor::postMessage(message);
    }
  } else {
    auto partitionId = context->getPartitionId();
    auto partitionActor = rddLocal->getPartitionActor(partitionId);

    idgs::actor::ActorMessagePtr message = std::make_shared<idgs::actor::ActorMessage>();
    message->setOperationName(RDD_STORE_LISTENER);
    message->setSourceActorId(partitionActor.actor_id());
    message->setSourceMemberId(partitionActor.member_id());
    message->setDestActorId(partitionActor.actor_id());
    message->setDestMemberId(partitionActor.member_id());
    message->setPayload(std::make_shared<pb::RddRequest>());
    message->setAttachment(RE_PARTITION_KEY, key);
    message->setAttachment(RE_PARTITION_VALUE, value);
    idgs::actor::sendMessage(message);
  }

  return idgs::store::LRC_CONTINUE;
}

void RddStoreListener::setRddLocal(const std::shared_ptr<RddLocal>& rddlocal) {
  rddLocal = rddlocal;
}

const std::shared_ptr<RddLocal>& RddStoreListener::getRddLocal() const {
  return rddLocal;
}

} /* namespace rdd */
} /* namespace idgs */
