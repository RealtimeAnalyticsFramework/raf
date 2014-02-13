
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/cluster/cluster_framework.h"
#include "admin_node.h"
#include <vector>

namespace idgs {
namespace admin {
namespace cluster {

static const std::string MODULE_NAME = "cluster";

// Membership attributes
static const std::string ALL_MEMBERS_ID =
    "membership_table.all_members_info";
static const std::string MEMBER_INFO = "membership_table.member_info";
static const std::string MEMBER_ID_PARAM = "member_id";

class MembershipManagableNode: public idgs::admin::ManageableNode {
  public:
    MembershipManagableNode() {
      moduleName = MODULE_NAME;
    }

    ~MembershipManagableNode() {
      attributesPath.clear();
    }

    bool init();

  private:
    bool processAllMembersInfoReq(OperationContext& context);

    bool processMemberInfoReq(OperationContext& context);
};

////////////// Partition table attributes ///////////////////////////////////////
static const std::string PARTITION_TABLE = "partition_table";

class PartionManagableNode: public idgs::admin::ManageableNode {
  public:
    PartionManagableNode() {
      moduleName = MODULE_NAME;
    }

    ~PartionManagableNode() {
      attributesPath.clear();
    }

    bool init();

  private:
    bool processGetPartitionTableReq(OperationContext& context);
};

/////////////// Actor framework attributes ///////////////////////////////////
static const std::string ALL_STATEFUL_ACTORS = "actors.stateful_actors";
static const std::string ALL_STATELESS_ACTORS = "actors.stateless_actors";
static const std::string ACTOR = "actors.actor";

static const std::string ACTOR_ID_PARAM = "actor_id";

class ActorframeworkNode: public idgs::admin::ManageableNode {
  public:
    ActorframeworkNode() {
      moduleName = MODULE_NAME;
    }

    ~ActorframeworkNode() {
      attributesPath.clear();
    }

    bool init();

  private:
    bool processActorsReq(OperationContext& context);
    std::string statefulActos2Json(idgs::actor::StatefulActorMap& actorsMap);
};

class ClusterAdmin : public idgs::admin::AdminNode {
  public:
    ClusterAdmin() :
        memberShip(new MembershipManagableNode),
        partitionMgr(new PartionManagableNode),
        actorNode(new ActorframeworkNode){
    }

    ~ClusterAdmin() {
    }

    bool init();

  private:

    MembershipManagableNode* memberShip;
    PartionManagableNode* partitionMgr;
    ActorframeworkNode* actorNode;
};
}
}
}

