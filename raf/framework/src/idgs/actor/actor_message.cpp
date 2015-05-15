
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/actor/actor_message.h"

#include "idgs/application.h"
#include "idgs/util/backtrace.h"

using namespace std;
using namespace idgs::pb;

namespace idgs {
namespace actor {
ActorMessage::ActorMessage() :
    rpcMessage(), payload(), rpcBuffer(NULL), orientation(APP_ORIENTED) {
}

void ActorMessage::extractRawAttachments() {
  assert(rpcMessage.get());
  int size = getRpcMessage()->attachments_size();
  for (int i = 0; i < size; ++i) {
    auto a = getRpcMessage()->attachments(i);
    rawAttachments.insert(std::pair<std::string, std::string>(a.name(), a.value()));
  }
}

ActorMessage::ActorMessage(std::shared_ptr<RpcMessage> msg) :
    rpcMessage(msg), payload(NULL), rpcBuffer(NULL), orientation(APP_ORIENTED) {
  extractRawAttachments();
}

ActorMessage::ActorMessage(const ActorMessage& rhs) :
    payload(), rpcBuffer(), orientation(APP_ORIENTED) {

  rpcMessage = std::make_shared<idgs::pb::RpcMessage> (*((const_cast<ActorMessage&>(rhs)).getRpcMessage()));

  payload = rhs.payload;
  attachments = rhs.attachments;
  extractRawAttachments();
}

ActorMessage::~ActorMessage() {
  freeRpcBuffer();
}

idgs::ResultCode ActorMessage::toRpcBuffer() {
  idgs::ResultCode code = RC_SUCCESS;
  // encode payload
  protobuf::SerdesMode mode = (protobuf::SerdesMode) ((int32_t) getRpcMessage()->serdes_type());
  if (payload.get() && (!rpcMessage->has_payload())) {
    DVLOG(7) << "actor message payload: " << payload->DebugString();
    if (!protobuf::ProtoSerdesHelper::serialize(mode, payload.get(), getRpcMessage()->mutable_payload())) {
      LOG(ERROR)<< "serialize actor message payload error! " << payload->DebugString();
      code = RC_ERROR;
      return code;
    }
  }

  // encode attachments
//  if (attachments.size() != (size_t)rpcMessage->attachments_size()) {
  for (auto it = attachments.begin(); it != attachments.end(); ++it) {
    auto a = getRpcMessage()->add_attachments();
    a->set_name(it->first);
    protobuf::ProtoSerdesHelper::serialize(mode, it->second.get(), a->mutable_value());
  }
//  }

  // encode whole message
  int totalSize = getRpcMessage()->ByteSize();
  freeRpcBuffer();
  rpcBuffer = std::make_shared<idgs::net::RpcBuffer>();
  auto& byteBuffer = rpcBuffer->byteBuffer();
  auto b = idgs::net::ByteBuffer::allocate(totalSize);
  byteBuffer.swap(b);
  rpcBuffer->setBodyLength(totalSize);
  if (!protobuf::ProtoSerdes<DEFAULT_PB_SERDES>::serialize(getRpcMessage().get(), byteBuffer->data(), byteBuffer->capacity())) {
    LOG(ERROR)<< "serialize actor message error! " << getRpcMessage()->DebugString();
    code = RC_ERROR;
    return code;
  }
  DVLOG(5) << "bytebuffer, length: " << byteBuffer->capacity() << ", buffer: " << dumpBinaryBuffer2(byteBuffer->data(), byteBuffer->capacity());
  DVLOG(5) << "rpc buffer, length: " << rpcBuffer->byteBuffer()->capacity() << ", buffer: " << dumpBinaryBuffer2(rpcBuffer->byteBuffer()->data(), rpcBuffer->byteBuffer()->capacity());
  DVLOG(5) << "getrpcbuffer, length: " << getRpcBuffer()->byteBuffer()->capacity() << ", buffer: " << dumpBinaryBuffer2(getRpcBuffer()->byteBuffer()->data(), getRpcBuffer()->byteBuffer()->capacity());
  return code;
}

/// encode the message to byte array.
std::string ActorMessage::toString() {
  std::stringstream ss;
  if (rpcMessage) {
    ss << "RPC Message: " << rpcMessage->DebugString() << endl;
  }
  if (payload) {
    ss << ", Message payload:" << payload->DebugString();
  }
  if (!attachments.empty()) {
    ss << ", Attachemnts: " << endl;
    for (auto it = attachments.begin(); it != attachments.end(); ++it) {
      ss << "{" << it->first << "} = {" << it->second->DebugString() << "}" << endl;
    }
  }
  if (!rawAttachments.empty()) {
    ss << "Raw Attachemnts: " << endl;
    for (auto it = rawAttachments.begin(); it != rawAttachments.end(); ++it) {
      ss << "{" << dumpBinaryBuffer(it->first) << "} = {" << dumpBinaryBuffer(it->second) << "}" << endl;
    }
  }
  return ss.str();
}

std::shared_ptr<ActorMessage> ActorMessage::createResponse() {
  std::shared_ptr<ActorMessage> resp = std::make_shared<ActorMessage>();
  RpcMessage* newRpc = resp->getRpcMessage().get();
  RpcMessage* myRpc = getRpcMessage().get();

  newRpc->mutable_dest_actor()->CopyFrom(myRpc->source_actor());
  newRpc->mutable_source_actor()->set_member_id(
      myRpc->dest_actor().member_id() < 0 ?
          idgs_application()->getMemberManager()->getLocalMemberId() :
          myRpc->dest_actor().member_id());
  newRpc->mutable_source_actor()->set_actor_id(myRpc->dest_actor().actor_id());

  if (myRpc->has_channel()) {
    newRpc->set_channel(myRpc->channel());
  }

  if (myRpc->has_client_actor()) {
    newRpc->mutable_client_actor()->CopyFrom(myRpc->client_actor());
  }

  //      newRpc->set_message_id(myRpc->message_id());

  return resp;
}

std::shared_ptr<ActorMessage> ActorMessage::createSessionRequest() {
  std::shared_ptr<ActorMessage> req = std::make_shared<ActorMessage>();
  RpcMessage* newRpc = req->getRpcMessage().get();
  RpcMessage* myRpc = getRpcMessage().get();

  newRpc->mutable_dest_actor()->CopyFrom(myRpc->dest_actor());
  newRpc->mutable_source_actor()->set_member_id(
      myRpc->source_actor().member_id() < 0 ?
          idgs_application()->getMemberManager()->getLocalMemberId() :
          myRpc->source_actor().member_id());
  newRpc->mutable_source_actor()->set_actor_id(myRpc->source_actor().actor_id());
  if (myRpc->has_channel()) {
    newRpc->set_channel(myRpc->channel());
  }
  if (myRpc->has_client_actor()) {
    newRpc->mutable_client_actor()->CopyFrom(myRpc->client_actor());
  }

  return req;
}

std::shared_ptr<ActorMessage> ActorMessage::createRouteMessage(int32_t destMemId, std::string destActorId, const bool& reloadPayload) {
  std::shared_ptr<ActorMessage> route = std::make_shared < ActorMessage > (*this);
  RpcMessage* newRpc = route->getRpcMessage().get();

  //set the dest member id
  newRpc->mutable_dest_actor()->set_member_id(destMemId);
  //set the dest actor id
  newRpc->mutable_dest_actor()->set_actor_id(destActorId);

  if (reloadPayload && newRpc->has_payload()) {
    newRpc->clear_payload();
  }

  DVLOG(7) << "the route rpc message = " << route->toString();
  return route;
}

std::shared_ptr<ActorMessage> ActorMessage::createMulticastMessage() {
  int32_t sourceMemberId = idgs_application()->getMemberManager()->getLocalMemberId();
  std::shared_ptr<ActorMessage> multMsg = std::make_shared < ActorMessage > (*this);

  RpcMessage* rpcMsg = multMsg->getRpcMessage().get();
  rpcMsg->mutable_source_actor()->set_member_id(sourceMemberId);
  rpcMsg->mutable_dest_actor()->set_member_id(ALL_MEMBERS);
  rpcMsg->set_channel(TC_MULTICAST);
  multMsg->setPayload(getPayload());

  return multMsg;
}

std::shared_ptr<google::protobuf::Message> ActorMessage::getAttachement(const std::string & name) {
  auto it = attachments.find(name);
  if (it != attachments.end()) {
    return it->second;
  }
  return std::shared_ptr<google::protobuf::Message>();
}

void ActorMessage::setAttachment(const std::string & name, std::shared_ptr<google::protobuf::Message> value) {
  attachments.insert(std::pair<std::string, std::shared_ptr<google::protobuf::Message> >(name, value));
}

void ActorMessage::setAttachment(const std::string & name, std::string & value) {
  rawAttachments.insert(std::pair<std::string, std::string>(name, value));
  auto attachment = getRpcMessage()->add_attachments();
  attachment->set_name(name);
  attachment->set_value(value);
}

void ActorMessage::replaceAttachment(const std::string & name, std::shared_ptr<google::protobuf::Message> value) {
  auto it = attachments.find(name);
  if (it != attachments.end()) {
    attachments.erase(it);
  }
  attachments.insert(std::pair<std::string, std::shared_ptr<google::protobuf::Message> >(name, value));
}

bool ActorMessage::parseAttachment1(const std::string& name, PbMessagePtr& msg) {
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
    LOG(ERROR) << "No raw attachment for name: " << name;
    return false;
  }
  for (auto it = range.first; it != range.second; ++it) {
    PbMessagePtr v(msg->New());
    bool result = protobuf::ProtoSerdesHelper::deserialize(mode, it->second, v.get());
    if(result) {
      attachments.insert(std::pair<std::string, PbMessagePtr>(name, v));
    } else {
      LOG(ERROR) << "Failed to deserialize raw attachment: " << mode << ", data: " << it->second;
    }
  }

