
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/cluster/cluster_cfg_parser.h"
#include "partitiontable_mgr.h"
#include "idgs/cluster/corosync_adapter.h"
#include "idgs/actor/actor_descriptor_mgr.h"
#include "cluster_const.h"
#include "idgs/actor/rpc_framework.h"

namespace idgs {
namespace cluster {

// front declaration
class PartitionTableMgr;

/**
 * Cluster = CorosyncClusterEngine + MemberManager + PartitionManager
 */
template<class CLUSTERADAPTER, class MEMBERMANAGER = MembershipTableMgr,
    class PARTITIONMANAGER = PartitionTableMgr>
class BasicClusterFramework {
public:
  BasicClusterFramework() :
      initialized(false) {

  }


  ~BasicClusterFramework() {
    function_footprint();

    if (!initialized) {
      return;
    }
  }


  BasicClusterFramework(const BasicClusterFramework&) = delete;
  BasicClusterFramework(BasicClusterFramework&&) = delete;
  BasicClusterFramework& operator ()(const BasicClusterFramework&) = delete;
  BasicClusterFramework& operator ()(BasicClusterFramework&&) = delete;

  CLUSTERADAPTER* getClusterAdapter() {
    return &clusterAdapter;
  }

  MEMBERMANAGER* getMemberManager() {
    return &memberMgr;
  }

  PARTITIONMANAGER* getPartitionManager() {
    return &partitionMgr;
  }

  ResultCode init() {
    function_footprint();

    if (!initialized) {

      ResultCode rs = clusterAdapter.init(getClusterConfig());
      if (rs != RC_SUCCESS) {
        return RC_CLUSTER_ERR_CLUSTER_INIT;
      }

      // memberManager init
      memberMgr.init(getClusterConfig());

      // partitionManager init
      partitionMgr.init(getClusterConfig());
      partitionMgr.setMembershipTableManager(memberMgr);

      // partitionManager register member listener
      memberMgr.addListener(&partitionMgr);

      // register
      registerModuleDescriptor();

      // set init flag
      initialized = true;
    } else {
      DVLOG(2) << "Cluster has already been initialized";
    }
    return RC_SUCCESS;
  }

  idgs::ResultCode loadCfgFile(const char* cfgFile) {
    function_footprint();

    if (!clusterConfig.get()) {
      clusterConfig.reset(new pb::ClusterConfig);
    }
    return ClusterCfgParser::parse(*clusterConfig.get(), cfgFile);
  }

  pb::ClusterConfig* getClusterConfig() const {
    return clusterConfig.get();
  }

  const pb::Member& getMemberConfig() const {
    return getClusterConfig()->member();
  }

  size_t getPartitionCount() const {
    if (getClusterConfig() == NULL) {
      return 0;
    }
    return getClusterConfig()->partition_count();
  }

  const MemberWrapper* getLocalMember() const {
    return memberMgr.getLocalMember();
  }

  ResultCode start() {
    return clusterAdapter.start();
  }

  void destory() {
    function_footprint();
    partitionMgr.onDestroy();
    memberMgr.onDestroy();
    unRegisterModuleDescriptor();
  }

private:

  void registerModuleDescriptor() {
    function_footprint();
    std::shared_ptr<idgs::actor::ModuleDescriptorWrapper> module_descriptor(new idgs::actor::ModuleDescriptorWrapper);
    module_descriptor->setName(CLUSTER_MODULE_DESCRIPTOR_NAME);
    module_descriptor->setDescription(CLUSTER_MODULE_DESCRIPTOR_DESCRIPTION);

    // add membershipmgr, partitionmgr actor descriptor
    module_descriptor->addActorDescriptor(memberMgr.getDescriptor());
    module_descriptor->addActorDescriptor(partitionMgr.getDescriptor());

    ::idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance().registerModuleDescriptor(
        module_descriptor->getName(), module_descriptor);
  }

  void unRegisterModuleDescriptor() {
    function_footprint();
    // register
    ::idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance().unRegisterModuleDescriptor(
        CLUSTER_MODULE_DESCRIPTOR_NAME);
  }

  CLUSTERADAPTER clusterAdapter;
  MEMBERMANAGER memberMgr;
  PARTITIONMANAGER partitionMgr;

  bool initialized;

  std::shared_ptr<pb::ClusterConfig> clusterConfig;

};
// end class(interface) ClusterManager

typedef BasicClusterFramework<CorosyncClusterAdapter, MembershipTableMgr, PartitionTableMgr> ClusterFramework;

} // end namespace cluster
} // end namespace idgs
