
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "http_request.h"
#include "http_response.h"
#include "idgs/net/stateful_tcp_actor.h"

namespace idgs{
namespace http {
namespace server {
static const std::string GET = "GET";
static const std::string DELETE = "DELETE";
static const std::string HEAD = "HEAD";
static const std::string OPTIONS = "OPTIONS";
static const std::string POST = "POST";
static const std::string PUT = "PUT";

class HttpServlet {
  public:
    virtual ~HttpServlet() {}
    virtual void init() = 0;
    virtual std::string& getName() = 0;
    virtual void service(HttpRequest& req, HttpResponse& rep);
    virtual void doGet(HttpRequest& req, HttpResponse& rep);
    virtual void doDelete(HttpRequest& req, HttpResponse& rep);
    virtual void doHead(HttpRequest& req, HttpResponse& rep);
    virtual void doOptions(HttpRequest& req, HttpResponse& rep);
    virtual void doPost(HttpRequest& req, HttpResponse& rep);
    virtual void doPut(HttpRequest& req, HttpResponse& rep);
  protected:
    void notSupportedAction(HttpRequest& req, HttpResponse& rep);
};

class HttpAsyncServlet : public idgs::actor::StatefulActor {
  public:

    HttpAsyncServlet();
    virtual ~HttpAsyncServlet();
    virtual void init() = 0;
    virtual std::string& getName() = 0;
    virtual void service(HttpRequest& req);
    virtual void doGet(HttpRequest& req);
    virtual void doDelete(HttpRequest& req);
    virtual void doHead(HttpRequest& req);
    virtual void doOptions(HttpRequest& req);
    virtual void doPost(HttpRequest& req);
    virtual void doPut(HttpRequest& req);

    virtual const idgs::actor::ActorDescriptorPtr& getDescriptor() const {
      static idgs::actor::ActorDescriptorPtr nullDesc(NULL);
      return nullDesc;
    }

    const std::string& getActorName() const override{
      static const std::string actorName("HttpAsyncServlet");
      return actorName;
    }

    virtual const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override {
      static std::map<std::string, idgs::actor::ActorMessageHandler> handlerMap = {
          {"*", static_cast<idgs::actor::ActorMessageHandler>(&HttpAsyncServlet::actorMessageHandler)}
      };
      return handlerMap;
    }

    template<class HttpAsyncMsgHandler>
    void registerHandler(HttpAsyncMsgHandler _handler) {
      handler = _handler;
    }
  protected:
    void notSupportedAction(HttpRequest& req);
    void actorMessageHandler(const idgs::actor::ActorMessagePtr& msg);
    std::function<void (const HttpResponse& msg)> handler;
};


typedef std::shared_ptr<idgs::http::server::HttpServlet> HttpServletPtr;
typedef std::shared_ptr<idgs::http::server::HttpAsyncServlet> HttpAsyncServletPtr;

}
}
}
