/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#if defined(__GNUC__) || defined(__clang__)
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "idgs/client/client_actor_message.h"

using namespace idgs::pb;

namespace idgs {

namespace client {

ClientActorMessage::ClientActorMessage() :
    rpcMessage(new RpcMessage) {

}

ClientActorMessage::ClientActorMessage(const RpcMessagePtr& msg) :
    rpcMessage(msg) {
  assert(msg.get());
  int size = msg->attachments_size();
  rpcMessage->payload();
  for (int i = 0; i < size; ++i) {
    auto a = msg->attachments(i);
    rawAttachments.insert(std::pair<std::string, std::string>(a.name(), a.value()));
  }
}

ClientActorMessage::~ClientActorMessage() {

}

/// encode the message to byte array.
std::string ClientActorMessage::toBuffer() {
  // encode payload
  protobuf::SerdesMode mode = (protobuf::SerdesMode) ((int32_t) rpcMessage->serdes_type());
  if (payload.get()) {
    if (!protobuf::ProtoSerdesHelper::serialize(mode, payload.get(), rpcMessage->mutable_payload())) {
      LOG(ERROR)<< "parse actor message payload error! " << payload->DebugString();
      return "";
    }
  }

      // encode attachments
  for (auto it = attachments.begin(); it != attachments.end(); ++it) {
    auto a = rpcMessage->add_attachments();
    a->set_name(it->first);
    protobuf::ProtoSerdesHelper::serialize(mode, it->second.get(), a->mutable_value());

    // @todo remove
//        if (rawAttachments.find(it->first) == rawAttachments.end()) {
//          protobuf::ProtoSerdesHelper::serialize(mode, it->second.get(), &rawAttachments[it->first]);
//        }
  }

  std::string s;
  google::protobuf::TextFormat::PrintToString(*rpcMessage.get(), &s);
  DVLOG(3) << "==============to buffer==============";
  DVLOG(3) << s;
  DVLOG(3) << "============//to buffer==============";

  // encode whole message
  std::string result;
  if (!protobuf::ProtoSerdes<DEFAULT_PB_SERDES>::serialize(rpcMessage.get(), &result)) {
    LOG(ERROR)<< "parse actor message error! " << rpcMessage->DebugString();
    return "";
  }
  return result;
}

bool ClientActorMessage::parseProto(const std::string& protoString, google::protobuf::Message* msg) {
  if (!msg) {
    return false;
  }

  protobuf::SerdesMode mode = static_cast<protobuf::SerdesMode>(rpcMessage->serdes_type());
  return protobuf::ProtoSerdesHelper::deserialize(mode, protoString, msg);
}

bool ClientActorMessage::parsePayload(google::protobuf::Message* msg) {
  if (!msg) {
    return false;
  }

  protobuf::SerdesMode mode = static_cast<protobuf::SerdesMode>(rpcMessage->serdes_type());
  return protobuf::ProtoSerdesHelper::deserialize(mode, rpcMessage->payload(), msg);
}

bool ClientActorMessage::parseAttachment(const std::string& name, google::protobuf::Message* msg) {
  if (!msg) {
    LOG(ERROR)<< "message is NULL";
    return false;
  }

  auto it = attachments.find(name);
  if (it != attachments.end()) {
    msg->CopyFrom(*it->second);
    return true;
  }

  protobuf::SerdesMode mode = static_cast<protobuf::SerdesMode>(getRpcMessage()->serdes_type());
  auto range = rawAttachments.equal_range(name);
  if (range.first == range.second) {
    LOG(ERROR) << "No raw attachment";
    return false;
  }
  for (auto it = range.first; it != range.second; ++it) {
    bool result = protobuf::ProtoSerdesHelper::deserialize(mode, it->second, msg);
    if(!result) {
      LOG(ERROR) << "Failed to deserialize raw attachment: " << mode << ", data: " << it->second;
    } else {
      PbMessagePtr v(msg->New());
      v->CopyFrom(*msg);
      attachments.insert(std::pair<std::string, PbMessagePtr>(name, v));
    }
  }
  return true;
}

/// encode the message to byte array.
std::string ClientActorMessage::toString() {
  std::stringstream ss;
  ss << rpcMessage->DebugString();
  if (payload.get()) {
    ss << payload->DebugString();
  }
  return ss.str();
}

std::shared_ptr<google::protobuf::Message> ClientActorMessage::getAttachement(const std::string & name) {
  auto it = attachments.find(name);
  if (it != attachments.end()) {
    return it->second;
  }
  return std::shared_ptr<google::protobuf::Message>();
}
void ClientActorMessage::setAttachment(const std::string & name, std::shared_ptr<google::protobuf::Message> value) {
  attachments.insert(std::pair<std::string, std::shared_ptr<google::protobuf::Message> >(name, value));

}

} // namespace client
} // namespace idgs

