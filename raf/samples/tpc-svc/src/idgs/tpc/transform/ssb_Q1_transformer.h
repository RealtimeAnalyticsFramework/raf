/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#pragma once

#include "idgs/rdd/transform/transformer.h"
#include "idgs/tpc/tpc_svc_const.h"


namespace idgs {
namespace tpc {
namespace transform {

class SsbQ1_1Transformer: public idgs::rdd::transform::Transformer {
public:
  SsbQ1_1Transformer();
  virtual ~SsbQ1_1Transformer();

  virtual const std::string& getName() override {
    return SSB_Q1_1_TRANSFORMER;
  }


  SsbQ1_1Transformer(const SsbQ1_1Transformer& other) = delete;
  SsbQ1_1Transformer(SsbQ1_1Transformer&& other) = delete;
  SsbQ1_1Transformer& operator()(const SsbQ1_1Transformer& other) = delete;
  SsbQ1_1Transformer& operator()(SsbQ1_1Transformer&& other) = delete;

  idgs::rdd::pb::RddResultCode transform(const idgs::actor::ActorMessagePtr& msg,
      const std::vector<idgs::rdd::BaseRddPartition*>& input, idgs::rdd::RddPartition* output);
};

} /* namespace transform */
} /* namespace rdd */
} /* namespace idgs */
