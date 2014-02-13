/*
Copyright (c) <2012>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__)
#include "idgs_gch.h"
#endif // GNUC_ $

#include "http_servlet.h"
#include "idgs/application.h"

namespace idgs{
namespace http {
namespace server {
void HttpServlet::service(HttpRequest& req, HttpResponse& rep) {
  if (req.getMethod() == GET){
    doGet(req,rep);
  } else if (req.getMethod() == DELETE){
    doDelete(req,rep);
  } else if (req.getMethod() == HEAD){
    doHead(req,rep);
  } else if (req.getMethod() == OPTIONS){
    doOptions(req,rep);
  } else if (req.getMethod() == POST){
    doPost(req,rep);
  } else if (req.getMethod() == PUT){
    doPut(req,rep);
  } else {
    notSupportedAction(req, rep);
  }
}

void HttpServlet::doGet(HttpRequest& req, HttpResponse& rep) {
  notSupportedAction(req, rep);
}

void HttpServlet::doDelete(HttpRequest& req, HttpResponse& rep) {
  notSupportedAction(req, rep);
}

void HttpServlet::doHead(HttpRequest& req, HttpResponse& rep) {
  notSupportedAction(req, rep);
}

void HttpServlet::doOptions(HttpRequest& req, HttpResponse& rep) {
  notSupportedAction(req, rep);
}

void HttpServlet::doPost(HttpRequest& req, HttpResponse& rep) {
  notSupportedAction(req, rep);
}

void HttpServlet::doPut(HttpRequest& req, HttpResponse& rep) {
  notSupportedAction(req, rep);
}

void HttpServlet::notSupportedAction(HttpRequest& req, HttpResponse& rep) {
  std::string con = "method " + req.getMethod() + " not supported";
  rep = HttpResponse::createReply(HttpResponse::not_implemented, con);
}

////////////////////////// HttpAsyncServlet ///////////////////////////////////

HttpAsyncServlet::HttpAsyncServlet():handler(NULL) {
  VLOG(2) << "HttpAsyncServlet " << this->getActorId() << " is created";
  idgs::util::singleton<idgs::actor::RpcFramework>::getInstance().getActorFramework()->Register(getActorId(), this);
}

HttpAsyncServlet::~HttpAsyncServlet() {
  VLOG(2) << "HttpAsyncServlet " << this->getActorName() << "::" << this->getActorId() << " is stopped";
  idgs::util::singleton<idgs::actor::RpcFramework>::getInstance().getActorFramework()->unRegisterStatefulActor(getActorId());
}

void HttpAsyncServlet::service(HttpRequest& req) {
  if (req.getMethod() == GET){
    doGet(req);
  } else if (req.getMethod() == DELETE){
    doDelete(req);
  } else if (req.getMethod() == HEAD){
    doHead(req);
  } else if (req.getMethod() == OPTIONS){
    doOptions(req);
  } else if (req.getMethod() == POST){
    doPost(req);
  } else if (req.getMethod() == PUT){
    doPut(req);
  } else {
    notSupportedAction(req);
  }
}

void HttpAsyncServlet::doGet(HttpRequest& req) {
  notSupportedAction(req);
}

void HttpAsyncServlet::doDelete(HttpRequest& req) {
  notSupportedAction(req);
}

void HttpAsyncServlet::doHead(HttpRequest& req) {
  notSupportedAction(req);
}

void HttpAsyncServlet::doOptions(HttpRequest& req) {
  notSupportedAction(req);
}

void HttpAsyncServlet::doPost(HttpRequest& req) {
  notSupportedAction(req);
}

void HttpAsyncServlet::doPut(HttpRequest& req) {
  notSupportedAction(req);
}

void HttpAsyncServlet::actorMessageHandler(const idgs::actor::ActorMessagePtr& msg) {
  VLOG(3) << "get actor response: " << msg->toString();
  std::stringstream ss;
  std::string payload = "";
  if (msg->getPayload().get()) {
    payload = protobuf::JsonMessage::toPrettyJsonString(msg->getPayload().get());
  } else {
    if(msg->getRpcMessage()->has_payload() && msg->getSerdesType() == idgs::pb::PayloadSerdes::PB_JSON) {
      const google::protobuf::FieldDescriptor* fd = msg->getRpcMessage()->GetDescriptor()->FindFieldByName("payload");
      payload = msg->getRpcMessage()->GetReflection()->GetString(*msg->getRpcMessage(), fd);
      payload = protobuf::JsonMessage::toIndentJsonString(payload);
    }
  }

  ss << payload << std::endl;

  VLOG(3) << "get payload: " << ss.str();

  std::multimap<std::string, idgs::actor::PbMessagePtr>& attachments = msg->getAttachments();
  if (!attachments.empty()) {
    for (auto it = attachments.begin(); it != attachments.end(); ++it) {
      ss << "{" << it->first << "} = {" << protobuf::JsonMessage::toPrettyJsonString(it->second.get()) << "}" << std::endl;
    }
  }

  HttpResponse rep = HttpResponse::createReply(HttpResponse::ok, ss.str());
  handler(rep);
}

void HttpAsyncServlet::notSupportedAction(HttpRequest& req) {
  HttpResponse rep = HttpResponse::createReply(HttpResponse::not_implemented);
  handler(rep);
}



} // idgs
} // http
} // server
