/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#pragma once

#include "idgs/admin/pb/admin_svc.pb.h"
#include "idgs/client/client_pool.h"
#include "idgs/admin/cluster_admin.h"
#include "idgs/admin/rdd_admin.h"

namespace idgs {
namespace client {
namespace admin {

enum Operation {
  GET = 0
};

enum Module {
  Cluster = 0, RDD = 1
};

static const std::string Module_name[] = { "cluster", "rdd" };
static const std::string Operation_name[] = { "get" };
static const std::string ADMIN_ACTOR_ID = "admin_service";

class ModuleRequest {
public:
  virtual ~ModuleRequest() {
    attributes.clear();
  }

  virtual Module getModuleName() = 0;

  std::vector<std::string>& getAllAttributes();

protected:
  std::vector<std::string> attributes;
};

class RddModuleRequest: public ModuleRequest {
public:
  RddModuleRequest() {
  }

  Module getModuleName() {
    return RDD;
  }

  void addRddDependencyAttr(std::string rddName);
  void addDependingRddInfoAttr(std::string rddName);
};

class ClusterModuleRequest: public ModuleRequest {
public:
  ClusterModuleRequest() {
  }

  Module getModuleName() {
    return Cluster;
  }

  void addAllMembersInfoAttr();
  void addOneMemberInfoAttr(std::string memberId);
};

class AdminUtil {
public:
  AdminUtil() {
  }
  ~AdminUtil() {
  }

  static idgs::client::ClientActorMessagePtr createAdminRequest(std::shared_ptr<ModuleRequest> module,
      Operation operation);

  static bool getAttributeResponse(idgs::admin::pb::AdminResponse& adminResponse, Module moduleName, std::string attr,
      idgs::admin::pb::AttributeResponse& attrResponse);

  static bool getAttributeResponse(idgs::admin::pb::AdminResponse& adminResponse, int index,
      idgs::admin::pb::AttributeResponse& attrResponse);
};

}
}
}
