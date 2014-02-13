
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "manageable_node.h"
#include "admin_util.h"


namespace idgs{
namespace admin {
bool ManageableNode::processRequest(OperationContext& context) {
  const std::string& module_name = context.module_name;
  idgs::admin::AttributePathPtr& attr = context.attr;
  idgs::actor::ActorMessagePtr& actorMsg = context.actorMsg;

  assert(attr.get() != NULL && actorMsg.get() != NULL);
  const std::string& operation_name= actorMsg->getOperationName();
  DVLOG(2)<<"will process admin request: module_name " << module_name << ", operation_name " << operation_name
      << ", attribute " << attr->getAttributePath();
  if (module_name != this->getModuleName()) {
    LOG(ERROR) << "can not process admin request with module " << module_name;
    idgs::admin::util::sendAdminErrorResponse(context, "Unknown module name: " + module_name);
    return false;
  }

  auto itr = opMap.find(attr->getAttributePath());
  if(itr != opMap.end()) {
    itr -> second (context);
  } else {
    LOG(ERROR) << "Unknown attribute: " << attr->getAttributePath();
    idgs::admin::util::sendAdminErrorResponse(context, "Unknown attribute: " + attr->getAttributePath());
  }

  return true;
}

bool ManageableNode::checkOperationName(
    OperationContext& context,
    const std::string& operationName) {
  idgs::actor::ActorMessagePtr& actorMsg = context.actorMsg;

  if (actorMsg.get() == NULL) {
    LOG(ERROR)<< "ActorMessage is null ";
    idgs::admin::util::sendAdminErrorResponse(context, "ActorMessage is null ");
    return false;
  }

  const std::string& operation_name = actorMsg->getOperationName();
  if (operation_name != operationName) {
    LOG(ERROR)<< "unsupported operation name: " << operation_name;
    idgs::admin::util::sendAdminErrorResponse(context, "unsupported operation name: " + operation_name);
    return false;
  }

  return true;
}

bool ManageableNode::checkParameterValue(
    OperationContext& context,
    const std::string& key,
    std::string& value) {
  AttributePathPtr& attr = context.attr;
  if (attr.get() == NULL) {
    LOG(ERROR)<< "AttributePath is null ";
    idgs::admin::util::sendAdminErrorResponse(context, "AttributePath is null ");
    return false;
  }
  if (!attr->getParameterValue(key, value)) {
    LOG(ERROR)<< "can not get attribute value for " << key;
    idgs::admin::util::sendAdminErrorResponse(context, "can not get attribute value for " + key);
    return false;
  }

  return true;
}
}
}





