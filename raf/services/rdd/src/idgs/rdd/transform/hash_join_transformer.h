
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "transformer.h"
#include "idgs/store/data_map.h"

namespace idgs {
namespace rdd {
namespace transform {

/// Transformer of RDD.
/// To handle transform which would do join of two or more stores.
class HashJoinTransformer: public Transformer {
public:
  HashJoinTransformer();
  virtual ~HashJoinTransformer();

  virtual const std::string& getName() override {
    return HASH_JOIN_TRANSFORMER;
  }


  /// Copy constructor, called by containers.
  /// This should be singleton, copy constructor should be deleted.
  HashJoinTransformer(const HashJoinTransformer& other) = delete;
  HashJoinTransformer(HashJoinTransformer&& other) = delete;
  HashJoinTransformer& operator()(const HashJoinTransformer& other) = delete;
  HashJoinTransformer& operator()(HashJoinTransformer&& other) = delete;

  /// @brief  Handle transform and save result.
  /// @param  msg     The message from client when create RDD.
  /// @param  input   One or more Input RDDs for each partition.
  /// @param  output  Output RDD for each partition, to save transform result.
  /// @return Status code of result.
  pb::RddResultCode transform(const idgs::actor::ActorMessagePtr& msg, const std::vector<BaseRddPartition*>& input,
      RddPartition* output);

private:
  pb::RddResultCode handleLeftJoin(const std::vector<BaseRddPartition*>& input, RddPartition* output);
  pb::RddResultCode handleInnerJoin(const std::vector<BaseRddPartition*>& input, RddPartition* output);
  pb::RddResultCode handleOuterJoin(const std::vector<BaseRddPartition*>& input, RddPartition* output);

};

} // namespace transform
} // namespace rdd
} // namespace idgs 