  return true;
}

bool ActorMessage::parseAttachments(const std::string& keyName, const std::string& valueName, std::map<PbMessagePtr, PbMessagePtr>& map) {
  PbMessagePtr key;
  PbMessagePtr value;
  for (auto it = attachments.begin(); it != attachments.end(); ++it) {
    if (it->first == keyName) {
      key->MergeFrom(*it->second);
      
    } else {
      continue;
    }
    if (it != attachments.end()) {
      ++it;
    } else {
      break;
    }
    if (it->first == valueName) {
      value->MergeFrom(*it->second);
    } else {
      continue;
    }
    map[key] = value;
  }
  
  if (map.size() == 0) {
    bool result;
    protobuf::SerdesMode mode = static_cast<protobuf::SerdesMode>(getRpcMessage()->serdes_type());

    for (auto rawit = rawAttachments.begin(); rawit != rawAttachments.end(); ++rawit) {
      if (rawit->first == keyName) {
        result = protobuf::ProtoSerdesHelper::deserialize(mode, rawit->second, key.get());
        if (result == false) {
          continue;
        }
      } else {
        continue;
      }
      if (rawit != rawAttachments.end()) {
        ++rawit;
      } else {
        break;
      }
      if (rawit->first == valueName) {
        result = protobuf::ProtoSerdesHelper::deserialize(mode, rawit->second, value.get());
        if (result == false) {
          continue;
        }
      } else {
        continue;
      }
      map[key] = value;
    }
    if (map.size() == 0) {
      return false;
    }
  }

  return true;
}

idgs::ResultCode ActorMessage::parseRpcBuffer() {
  if (!rpcBuffer) {
    DVLOG(3) << "RPC buffer is NULL";
    return RC_ERROR;
  }

  auto& byteBuffer = getRpcBuffer()->byteBuffer();
  if (!byteBuffer) {
    DVLOG(3) << "bytebuffer is NULL";
    return RC_ERROR;
  } else {
    DVLOG(5) << "Bodylength: " << rpcBuffer->getBodyLength()
        << ", bytebuffer length: " <<  byteBuffer->capacity();
//        << ", buffer: " << dumpBinaryBuffer2(byteBuffer->data(), byteBuffer->capacity());
  }
  auto ret = protobuf::ProtoSerdes<DEFAULT_PB_SERDES>::deserializeFromArray(
      byteBuffer->data(),
      byteBuffer->capacity(), getRpcMessage().get());
  if (!ret) {
    LOG(ERROR) << "Failed to deserialize message, length: " <<  byteBuffer->capacity()
        << ", buffer: " << dumpBinaryBuffer2(byteBuffer->data(), byteBuffer->capacity())
        << ", stack: " << idgs::util::stacktrace();
    return RC_ERROR;
  }

  extractRawAttachments();

  setMessageOrietation(ActorMessage::APP_ORIENTED);
  DVLOG_FIRST_N(2, 20) << "the RpcMessage = " << getRpcMessage()->DebugString();

  freeRpcBuffer();

  return RC_OK;
}

void ActorMessage::freeRpcBuffer() {
  rpcBuffer.reset();
}
void ActorMessage::freePbMemory() {
  rpcMessage.reset();
  payload.reset();
  attachments.clear();
  rawAttachments.clear();
}

void relayMessage(ActorMessagePtr& msg) {
  static ActorMessageQueue* q = idgs_application()->getActorMessageQueue();
  q->push(msg);
}

void relayTimerMessage(ActorMessagePtr& msg) {
  static ActorMessageQueue* q = idgs_application()->getActorMessageQueue();
  q->push(msg);
}

} // namespace rpc
} // namespace idgs

