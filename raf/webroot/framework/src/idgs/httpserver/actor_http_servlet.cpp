/*
 Copyright (c) <2012>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#include "actor_http_servlet.h"
#include "idgs/application.h"
#include "http_actor.h"

namespace idgs {
namespace httpserver {

ActorHttpServlet::ActorHttpServlet() {
}

ActorHttpServlet::~ActorHttpServlet() {
}

void ActorHttpServlet::doGet(HttpRequest& request, HttpResponse& response){
  doPost(request, response);
}

void ActorHttpServlet::doPost(HttpRequest& request, HttpResponse& response){
  static idgs::actor::ActorManager* af = idgs_application()->getRpcFramework()->getActorManager();

  auto httpActor = new HttpActor();
  af->registerSessionActor(httpActor->getActorId(), httpActor);

  auto asyncContext = request.startAsync();
  httpActor->service(asyncContext);
}

} // namespace httpserver
} // namespace idgs
