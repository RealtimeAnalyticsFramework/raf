/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#pragma once

#include <memory>
#include "idgs/pb/rpc_message.pb.h"
#include "protobuf/pb_serdes.h"

namespace idgs {
namespace client {
typedef std::shared_ptr<idgs::pb::RpcMessage> RpcMessagePtr;
typedef std::shared_ptr<google::protobuf::Message> PbMessagePtr;

class ClientActorMessage {
public:

  ClientActorMessage();

  ClientActorMessage(const RpcMessagePtr& msg);

  ClientActorMessage(const ClientActorMessage& other) = default;

  ClientActorMessage(ClientActorMessage&& other) = default;

  ClientActorMessage& operator =(const ClientActorMessage& other) = default;

  ClientActorMessage& operator =(ClientActorMessage&& other) = default;

  virtual ~ClientActorMessage();

  /// get rpc message
  /// @deprecated
  const RpcMessagePtr& getRpcMessage() const {
    return rpcMessage;
  }

  /// get payload
  std::shared_ptr<google::protobuf::Message> getPayload() {
    return payload;
  }

  /// set payload
  void setPayload(std::shared_ptr<google::protobuf::Message> data) {
    payload = data;
  }

  void setOperationName(const std::string& op_name) {
    rpcMessage->set_operation_name(op_name);
  }

  const std::string& getOperationName() const {
    return rpcMessage->operation_name();
  }

  int32_t getSourceMemberId() const {
    return rpcMessage->source_actor().member_id();
  }

  void setSourceMemberId(int32_t memberId) {
    rpcMessage->mutable_source_actor()->set_member_id(memberId);
  }

  const std::string& getSourceActorId() {
    return rpcMessage->mutable_source_actor()->actor_id();
  }

  void setSourceActorId(const std::string& actorId) {
    rpcMessage->mutable_source_actor()->set_actor_id(actorId);
  }

  int32_t getDestMemberId() {
    return rpcMessage->mutable_dest_actor()->member_id();
  }

  void setDestMemberId(int32_t memberId) {
    rpcMessage->mutable_dest_actor()->set_member_id(memberId);
  }

  const std::string& getDestActorId() {
    return rpcMessage->mutable_dest_actor()->actor_id();
  }

  void setDestActorId(const std::string& actorId) {
    rpcMessage->mutable_dest_actor()->set_actor_id(actorId);
  }

  void setChannel(idgs::pb::TransportChannel value) {
    rpcMessage->set_channel(value);
  }

  const idgs::pb::TransportChannel getChannel() {
    return rpcMessage->channel();
  }

  void setSerdesType(const idgs::pb::PayloadSerdes& mode) {
    rpcMessage->set_serdes_type((idgs::pb::PayloadSerdes) ((int32_t) mode));
  }

  const idgs::pb::PayloadSerdes getSerdesType() const {
    return rpcMessage->serdes_type();
  }

  bool parseProto(const std::string& protoString, google::protobuf::Message* msg);

  bool parsePayload(google::protobuf::Message* msg);

  bool parseAttachment(const std::string& name, google::protobuf::Message* msg);

  /// encode the message to byte array.
  std::string toBuffer();

  /// dump actor message in human readable format.
  std::string toString();

  std::shared_ptr<google::protobuf::Message> getAttachement(const std::string & name);
  std::multimap<std::string, PbMessagePtr>& getAttachements() {
    return attachments;
  }

  void setAttachment(const std::string & name, std::shared_ptr<google::protobuf::Message> value);

  std::multimap<std::string, std::string>& getRawAttachments() {
    return rawAttachments;
  }
protected:
  RpcMessagePtr rpcMessage;
  PbMessagePtr payload;
  std::multimap<std::string, PbMessagePtr> attachments;
  std::multimap<std::string, std::string> rawAttachments;
};

typedef std::shared_ptr<ClientActorMessage> ClientActorMessagePtr;
typedef std::map<std::string, std::function<void(ClientActorMessagePtr& msg)> > ClientActorOperationMap;
} // namespace client
} // namespace idgs

