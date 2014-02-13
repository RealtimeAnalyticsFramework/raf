//
///*
//Copyright (c) <2013>, Intel Corporation All Rights Reserved.
//
//The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.
//
//Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
//*/
//#pragma once
//
//#include <functional>
//#include "transformer.h"
//#include "reduce_operation.h"
//#include "idgs/rdd/pb/rdd_transform.pb.h"
//#include <sstream>
//
//namespace idgs {
//namespace rdd {
//namespace transform {
//
//class ParallelReduceByKeyTransformer: public Transformer {
//public:
//
//  ParallelReduceByKeyTransformer();
//
//  ~ParallelReduceByKeyTransformer();
//
//  void initReducerMap() {
//
//  }
//
//  virtual const std::string& getName() override {
//    return PARALLEL_REDUCEVALUE_TRANSFORMER;
//  }
//
//  pb::RddResultCode transform(const idgs::actor::ActorMessagePtr& msg, const std::vector<BaseRddPartition*>& input,
//      RddPartition* output);
//
//private:
//
//  pb::RddResultCode parseOperations(const idgs::actor::ActorMessagePtr& msg, const BaseRddPartition* input,
//      RddPartition* output, std::vector<reduce::ParallelReduceOperation>& operations);
//
//  void handleReduceResult(std::vector<reduce::ParallelReduceOperation>& operations,
//      const idgs::actor::PbMessagePtr& key, RddPartition* output, bool reuse_key);
//
//  std::vector<reduce::ParallelReduceOperation> merge(const std::vector<reduce::ParallelReduceOperation>& x,
//      const std::vector<reduce::ParallelReduceOperation>& y);
//};
//
//} // namespace transform
//} // namespace rdd
//} // namespace idgs
