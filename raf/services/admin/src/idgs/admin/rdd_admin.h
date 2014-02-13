
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "admin_node.h"
#include "idgs/rdd/rdd_actor.h"
#include "idgs/rdd/rdd_info.h"
#include "idgs/result_code.h"
#include <map>

namespace idgs {
namespace admin {
namespace rdd {
static const std::string RDD_DEPENDENCY = "dependency";
static const std::string RDD_INFO = "rdd_info";
static const std::string RDD_NAME_PARAM = "rdd_name";
static const std::string MODULE_NAME = "rdd";
class RddDependencyNode: public idgs::admin::ManageableNode {
  public:
    RddDependencyNode() {
      moduleName = MODULE_NAME;
    }

    ~RddDependencyNode() {
      attributesPath.clear();
    }

    bool init();

    bool dependingRddInfo2Json(idgs::rdd::RddActor* actor, std::string& json);

    bool rddInfo2Json(idgs::rdd::RddActor* actor, std::string& json);

    bool rddSnapshot2Json(idgs::rdd::RddSnapshot& snapshot, std::string& json);

    bool dependency2Json(idgs::rdd::RddSnapshot& parent, std::map<std::string, std::shared_ptr<idgs::rdd::RddSnapshot>>& childRdds, std::string& json);

    bool getDependency(idgs::rdd::RddSnapshot& rdd, std::map<std::string, std::shared_ptr<idgs::rdd::RddSnapshot>>& childRdds);

  private:
    bool processRddDependencyReq(OperationContext& context);

    bool processRddInfoReq(OperationContext& context);

    idgs::rdd::RddActor* checkMsgAndLoadRddActor(OperationContext& context,
        std::string& rddName);

    idgs::rdd::RddActor* getRddActor(const std::string& rddName,
        OperationContext& context);

    idgs::ResultCode addChildRdd(idgs::rdd::RddSnapshot& rdd, std::map<std::string, std::shared_ptr<idgs::rdd::RddSnapshot>>& childRdds);


  };

class RddAdmin : public idgs::admin::AdminNode {
  public:
    RddAdmin() :
        dNode(new RddDependencyNode) {
    }

    ~RddAdmin() {
    }

    bool init();

  private:
    RddDependencyNode* dNode;
};
}
}
};
