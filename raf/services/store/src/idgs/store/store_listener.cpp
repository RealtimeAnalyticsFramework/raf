/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__)
#include "idgs_gch.h"
#endif // GNUC_ $

#include "store_listener.h"

#include "idgs/actor/actor_message.h"

#include "idgs/cluster/cluster_framework.h"

#include "idgs/store/datastore_const.h"
#include "idgs/store/pb/store_service.pb.h"

#include "idgs/util/singleton.h"

#include "protobuf/message_helper.h"

using namespace std;
using namespace idgs::actor;
using namespace idgs::cluster;
using namespace idgs::store::pb;
using namespace idgs::util;
using namespace google::protobuf;

namespace idgs {
namespace store {

StoreListener::StoreListener() : listenerIndex(0) {
}

StoreListener::~StoreListener() {
}

void StoreListener::sendNextListener(const idgs::actor::ActorMessagePtr& msg, const uint32_t& destMemberId) {
  ClusterFramework& cluster = singleton<ClusterFramework>::getInstance();
  auto localMemberId = cluster.getMemberManager()->getLocalMemberId();

  ActorMessagePtr message = msg->createRouteMessage(destMemberId, LISTENER_MANAGER);

  shared_ptr<StoreListenerInfo> listener = make_shared<StoreListenerInfo>();
  listener->set_listener_index(listenerIndex + 1);
  message->setAttachment(STORE_ATTACH_LISTENER, listener);

  idgs::actor::sendMessage(message);
}

void StoreListener::sendBreakBranch(const idgs::actor::ActorMessagePtr& msg, const uint32_t& destMemberId) {
  ClusterFramework& cluster = singleton<ClusterFramework>::getInstance();
  auto localMemberId = cluster.getMemberManager()->getLocalMemberId();

  ActorMessagePtr message = msg->createRouteMessage(destMemberId, LISTENER_MANAGER);

  shared_ptr<StoreListenerInfo> listener = make_shared<StoreListenerInfo>();
  listener->set_listener_index(listenerIndex);
  message->setAttachment(STORE_ATTACH_LISTENER, listener);

  idgs::actor::sendMessage(message);
}

bool StoreListener::getAttachmentData(const idgs::actor::ActorMessagePtr& msg, idgs::actor::PbMessagePtr& key, idgs::actor::PbMessagePtr& value) {
  protobuf::MessageHelper& helper = idgs::util::singleton<protobuf::MessageHelper>::getInstance();
  const string& keyType = storeConfig->getStoreConfig().key_type();
  key = msg->getAttachement(STORE_ATTACH_KEY);
  if (!key) {
    key = helper.createMessage(keyType);
    if (!key) {
      LOG(ERROR) << "key type " << keyType << " is not registered.";
      return false;
    }

    if (!msg->parseAttachment(STORE_ATTACH_KEY, key.get())) {
      LOG(ERROR) << "invalid key with type " << keyType;
      return false;
    }
  }

  const string& valueType = storeConfig->getStoreConfig().value_type();
  value = msg->getAttachement(STORE_ATTACH_VALUE);
  if (!value) {
    value = helper.createMessage(valueType);
    if (!key) {
      LOG(ERROR) << "value type " << valueType << " is not registered.";
      return false;
    }

    if (!msg->parseAttachment(STORE_ATTACH_VALUE, value.get())) {
      LOG(ERROR) << "invalid value with type " << valueType;
      return false;
    }
  }

  return true;
}

} /* namespace store */
} /* namespace idgs */
