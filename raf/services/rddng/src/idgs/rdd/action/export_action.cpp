
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "export_action.h"

#include "idgs/rdd/rdd_const.h"
#include "idgs/rdd/db/local_file_connection.h"

using namespace std;
using namespace idgs::actor;
using namespace idgs::rdd::pb;
using namespace idgs::rdd::db;

namespace idgs {
namespace rdd {
namespace action {

ExportAction::ExportAction() {
  BaseConnectionPtr conn = make_shared<LocalFileConnection>();
  conns.put(conn->name(), conn);
}

ExportAction::~ExportAction() {
}

BaseConnectionPtr ExportAction::getConnection(const std::string& name) {
  BaseConnectionPtr connection;
  auto& conn = conns.get(name);
  if (conn) {
    connection.reset(conn->clone());
  }

  return connection;
}

RddResultCode ExportAction::action(ActionContext* ctx, const BaseRddPartition* input) {
  if (!input->empty()) {
    ExportActionRequest request;
    if (!ctx->getActionParam(ACTION_PARAM, &request)) {
      return RRC_INVALID_ACTION_PARAM;
    }

    auto conn = getConnection(request.type());
    if (!conn) {
      return RRC_NOT_SUPPORT;
    }

    map<string, string> params;
    params.insert(pair<string, string>("PARTITION_ID", to_string(input->getPartition())));
    for (int32_t i = 0; i < request.param_size(); ++ i) {
      auto& param = request.param(i);
      params.insert(pair<string, string>(param.name(), param.value()));
    }

    conn->init(params);

    if (conn->connect() != RC_SUCCESS) {
      return RRC_INVALID_ACTION_PARAM;
    }

    ResultCode code = RC_SUCCESS;
    input->foreach([&conn, &ctx, &code] (const PbMessagePtr& key, const PbMessagePtr& value) {
      ctx->setKeyValue(&key, & value);
      if (ctx->evaluateFilterExpr()) {
        auto rc = conn->insert(key, value);
        if (rc != RC_SUCCESS) {
          code = rc;
        }
      }
    });

    if (code == RC_SUCCESS) {
      conn->commit();
    } else {
      conn->rollback();
    }
  }

  return RRC_SUCCESS;
}

RddResultCode ExportAction::aggregate(ActionContext* ctx) {
  ctx->setActionResult(make_shared<ExportActionResult>());
  return RRC_SUCCESS;
}

} // namespace action
} // namespace rdd
} // namespace idgs 
