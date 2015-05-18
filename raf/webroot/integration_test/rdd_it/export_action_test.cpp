
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "gtest/gtest.h"
#include "idgs/client/rdd/rdd_client.h"
#include "idgs/rdd/rdd_const.h"
#include "idgs/rdd/pb/rdd_action.pb.h"
#include "idgs/expr/expression_helper.h"

using namespace std;
using namespace idgs::pb;
using namespace idgs::rdd::pb;
using namespace idgs::client::rdd;

namespace idgs {
namespace rdd {
namespace export_test {

RddClient client;

void exportAction() {
  client.init("conf/client.conf");

  string delegateRDD = "line_item_rdd";

  LOG(INFO) << "create store delegate RDD for LineItem";

  DelegateRddRequestPtr delegateRddRequest = std::make_shared<CreateDelegateRddRequest>();
  DelegateRddResponsePtr delegateRddResponse = std::make_shared<CreateDelegateRddResponse>();
  delegateRddRequest->set_schema_name("tpch");
  delegateRddRequest->set_store_name("LineItem");
  delegateRddRequest->set_rdd_name(delegateRDD);

  client.createStoreDelegateRDD(delegateRddRequest, delegateRddResponse);

  ActionRequestPtr actionRequest = std::make_shared<ActionRequest>();
  ActionResponsePtr actionResponse = std::make_shared<ActionResponse>();
  ActionResultPtr actionResult = std::make_shared<ExportActionResult>();

  actionRequest->set_action_id("100000");
  actionRequest->set_rdd_name(delegateRDD);
  actionRequest->set_action_op_name(EXPORT_ACTION);

  MOVE_EXPR(actionRequest->mutable_filter(), AND(GE(FIELD("l_shipdate"), CONST("1994-01-01")),
                                                  LT(FIELD("l_shipdate"), CONST("1995-01-01")),
                                                  GE(FIELD("l_discount"), CONST("0.05", DOUBLE)),
                                                  LE(FIELD("l_discount"), CONST("0.07")),
                                                  LT(FIELD("l_quantity"), CONST("24"))));

  shared_ptr<ExportActionRequest> exportRequest = std::make_shared<ExportActionRequest>();
  exportRequest->set_type("LOCAL_FILE");
  auto param = exportRequest->add_param();
  param->set_name("LOCAL_FILE_NAME");
  param->set_value("line_item_data.tbl");

  AttachMessage attach;
  attach[ACTION_PARAM] = exportRequest;

  LOG(INFO) << "send export action";
  client.sendAction(actionRequest, actionResponse, actionResult, attach);
}

}  // namespace export_action_test
}  // namespace rdd
}  // namespace idgs

TEST(export_action, file) {
  idgs::rdd::export_test::exportAction();
}
