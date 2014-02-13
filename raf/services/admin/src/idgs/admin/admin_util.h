
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/actor/actor_message.h"
#include "idgs/admin/pb/admin_svc.pb.h"
#include "idgs/pb/cluster_config.pb.h"
#include "idgs/actor/rpc_framework.h"

#include <memory>
#include <string>
#include <sstream>
#include <vector>

namespace idgs {
namespace admin {
namespace util {
inline static idgs::actor::ActorMessagePtr createAdminResponse(
    OperationContext& context,
    const ::idgs::admin::pb::ResponseStatus status,
    const std::string& value) {
  const std::string& module_name = context.module_name;
  idgs::admin::AttributePathPtr& attr = context.attr;
  idgs::actor::ActorMessagePtr& actorMsg = context.actorMsg;
  const std::string& operation_name= actorMsg->getOperationName();

  std::shared_ptr<idgs::actor::ActorMessage> resposne = actorMsg->createResponse();
  std::shared_ptr<idgs::admin::pb::AdminResponse> adminResp(new idgs::admin::pb::AdminResponse);
  idgs::admin::pb::ModuleOpResponse* mor = adminResp->add_module_op_response();
  mor->set_module_name(module_name);

  idgs::admin::pb::AttributeResponse* _attrResponse = mor->add_attributes();
  _attrResponse->set_status(status);
  _attrResponse->set_module_name(module_name);
  _attrResponse->set_attribute(attr->getFullPath());
  _attrResponse->set_value(value);

  resposne->setPayload(adminResp);
  resposne->setSerdesType(idgs::pb::PB_JSON);
  resposne->setOperationName(operation_name);
  return resposne;
}

inline static void sendAdminErrorResponse(
    OperationContext& context,
    const std::string& reason) {
  std::shared_ptr<idgs::actor::ActorMessage> resposne =
      createAdminResponse(context, ::idgs::admin::pb::ResponseStatus::Error, reason);
  idgs::actor::sendMessage(resposne);
  return;
}

inline static bool toJsonItem(const std::string& key, const std::string& value, std::string& output) {
  output.append("\"" + key + "\"");
  output.append(":");
  output.append("\"" + value + "\"");
  return true;
}

inline static bool toJsonItem(const std::string& key, const int32_t value, std::string& output) {
  std::stringstream ss;
  ss << value;
  const std::string& _s_value = ss.str();
  return toJsonItem(key, _s_value, output);
}

inline static std::string* aggregateJsonItems(std::vector<std::string> items) {
  std::string* aggr_str = new std::string;
  std::vector<std::string>::iterator itr = items.begin();
  aggr_str->append("{");
  for (;itr < items.end();++itr) {
    aggr_str->append(*itr);
    aggr_str->append(",");
  }
  aggr_str->pop_back();
  aggr_str->append("}");
  return aggr_str;
}
}
}
}

#define AGGREGATE_JSON_ITEMS(...) idgs::admin::util::aggregateJsonItems({ __VA_ARGS__ })
