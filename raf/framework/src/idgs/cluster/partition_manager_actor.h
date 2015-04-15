
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once
#include "member_manager_actor.h"
#include "partition_listener.h"
#include "partition_wrapper.h"

namespace idgs {
namespace cluster {

class PartitionManagerActor: public idgs::actor::StatefulActor, public idgs::cluster::MemberEventListener {
public:
  PartitionManagerActor();
  ~PartitionManagerActor();
  void onDestroy() override;

  const std::string& getActorName() const override {
    return getActorId();
  }

  virtual idgs::actor::ActorMessagePtr createActorMessage() const override;
  virtual const idgs::actor::ActorMessageHandlerMap& getMessageHandlerMap() const override;

  /**
   *  initialize partition manager
   */
  void init(idgs::pb::ClusterConfig* cfg);

  /**
   *  Balance partition table
   */
  void balance(MemberWrapper& member, idgs::pb::DeltaPartitionEvent& evt);

  /**
   *  recalculate each member own node counts on partition table.
   */
  void recalculate();

  /**
   *  Set new member Id and partition state after balancing
   */
  void balanceOk(pb::DeltaPartitionEvent& evt);

  /**
   *  multicast partition message
   */
  idgs::ResultCode multicastPartitionMessage(const std::string& msg_type, std::shared_ptr<google::protobuf::Message> actor_msg);

  /**
   *  set membership table manager
   */
  void setMembershipTableManager(MemberManagerActor& memberManager) {
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

  virtual const idgs::actor::ActorDescriptorPtr& getDescriptor() const override {
    return generateActorDescriptor();
  }


private:

  /**
   * cluster configuration
   */
  idgs::pb::ClusterConfig* config;

  /**
   *  Partition table containers
   */
  std::vector<PartitionWrapper> partitions;

  /**
   *  all registered partition changed listeners.
   */
  std::list<PartitionListener*> listeners;

  /**
   * membership table manager.
   */
  MemberManagerActor* member_manager;
private:
  /**
   *  Generate actor descirptor
   */
  static const ::idgs::actor::ActorDescriptorPtr& generateActorDescriptor();

  /**
   *  create partition table by fixed partition count and backup nodes
   */
  void createPartitionTable(size_t partition_count, uint8_t backup_nodes);

  /**
   *  fire partition changed event.
   */
  void firePartitionChangedEvent(const idgs::pb::DeltaPartitionEvent& evt);

  /**
   *  multicast whole partition table event.
   */
  idgs::ResultCode multicastWholePartitionTable(uint32_t joinedMemberId);

  /**
   *  multicast whole partition table event.
   */
  idgs::ResultCode multicastDeltaPartitionEvent(const std::shared_ptr<idgs::pb::DeltaPartitionEvent>& deltaPartitionEvent);

  /**
   *  only new joined member handle whole partition event.
   */
  void handleWholePartitionTableEvent(const std::shared_ptr<idgs::actor::ActorMessage>& actor_msg_ptr);

  /**
   *  handle delta partition event.
   */
  void handleDeltaPartitionEvent(const std::shared_ptr<idgs::actor::ActorMessage>& actor_msg_ptr);

  void handlePartitionStatusChangedEvent(const std::shared_ptr<idgs::actor::ActorMessage>& actor_msg_ptr);

  void handleListPartitions(const std::shared_ptr<idgs::actor::ActorMessage>& actor_msg_ptr);

  /**
   * implement member event listener.
   */
  void memberStatusChanged(const MemberWrapper& member) override;

}; // end class PartitionManagerActor
} // namespace cluster
} // namespace idgs
