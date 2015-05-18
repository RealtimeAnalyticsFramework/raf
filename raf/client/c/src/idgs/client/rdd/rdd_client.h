/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#pragma once

#include "idgs/rdd/pb/rdd_svc.pb.h"
#include "idgs/store/store.h"

namespace idgs {
namespace client {
namespace rdd {

typedef std::shared_ptr<idgs::rdd::pb::CreateDelegateRddRequest> DelegateRddRequestPtr;
typedef std::shared_ptr<idgs::rdd::pb::CreateRddRequest> RddRequestPtr;
typedef std::shared_ptr<idgs::rdd::pb::CreateDelegateRddResponse> DelegateRddResponsePtr;
typedef std::shared_ptr<idgs::rdd::pb::CreateRddResponse> RddResponsePtr;
typedef std::shared_ptr<idgs::rdd::pb::ActionRequest> ActionRequestPtr;
typedef std::shared_ptr<idgs::rdd::pb::ActionResponse> ActionResponsePtr;
typedef std::shared_ptr<google::protobuf::Message> ActionResultPtr;
typedef std::map<std::string, std::shared_ptr<google::protobuf::Message>> AttachMessage;

class RddClient {

public:
  /// create store delegate RDD
  idgs::ResultCode createStoreDelegateRDD(const DelegateRddRequestPtr& request, DelegateRddResponsePtr& response,
      int time_out = 20);

  /// create RDD
  idgs::ResultCode createRdd(const RddRequestPtr& request, RddResponsePtr& response, const AttachMessage& attach =
      AttachMessage(), int time_out = 20);

  // send action with rdd name
  idgs::ResultCode sendAction(const ActionRequestPtr& request, ActionResponsePtr& response, ActionResultPtr& result,
      const AttachMessage& attach = AttachMessage(), int time_out = 20);

  // send action with rdd actor id
  idgs::ResultCode sendAction(const ActionRequestPtr& request, ActionResponsePtr& response, ActionResultPtr& result,
      const idgs::pb::ActorId& rddId, const AttachMessage& attach = AttachMessage(), int time_out = 20);

  idgs::ResultCode init(const std::string& clientConfig = "");

  const idgs::store::StoreConfigWrapperPtr& getStoreConfig(const std::string& storeName) const;
};
}
}
}
