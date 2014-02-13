
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
#include "idgs/expr/expression_factory.h"
#include "idgs/rdd/db/local_file_connection.h"

using namespace std;
using namespace protobuf;
using namespace idgs::actor;
using namespace idgs::rdd::pb;
using namespace idgs::rdd::db;
using namespace idgs::expr;

namespace idgs {
namespace rdd {
namespace action {

ExportAction::ExportAction() {
  BaseConnection* conn;
  conn = new LocalFileConnection;

  conns.put(conn->name(), conn);

  ExportFunc function =
      [this] (shared_ptr<BaseConnection>& conn, const shared_ptr<ExportActionRequest>& request, const uint32_t& partition) {
        initLocalFileExport(conn, request, partition);
      };

  func.put(conn->name(), function);
}

ExportAction::~ExportAction() {
}

RddResultCode ExportAction::action(const idgs::actor::ActorMessagePtr& msg, const BaseRddPartition* input,
    std::vector<PbVariant>& output) {
  if (input->empty()) {
    return RRC_SUCCESS;
  }

  ActionRequest* payload = dynamic_cast<ActionRequest*>(msg->getPayload().get());
  Expression* filterExpr = NULL;
  if (payload->has_filter()) {
    auto rc = ExpressionFactory::build(&filterExpr, payload->filter(), input->getKeyTemplate(),
        input->getValueTemplate());
    if (idgs::RC_SUCCESS != rc) {
      LOG(ERROR)<< "RDD \"" << input->getRddName() << " partition[" << input->getPartition() << "]" << "parse filter expression error, caused by " << idgs::getErrorDescription(rc);
      return RRC_NOT_SUPPORT;
    }
  }

  shared_ptr<ExportActionRequest> request(new ExportActionRequest);
  if (!msg->parseAttachment(ACTION_PARAM, request.get())) {
    return RRC_INVALID_ACTION_PARAM;
  }

  string type = ExportType_Name(request->type());
  shared_ptr<BaseConnection> conn(conns.get(type)->clone());
  func.get(type)(conn, request, input->getPartition());
  if (conn->connect() != RC_SUCCESS) {
    return RRC_UNKOWN_ERROR;
  }

  ExpressionContext ctx;
  ResultCode code = RC_SUCCESS;
  input->foreach([conn, &code, filterExpr, &ctx] (const PbMessagePtr& key, const PbMessagePtr& value) {
    if (filterExpr) {
      ctx.setKeyValue(&key, & value);
      if (!(bool) filterExpr->evaluate(&ctx)) {
        return;
      }
    }

    auto c = conn->insert(key, value);
    if (c != RC_SUCCESS) {
      code = c;
    }
  });

  if (code == RC_SUCCESS) {
    conn->commit();
  } else {
    conn->rollback();
  }

  return RRC_SUCCESS;
}

RddResultCode ExportAction::aggregate(const idgs::actor::ActorMessagePtr& actionRequest, idgs::actor::ActorMessagePtr& actionResponse, const vector<vector<string>>& input) {
  shared_ptr<ExportActionResult> response(new ExportActionResult);
  actionResponse->setAttachment(ACTION_RESULT, response);

  return RRC_SUCCESS;
}

void ExportAction::initLocalFileExport(shared_ptr<BaseConnection>& conn, const shared_ptr<ExportActionRequest>& request,
    const uint32_t& partition) {
  LocalFileConnection* con = dynamic_cast<LocalFileConnection*>(conn.get());
  con->setFileName(request->file_name());
  con->setPartition(partition);
}

} // namespace action
} // namespace rdd
} // namespace idgs 
