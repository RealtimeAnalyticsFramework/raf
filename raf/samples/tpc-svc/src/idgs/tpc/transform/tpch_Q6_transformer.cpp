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

namespace idgs {
namespace tpc {
namespace transform {

TpchQ6Transformer::TpchQ6Transformer() {
}

TpchQ6Transformer::~TpchQ6Transformer() {
}

RddResultCode TpchQ6Transformer::transform(const ActorMessagePtr& msg, const std::vector<BaseRddPartition*>& input,
    RddPartition* output) {
  if (input.size() != 1) {
    return RRC_INVALID_RDD_INPUT;
  }

  if (input[0]->empty()) {
    return RRC_SUCCESS;
  }

  auto valueTemplate = input[0]->getValueTemplate();
  auto descriptor = valueTemplate->GetDescriptor();
  auto reflection = valueTemplate->GetReflection();

  auto dateField = descriptor->FindFieldByName("l_shipdate");
  auto discountField = descriptor->FindFieldByName("l_discount");
  auto quantityField = descriptor->FindFieldByName("l_quantity");

  input[0]->foreach(
      [output, reflection, dateField, discountField, quantityField] (const PbMessagePtr& key, const PbMessagePtr& value) {
        double discount = 0, quantity = 0;
        string date = reflection->GetString(* value, dateField);
        discount = reflection->GetDouble(* value, discountField);
        quantity = reflection->GetDouble(* value, quantityField);

        if (date >= "1994-01-01" && date < "1995-01-01" && discount >= 0.05 && discount <= 0.07 && quantity < 24) {
          output->putLocal(key, value);
        }
      });

  return RRC_SUCCESS;
}

} /* namespace transform */
} /* namespace tpc */
} /* namespace idgs */
