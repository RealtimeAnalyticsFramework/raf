
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "admin_service_actor.h"
#include "idgs/admin/admin_server.h"
#include "idgs/actor/rpc_framework.h"

using namespace idgs::admin::pb;
using namespace idgs::actor;
using namespace std;


namespace idgs {
namespace admin {
namespace actor {
  idgs::actor::ActorDescriptorPtr AdminServiceActor::generateActorDescriptor() {
    static std::shared_ptr<ActorDescriptorWrapper> descriptor;
    if (descriptor)
      return descriptor;
    descriptor.reset(new ::ActorDescriptorWrapper);

    descriptor->setName(ADMIN_ACTOR_ID);
    descriptor->setDescription(ADMIN_ACTOR_DESCRIPTION);
    descriptor->setType(::idgs::pb::AT_STATELESS);

    // in operation
    ActorOperationDescriporWrapper admin_get_request;
    admin_get_request.setName(ADMIN_GET_REQUEST);
    admin_get_request.setDescription("get request");
    admin_get_request.setPayloadType(ADMIN_GET_PAYLOAD_TYPE);
    descriptor->setInOperation(admin_get_request.getName(), admin_get_request);

    return descriptor;
  }

  void AdminServiceActor::handAdminGetRequest(
      const idgs::actor::ActorMessagePtr& actorMsg) {
    idgs::admin::AdminServer& admin = ::idgs::util::singleton<
        idgs::admin::AdminServer>::getInstance();

    AdminRequest* payload =
        dynamic_cast<AdminRequest*>(actorMsg->getPayload().get());

    int op_size = payload->module_op_request_size();
    for (int i = 0; i < op_size; i++) {
      const ::idgs::admin::pb::ModuleOpRequest& moduleOp = payload->module_op_request(i);
      const string module_name = moduleOp.module_name();
      int attributes_size = moduleOp.attributes_size();
      for (int i = 0; i < attributes_size; i++) {
        const ::idgs::admin::pb::AttributeRequest& attrReq = moduleOp.attributes(i);
        idgs::admin::AttributePathPtr attrPath(new idgs::admin::AttributePath());
        const string& attrString = attrReq.attribute();
        if (!attrPath->parse(attrString)) {
          LOG(ERROR)<< "can not parse attribute: " << attrString;
          std::shared_ptr<ActorMessage> resposne = actorMsg->createResponse();
          shared_ptr<AttributeResponse> attrResponse(new AttributeResponse);
          attrResponse->set_status(::idgs::admin::pb::Error);
          attrResponse->set_module_name(module_name);
          attrResponse->set_attribute(attrString);
          attrResponse->set_value("can not parse attribute string");
          resposne->setPayload(attrResponse);
          idgs::actor::sendMessage(resposne);
          continue;
        }

        ManageableNode* node = admin.getManageableNode(module_name, attrPath->getAttributePath());
        if (node == NULL) {
          LOG(ERROR)<< "can not process attribute: " << attrString;
          std::shared_ptr<ActorMessage> resposne = actorMsg->createResponse();
          shared_ptr<AttributeResponse> attrResponse(new AttributeResponse);
          attrResponse->set_status(::idgs::admin::pb::Error);
          attrResponse->set_module_name(module_name);
          attrResponse->set_attribute(attrString);
          attrResponse->set_value("can not process attribute");
          resposne->setPayload(attrResponse);
          idgs::actor::sendMessage(resposne);
          continue;
        }

        OperationContext context;
        context.module_name = module_name;
        context.attr = attrPath;
        context.actorMsg = actorMsg;
        if (!node->processRequest(context)) {
          LOG(ERROR) << "failed to process attribute: "<< attrString;
        }
      }
    }
  }

  void AdminServiceActor::init() {
    this->descriptor = generateActorDescriptor();
  }

}
}
}
