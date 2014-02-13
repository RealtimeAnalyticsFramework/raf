
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <memory>
#include "idgs/pb/rpc_message.pb.h"
#include "idgs/net/rpc_buffer.h"
#include "protobuf/pb_serdes.h"

namespace idgs {
namespace actor {
typedef std::shared_ptr<google::protobuf::Message> PbMessagePtr;
class ActorMessage {
public:
  ActorMessage();
  ActorMessage(const ActorMessage& other);
  ActorMessage(ActorMessage&& other) = default;
  ActorMessage& operator =(const ActorMessage& other) = default;
  ActorMessage& operator =(ActorMessage&& other) = default;

  ActorMessage(std::shared_ptr<idgs::pb::RpcMessage> msg);

  ~ActorMessage();

public:
  std::shared_ptr<idgs::pb::RpcMessage>& getRpcMessage() {
    if (!rpcMessage) {
      rpcMessage = std::make_shared<idgs::pb::RpcMessage>();
    }
    return rpcMessage;
  }

  std::shared_ptr<google::protobuf::Message>& getPayload() {
    return payload;
  }

  void setPayload(std::shared_ptr<google::protobuf::Message> data) {
    payload = data;
  }

  int32_t getSourceMemberId() {
    return getRpcMessage()->source_actor().member_id();
  }

  void setOperationName(const std::string& op_name) {
    getRpcMessage()->set_operation_name(op_name);
  }

  const std::string& getOperationName() {
    return getRpcMessage()->operation_name();
  }

  void setSourceMemberId(int32_t memberId) {
    getRpcMessage()->mutable_source_actor()->set_member_id(memberId);
  }

  const std::string& getSourceActorId() {
    return getRpcMessage()->mutable_source_actor()->actor_id();
  }

  void setSourceActorId(const std::string& actorId) {
    getRpcMessage()->mutable_source_actor()->set_actor_id(actorId);
  }

  int32_t getDestMemberId() {
    return getRpcMessage()->mutable_dest_actor()->member_id();
  }

  void setDestMemberId(int32_t memberId) {
    getRpcMessage()->mutable_dest_actor()->set_member_id(memberId);
  }

  const std::string& getDestActorId() {
    return getRpcMessage()->mutable_dest_actor()->actor_id();
  }

  void setDestActorId(const std::string& actorId) {
    getRpcMessage()->mutable_dest_actor()->set_actor_id(actorId);
  }

  void setChannel(idgs::pb::TransportChannel value) {
    getRpcMessage()->set_channel(value);
  }

  idgs::pb::TransportChannel getChannel() {
    return getRpcMessage()->channel();
  }

  void setSerdesType(const idgs::pb::PayloadSerdes& mode) {
    getRpcMessage()->set_serdes_type((idgs::pb::PayloadSerdes) ((int32_t) mode));
  }

  const idgs::pb::PayloadSerdes getSerdesType() {
    return getRpcMessage()->serdes_type();
  }
  enum MessageOrietation {
    UDP_ORIENTED, OUTER_TCP, INNER_TCP, APP_ORIENTED,
  };

  enum MessageOrietation getMessageOrietation() const {
    return orientation;
  }

  void setMessageOrietation(MessageOrietation o) {
    orientation = o;
  }

  const std::string& getTcpActorId() const {
    return tcpActorId;
  }

  void setTcpActorId(const std::string& id) {
    tcpActorId = id;
  }

  uint32_t getResendCount() const {
    return resendCount;
  }

  void setResendCount(uint32_t count) {
    resendCount = count;
  }

public:
  bool parsePayload(google::protobuf::Message* msg) {
    if (!msg) {
      return false;
    }

    protobuf::SerdesMode mode = static_cast<protobuf::SerdesMode>(getRpcMessage()->serdes_type());
    return protobuf::ProtoSerdesHelper::deserialize(mode, getRpcMessage()->payload(), msg);
  }

  bool parseProto(const std::string& protoString, google::protobuf::Message* msg) {
    if (!msg) {
      return false;
    }

    protobuf::SerdesMode mode = static_cast<protobuf::SerdesMode>(getRpcMessage()->serdes_type());
    return protobuf::ProtoSerdesHelper::deserialize(mode, protoString, msg);
  }

  bool parseAttachment(const std::string& name, google::protobuf::Message* msg) {
    if (!msg) {
      return false;
    }

    auto it = attachments.find(name);
    if (it != attachments.end()) {
      msg->MergeFrom(*it->second);
      return true;
    }

    auto rawit = rawAttachments.find(name);
    if (rawit == rawAttachments.end()) {
      return false;
    }

    protobuf::SerdesMode mode = static_cast<protobuf::SerdesMode>(getRpcMessage()->serdes_type());
    bool result = protobuf::ProtoSerdesHelper::deserialize(mode, rawit->second, msg);
    return result;
  }

  bool parseAttachment1(const std::string& name, PbMessagePtr& msg);

  idgs::ResultCode parseRpcBuffer();

  idgs::ResultCode toRpcBuffer();

  idgs::net::RpcBuffer* getRpcBuffer() {
    return rpcBuffer;
  }

  void setRpcBuffer(idgs::net::RpcBuffer* buff) {
    this->rpcBuffer = buff;
  }

  /// called after toRpcMessage
  void freePbMemory();

public:
  /// dump actor message in human readable format.
  std::string toString();

  /// create a response message, and source and destination are swapped.
  std::shared_ptr<ActorMessage> createResponse();

  /// create a new request in the same session as
  std::shared_ptr<ActorMessage> createSessionRequest();

  /// create the route message, createRouteMessage in tcp actor
  /// @param  destMemId the dest member id
  /// @param  destActorId the dest actor id
  /// @return route message
  std::shared_ptr<ActorMessage> createRouteMessage(int32_t destMemId, std::string destActorId);

  /// create the multicast message
  /// @return multicast message
  std::shared_ptr<ActorMessage> createMulticastMessage();

  std::shared_ptr<google::protobuf::Message> getAttachement(const std::string & name);
  void setAttachment(const std::string & name, std::shared_ptr<google::protobuf::Message> value);

  std::multimap<std::string, PbMessagePtr>& getAttachments() {
    return attachments;
  }

  std::multimap<std::string, std::string>& getRawAttachments() {
    return rawAttachments;
  }

  void freeRpcBuffer();
private:
  void extractRawAttachments();
private:
  std::shared_ptr<idgs::pb::RpcMessage> rpcMessage;
  std::shared_ptr<google::protobuf::Message> payload;

  std::multimap<std::string, PbMessagePtr> attachments;
  std::multimap<std::string, std::string> rawAttachments;

  idgs::net::RpcBuffer* rpcBuffer;

  enum MessageOrietation orientation = APP_ORIENTED;
  std::string tcpActorId;
  uint32_t resendCount = 0;
};
typedef std::shared_ptr<ActorMessage> ActorMessagePtr;
typedef std::map<std::string, std::function<void(ActorMessagePtr& msg)> > ActorOperationMap;

} // namespace rpc
} // namespace idgs

