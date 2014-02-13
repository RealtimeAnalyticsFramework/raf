
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "gtest/gtest.h"

#include "idgs/store/data_store.h"
#include "idgs/expr/parsed_field_extractor.h"

#include "protobuf/message_helper.h"

using namespace std;
using namespace protobuf;
using namespace idgs::expr;
using namespace idgs::store;

TEST(parsed_field, evaluate) {
  idgs::util::singleton<DataStore>::getInstance().loadCfgFile("services/store/test/data_store.conf");

  string msgType = "idgs.sample.tpch.pb.LineItem";
  string field_name = "l_quantity";
  double l_quantity = 30;

  auto message = idgs::util::singleton<MessageHelper>::getInstance().createMessage(msgType);
  message->GetReflection()->SetDouble(message.get(), message->GetDescriptor()->FindFieldByName(field_name), l_quantity);

  idgs::pb::Expr expr;
  expr.set_type(idgs::pb::FIELD);
  expr.set_value(field_name);

  ParsedFieldExtractor extractor;
  extractor.parse(expr, message, message);

  ExpressionContext ctx;
  ctx.setKeyValue(&message, &message);
  PbVariant var = extractor.evaluate(&ctx);

  EXPECT_EQ(l_quantity, (double) var);
}
