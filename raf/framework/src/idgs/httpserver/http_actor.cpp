/*
 Copyright (c) <2012>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#include "http_actor.h"
#include "http_connection.h"
#include "idgs/application.h"


namespace idgs {
namespace httpserver {

HttpActor::HttpActor():idgs::actor::StatefulActor() {
  serdes = protobuf::PB_JSON;
}

HttpActor::~HttpActor() {
}

void HttpActor::process(const idgs::actor::ActorMessagePtr& msg) {
  if(handleSystemOperation(msg)) {
    DVLOG(5) << "System message: " << msg->toString();
    return;
  }

  bool success = true;
  do {
    // serialize RpcMessage to payload of response
    // encode payload
    if (msg->getPayload().get() && (!msg->getRpcMessage()->has_payload())) {
      DVLOG(7) << "actor message payload: " << msg->getPayload()->DebugString();
      if (!protobuf::ProtoSerdesHelper::serialize(serdes, msg->getPayload().get(), msg->getRpcMessage()->mutable_payload())) {
        LOG(ERROR)<< "serialize actor message payload error! " << msg->getPayload()->DebugString();
        success = false;
        break;
      }
    }

    // encode attachments
    auto& attachments = msg->getAttachments();
    if (attachments.size() != (size_t)msg->getRpcMessage()->attachments_size()) {
      for (auto it = attachments.begin(); it != attachments.end(); ++it) {
        auto a = msg->getRpcMessage()->add_attachments();
        a->set_name(it->first);
        protobuf::ProtoSerdesHelper::serialize(serdes, it->second.get(), a->mutable_value());
      }
    }

    msg->getRpcMessage()->mutable_dest_actor()->CopyFrom(msg->getRpcMessage()->client_actor());
    msg->getRpcMessage()->clear_client_actor();

    // encode whole message
    std::string buff;
    if (!protobuf::ProtoSerdesHelper::serialize(serdes, msg->getRpcMessage().get(), &buff)) {
      LOG(ERROR)<< "serialize actor message error! " << msg->getRpcMessage()->DebugString();
      success = false;
      break;
    }
    auto& response = asyncContext->getResponse();
    response.setContent(buff);
    response.setStatus(idgs::httpserver::HttpResponse::ok);

    if (serdes == protobuf::PB_JSON) {
      response.addHeader(HTTP_HEADER_CONTENT_TYPE, CONTENT_TYPE_JSON);
    } else if (serdes == protobuf::PB_BINARY) {
      response.addHeader(HTTP_HEADER_CONTENT_TYPE, CONTENT_TYPE_PROTOBUF);
    } else {
      LOG(ERROR) << "error";
    }
  } while(0);

  // send response
  if (!success) {
    DVLOG(2) << "test process";
    asyncContext->getResponse() = HttpResponse::createReply(HttpResponse::internal_server_error);
  }
  asyncContext->done();

  // destroy this actor
  terminate();
}

void HttpActor::service(std::shared_ptr<AsyncContext> asyncContext_) {
  asyncContext = asyncContext_;
  // @todo
  // deserialize payload to RpcMessage
  auto& request = asyncContext->getRequest();
  auto contentType = request.getHeader(HTTP_HEADER_CONTENT_TYPE);

  if (contentType.empty() || contentType.find(CONTENT_TYPE_JSON) != contentType.npos) {
    // JSON
    serdes = protobuf::PB_JSON;
  } else if(contentType.find(CONTENT_TYPE_PROTOBUF) != contentType.npos) {
    // protobuf
    serdes = protobuf::PB_BINARY;
  } else {
    // error, unknown content type
    LOG(ERROR) << "unknown content type: " << contentType;
    asyncContext->getResponse() = HttpResponse::createReply(HttpResponse::bad_request);
    asyncContext->done();
    terminate();
    return;
  }

  std::shared_ptr<idgs::pb::RpcMessage> rpcMsg = std::make_shared<idgs::pb::RpcMessage>();
  bool ret = protobuf::ProtoSerdesHelper::deserialize(serdes, request.getBody(), rpcMsg.get());
  if (!ret) {
    LOG(ERROR) << "Failed to parse: " << request.getBody();
    asyncContext->getResponse() = HttpResponse::createReply(HttpResponse::bad_request);
    asyncContext->done();
    terminate();
  }

  static const int32_t& localMemberId = idgs_application()->getMemberManager()->getLocalMemberId();

  rpcMsg->mutable_client_actor()->set_actor_id(rpcMsg->source_actor().actor_id());
  rpcMsg->mutable_client_actor()->set_member_id(rpcMsg->source_actor().member_id());

  rpcMsg->mutable_source_actor()->set_actor_id(getActorId());
  rpcMsg->mutable_source_actor()->set_member_id(localMemberId);

  std::shared_ptr<idgs::actor::ActorMessage> actorMsg = std::make_shared<idgs::actor::ActorMessage>(rpcMsg);


//  static NetworkStatistics* stats = StatefulTcpActor::outerTcpServer->network->getNetworkStatistics();
//  stats->outerTcpBytesRecv.fetch_add(readBuffer->getBodyLength());
//  stats->outerTcpPacketRecv.fetch_add(1);

  DVLOG(3) << "Http actor '" << getActorId() << "' receive message" << actorMsg->toString() << " end";
  LOG(INFO) << "HTTP request, actor ID: " << rpcMsg->dest_actor().actor_id()
      << ", member ID: " << rpcMsg->dest_actor().member_id()
      << ", operation: " << rpcMsg->operation_name();

  // push message to queue
  idgs::actor::relayMessage(actorMsg);
}


} // namespace httpserver
} // namespace idgs
