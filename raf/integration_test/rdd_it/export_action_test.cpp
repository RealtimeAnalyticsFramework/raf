
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "gtest/gtest.h"
#include "idgs/client/rdd/rdd_client.h"
#include "idgs/rdd/rdd_const.h"
#include "idgs/rdd/pb/rdd_action.pb.h"
#include "idgs/store/data_map.h"

using namespace std;
using namespace idgs::pb;
using namespace idgs::util;
using namespace idgs::rdd;
using namespace idgs::rdd::pb;
using namespace idgs::client::rdd;

void exportAction() {
  singleton<RddClient>::getInstance().init("integration_test/rdd_it/client.conf");

  string delegateRDD = "line_item_rdd";

  LOG(INFO) << "create store delegate RDD for LineItem";

  DelegateRddRequestPtr delegateRddRequest(new CreateDelegateRddRequest);
  DelegateRddResponsePtr delegateRddResponse(new CreateDelegateRddResponse);
  delegateRddRequest->set_store_name("LineItem");
  delegateRddRequest->set_rdd_name(delegateRDD);

  singleton<RddClient>::getInstance().createStoreDelegateRDD(delegateRddRequest, delegateRddResponse);

  ActionRequestPtr actionRequest(new ActionRequest);
  ActionResponsePtr actionResponse(new ActionResponse);
  ActionResultPtr actionResult(new ExportActionResult);

  actionRequest->set_action_id("100000");
  actionRequest->set_rdd_name(delegateRDD);
  actionRequest->set_action_op_name(EXPORT_ACTION);
  auto filterExpr = actionRequest->mutable_filter();

  filterExpr->set_type(AND);

  // >= 1994-01-01
  Expr* exp1 = filterExpr->add_expression();
  exp1->set_type(GE);

  Expr* elem1 = exp1->add_expression();
  elem1->set_type(FIELD);
  elem1->set_value("l_shipdate");

  Expr* elem2 = exp1->add_expression();
  elem2->set_type(CONST);
  elem2->set_const_type(STRING);
  elem2->set_value("1994-01-01");

  // < 1995-01-01
  Expr* exp2 = filterExpr->add_expression();
  exp2->set_type(LT);

  elem1 = exp2->add_expression();
  elem1->set_type(FIELD);
  elem1->set_value("l_shipdate");

  elem2 = exp2->add_expression();
  elem2->set_type(CONST);
  elem2->set_const_type(STRING);
  elem2->set_value("1995-01-01");

  // discount >= 0.05
  Expr* exp3 = filterExpr->add_expression();
  exp3->set_type(GE);

  elem1 = exp3->add_expression();
  elem1->set_type(FIELD);
  elem1->set_value("l_discount");

  elem2 = exp3->add_expression();
  elem2->set_type(CONST);
  elem2->set_const_type(DOUBLE);
  elem2->set_value("0.05");

  // discount <= 0.07
  auto exp4 = filterExpr->add_expression();
  exp4->set_type(LE);
  elem1 = exp4->add_expression();
  elem1->set_type(FIELD);
  elem1->set_value("l_discount");

  elem2 = exp4->add_expression();
  elem2->set_type(CONST);
  elem2->set_const_type(DOUBLE);
  elem2->set_value("0.07");

  // quantity < 24
  auto emp5 = filterExpr->add_expression();
  emp5->set_type(LT);
  elem1 = emp5->add_expression();
  elem1->set_type(FIELD);
  elem1->set_value("l_quantity");

  elem2 = emp5->add_expression();
  elem2->set_type(CONST);
  elem2->set_const_type(DOUBLE);
  elem2->set_value("24");

  shared_ptr<ExportActionRequest> exportRequest(new ExportActionRequest);
  exportRequest->set_type(LOCAL_FILE);
  exportRequest->set_file_name("line_item_data.tbl");

  AttachMessage attach;
  attach[ACTION_PARAM] = exportRequest;

  LOG(INFO) << "send export action";
  singleton<RddClient>::getInstance().sendAction(actionRequest, actionResponse, actionResult, attach);
}

TEST(export_action, file) {
  exportAction();
}
