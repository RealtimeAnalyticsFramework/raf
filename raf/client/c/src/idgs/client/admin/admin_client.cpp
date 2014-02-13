
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "idgs_gch.h"
#include "admin_client.h"

using namespace idgs::admin::pb;
using namespace std;

namespace idgs {
namespace client {
namespace admin {
void RddModuleRequest::addRddDependencyAttr(std::string rddName) {
  std::string attr = idgs::admin::rdd::RDD_DEPENDENCY;
  attr.append(";");
  attr.append(idgs::admin::rdd::RDD_NAME_PARAM);
  attr.append("=");
  attr.append(rddName);

  attributes.push_back(attr);
}

void RddModuleRequest::addDependingRddInfoAttr(std::string rddName) {
  std::string attr = idgs::admin::rdd::RDD_INFO;
  attr.append(";");
  attr.append(idgs::admin::rdd::RDD_NAME_PARAM);
  attr.append("=");
  attr.append(rddName);

  attributes.push_back(attr);
}

void ClusterModuleRequest::addAllMembersInfoAttr() {
  std::string attr = idgs::admin::cluster::ALL_MEMBERS_ID;
  attributes.push_back(attr);
}

void ClusterModuleRequest::addOneMemberInfoAttr(std::string memberId) {
  std::string attr = idgs::admin::cluster::MEMBER_INFO;
  attr.append(";");
  attr.append(idgs::admin::cluster::MEMBER_ID_PARAM);
  attr.append("=");
  attr.append(memberId);

  attributes.push_back(attr);
}

std::vector<std::string>& ModuleRequest::getAllAttributes() {
  return attributes;
}

idgs::client::ClientActorMessagePtr AdminUtil::createAdminRequest(std::shared_ptr<ModuleRequest> module,
    Operation operation) {
  std::shared_ptr<idgs::admin::pb::AdminRequest> adminReq(new idgs::admin::pb::AdminRequest);
  idgs::admin::pb::ModuleOpRequest* mop = adminReq->add_module_op_request();
  mop->set_module_name(Module_name[module->getModuleName()]);
  std::vector<std::string>& attrs = module->getAllAttributes();
  for (std::vector<std::string>::iterator itr = attrs.begin(); itr < attrs.end(); ++itr) {
    idgs::admin::pb::AttributeRequest* attr = mop->add_attributes();
    const std::string& tmp = *itr;
    attr->set_attribute(tmp);
  }

  idgs::client::ClientActorMessagePtr clientReq(new idgs::client::ClientActorMessage);
  clientReq->setOperationName(Operation_name[operation]);
  clientReq->setDestActorId(ADMIN_ACTOR_ID);
  clientReq->setSourceActorId("client_actor_id");
  clientReq->setDestMemberId(idgs::pb::ANY_MEMBER);
  clientReq->setChannel(idgs::pb::TC_AUTO);
  clientReq->setSourceMemberId(idgs::pb::CLIENT_MEMBER);
  clientReq->setPayload(adminReq);

  return clientReq;
}

bool AdminUtil::getAttributeResponse(AdminResponse& adminResponse, Module moduleName, string attr,
    AttributeResponse& attrResponse) {
  for (int i = 0; i < adminResponse.module_op_response_size(); i++) {
    const ModuleOpResponse& mor = adminResponse.module_op_response(i);
    if (mor.module_name() == Module_name[moduleName]) {
      for (int j = 0; j < mor.attributes_size(); j++) {
        VLOG(3) << "get attribute: " << mor.attributes(j).attribute();
        if (mor.attributes(j).attribute() == attr) {
          attrResponse = mor.attributes(j);
          return true;
        }
      }
    }
  }
  return false;
}

bool AdminUtil::getAttributeResponse(AdminResponse& adminResponse, int index, AttributeResponse& attrResponse) {
  return true;
}
}
}
}

