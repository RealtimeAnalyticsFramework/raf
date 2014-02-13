
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "google/protobuf/descriptor.h"
#include "idgs/expr/expression_context.h"
#include "idgs/actor/actor_message.h"
#include "idgs/rdd/base_rdd_partition.h"
#include "idgs/rdd/rdd_partition.h"
#include "idgs/rdd/pb/rdd_svc.pb.h"
#include "idgs/rdd/rdd_const.h"

namespace idgs {
namespace rdd {
namespace transform {

/// Transformer of RDD interface class.
/// To handle transformer of each partition and save the result.
class Transformer {
public:
  Transformer();
  virtual ~Transformer();

  virtual const std::string& getName() = 0;

  /// @brief  Handle transform and save result.
  /// @param  msg     The message from client when create RDD.
  /// @param  input   One or more Input RDDs for each partition.
  /// @param  output  Output RDD for each partition, to save transform result.
  /// @return Status code of result.
  virtual pb::RddResultCode transform(const idgs::actor::ActorMessagePtr& msg,
      const std::vector<idgs::rdd::BaseRddPartition*>& input, idgs::rdd::RddPartition* output) = 0;

};
// class Transformer

}// namespace transform
} // namespace rdd
} // namespace idgs 
