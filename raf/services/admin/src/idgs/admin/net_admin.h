
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/net/network_statistics.h"
#include "idgs/net/inner_tcp_server.h"
#include "admin_node.h"
#include <vector>

namespace idgs {
namespace admin {
namespace net{
static const std::string SINGLE_NETWORK_STATISTICS =
    "network_statistics";
static const std::string MEMBER_ID_PARAM = "member_id";
static const std::string MODULE_NAME = "net";

struct NetworkStatisticsCore {
  static bool processSingleNetworkStaticReq(idgs::admin::AttributePathPtr& attr,
      std::string& json, size_t& target_member_id);
};

class NetworkStatisticsNode: public idgs::admin::ManageableNode {
  public:
    NetworkStatisticsNode() {
      moduleName = MODULE_NAME;
    }

    ~NetworkStatisticsNode() {
      attributesPath.clear();
    }

    bool init();

  private:
    bool processSingleNetworkStaticReq(OperationContext& context);

};

class NetAdmin : public idgs::admin::AdminNode {
  public:
    NetAdmin() :
      netStat(new NetworkStatisticsNode) {
    }

    ~NetAdmin() {
      delete netStat;
    }

    bool init();

  private:
    NetworkStatisticsNode* netStat;
};
}
}
}
