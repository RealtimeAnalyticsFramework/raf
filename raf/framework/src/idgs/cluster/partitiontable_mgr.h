
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once
#include "membershiptable_mgr.h"
#include "partition_listener.h"
#include "naive_partition_balancer.h"

#include <tbb/tbb.h>

namespace idgs {

namespace cluster {

class PartitionTableMgr: public idgs::actor::StatelessActor, public idgs::cluster::MemberEventListener {

  typedef idgs::actor::StatelessActor super;

public:

  PartitionTableMgr();

  ~PartitionTableMgr();

  void onDestroy() override;

  virtual const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override;


  /**
   *  initialize partition manager
   */
  void init(idgs::pb::ClusterConfig* cfg);

  /**
   *  Register ActorDescriptor and Actor
   */
  void registerActorFramework();

  /**
   *  Balance partition table
   */
  void balance(MemberWrapper& member, idgs::pb::DeltaPartitionEvent& evt);

  /**
   *  Balance partition table
   */
  void reBalance();

  /**
   *  Set new member Id and partition state after balancing
   */
  void balanceOk(pb::DeltaPartitionEvent& evt);

  /**
   *  Multicast partition message
   */
  idgs::ResultCode multicastPartitionMessage(const std::string& msg_type,
      std::shared_ptr<google::protobuf::Message> actor_msg);

  /**
   *  Override parent create actor message
   */
  std::shared_ptr<idgs::actor::ActorMessage> createActorMessage();

  void setMembershipTableManager(MembershipTableMgr& memberManager) {
    this->member_manager = &memberManager;
  }

  /**
   *  Add a partition lister
   */
  void addListener(PartitionListener* listener);

  /**
   *  Remove a partition listener
   */
  void removeListener(PartitionListener* listener);

  /**
   *  Return member's id on some partition's some position
   */
  int32_t getMemberId(uint32_t partition_id, int32_t position) {
    return partitions[partition_id].getMemberId(position);
  }

  /**
   * Generate a new partition table by current partition table
   */
  void genPartitionTable(idgs::pb::PartitionTable& table);

  /**
   * Return current partition table's reference
   */
  const std::vector<PartitionWrapper>& getPartitionTable() const {
    return partitions;
  }

  /**
   *  Return some partition by partition's id
   */
  PartitionWrapper* getPartition(size_t partition_id) {
    return &partitions[partition_id];
  }

  /**
   * Return string value of current partition table
   */
  std::string toString();

  /**
   *  Generate actor descirptor
   */
  static ::idgs::actor::ActorDescriptorPtr generateActorDescriptor();

  PartitionBalancer* getPartitionBalancer() const {
    return partitionBalancer;
  }

  void setPartitionBalancer(PartitionBalancer* partitionBalancer) {
    this->partitionBalancer = partitionBalancer;
  }

private:

  tbb::atomic<bool> ready;

  /**
   * Pointer indirect to Cluster's configuration
   */
  idgs::pb::ClusterConfig* config;

  /**
   *  Partition table containers
   */
  std::vector<PartitionWrapper> partitions;

  /**
   *  PartitonListner containers
   */
  std::list<PartitionListener*> listeners;

  /**
   * Pointer pointed to membership table manager
   */
  MembershipTableMgr* member_manager;

  /**
   * Partition balancer
   */
  PartitionBalancer* partitionBalancer;

  /**
   *  create partition table by fixed partition count and backup nodes
   */
  void createPartitionTable(size_t partition_count, uint8_t backup_nodes);

  /**
   *  Fire partition event when partition changed, registered listeners will be notified
   */
  void fireEvent(const idgs::pb::DeltaPartitionEvent& evt);

  /**
   *  New joined member handle whole partition event sent by leading
   */
  void handleWholePartitionTableEvent(const std::shared_ptr<idgs::actor::ActorMessage>& actor_msg_ptr);

  /**
   *  Handle delta partition event sent by leading after partition table is balanced caused by new member joined or active member leave
   */
  void handleDeltaPartitionEvent(const std::shared_ptr<idgs::actor::ActorMessage>& actor_msg_ptr);

  /**
   * Create member changed event
   */
  void statusChanged(const MemberWrapper& member);

};
// end class PartitionTableMgr
}// end namespace cluster
} // end namespace idgs
