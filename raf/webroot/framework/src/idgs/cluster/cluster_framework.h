
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/cluster/cluster_cfg_parser.h"
#include "idgs/cluster/partition_manager_actor.h"
#include "idgs/cluster/adapter/corosync_cluster_adapter.h"
#include "idgs/cluster/adapter/standalone_cluster_adapter.h"
#include "idgs/cluster/cluster_const.h"

#include "idgs/actor/actor_descriptor_mgr.h"
#include "idgs/actor/rpc_framework.h"

namespace idgs {
namespace cluster {

// front declaration
class PartitionManagerActor;

typedef std::shared_ptr<pb::ClusterConfig> ClusterCfgPtr;

/**
 * Cluster = CorosyncClusterEngine + MemberManager + PartitionManager
 */
template<class CLUSTERADAPTER, class MEMBERMANAGER = MemberManagerActor, class PARTITIONMANAGER = PartitionManagerActor>
class BasicClusterFramework {
public:
  BasicClusterFramework() {
  }

  ~BasicClusterFramework() {
    function_footprint();
  }

  BasicClusterFramework(const BasicClusterFramework&) = delete;
  BasicClusterFramework(BasicClusterFramework&&) = delete;
  BasicClusterFramework& operator ()(const BasicClusterFramework&) = delete;
  BasicClusterFramework& operator ()(BasicClusterFramework&&) = delete;

  CLUSTERADAPTER* getClusterAdapter() { return &clusterAdapter;}

  MEMBERMANAGER* getMemberManager() { return &memberMgr; }

  PARTITIONMANAGER* getPartitionManager() { return &partitionMgr;}

  ResultCode init() {
    function_footprint();
    ResultCode rc = RC_OK;
    rc = clusterAdapter.init(getClusterConfig());
    if (rc != RC_SUCCESS) {
      return RC_CLUSTER_ERR_CLUSTER_INIT;
    }
    /// memberManager initialize
    memberMgr.init(getClusterConfig());

    /// partition manager initialize
    partitionMgr.init(getClusterConfig());
    partitionMgr.setMembershipTableManager(memberMgr);

    /// partition manager register member event listener
    memberMgr.addListener(&partitionMgr);

    return rc;
  }

  idgs::ResultCode loadCfgFile(const char* cfgFile) {
    function_footprint();
    if (!clusterConfig.get()) {
      clusterConfig = std::make_shared<pb::ClusterConfig>();
    }
    return ClusterCfgParser::parse(*clusterConfig, cfgFile);
  }

  idgs::pb::ClusterConfig* getClusterConfig() const { return clusterConfig.get();}

  const idgs::pb::Member& getMemberConfig() const { return getClusterConfig()->member();}

  size_t getPartitionCount() const {
    if (getClusterConfig() == NULL) {
      return 0;
    }
    return getClusterConfig()->partition_count();
  }

  const MemberWrapper* getLocalMember() const {
    return memberMgr.getLocalMember();
  }

  ResultCode start() { return clusterAdapter.start(); }

  void terminate() {
    function_footprint();
    partitionMgr.terminate();
    memberMgr.terminate();
  }

private:
  CLUSTERADAPTER clusterAdapter;
  MEMBERMANAGER memberMgr;
  PARTITIONMANAGER partitionMgr;
  ClusterCfgPtr clusterConfig;
}; /// end class

typedef BasicClusterFramework<StandaloneClusterAdapter, MemberManagerActor, PartitionManagerActor> StandaloneClusterFramework;

#if defined(WITH_COROSYNC)
typedef BasicClusterFramework<CorosyncClusterAdapter, MemberManagerActor, PartitionManagerActor> ClusterFramework;
#else
typedef BasicClusterFramework<StandaloneClusterAdapter, MemberManagerActor, PartitionManagerActor> ClusterFramework;
#endif
} // end namespace cluster
} // end namespace idgs
