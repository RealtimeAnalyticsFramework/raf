/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // defined(__GNUC__) || defined(__clang__) $
#include "tpch_Q6_transformer.h"

using namespace std;
using namespace idgs::actor;
using namespace idgs::rdd;
using namespace idgs::rdd::pb;
using namespace idgs::rdd::transform;

namespace idgs {
namespace tpc {
namespace transform {

TpchQ6Transformer::TpchQ6Transformer() {
}

TpchQ6Transformer::~TpchQ6Transformer() {
}

RddResultCode TpchQ6Transformer::transform(TransformerContext* ctx, const BaseRddPartition* input, PairRddPartition* output) {
  auto descriptor = input->getValueTemplate()->GetDescriptor();
  auto reflection = input->getValueTemplate()->GetReflection();

  auto fldDate = descriptor->FindFieldByName("l_shipdate");
  auto fldDiscount = descriptor->FindFieldByName("l_discount");
  auto fldQquantity = descriptor->FindFieldByName("l_quantity");

  input->foreach([output, reflection, fldDate, fldDiscount, fldQquantity] (const PbMessagePtr& key, const PbMessagePtr& value) {
    double discount = 0, quantity = 0;
    string date = reflection->GetString(* value, fldDate);
    discount = reflection->GetDouble(* value, fldDiscount);
    quantity = reflection->GetDouble(* value, fldQquantity);

    if (date >= "1994-01-01" && date < "1995-01-01" && discount >= 0.05 && discount <= 0.07 && quantity < 24) {
      output->put(key, value);
    }
  });

  return RRC_SUCCESS;
}

} /* namespace transform */
} /* namespace tpc */
} /* namespace idgs */
