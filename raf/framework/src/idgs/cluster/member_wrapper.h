
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/pb/cluster_config.pb.h"

namespace idgs {

namespace cluster {

static std::vector<std::string> MEMBER_STATUES_DSP = {
  "INITIAL",
  "TAKEN",
  "JOINED",
  "PREPARED",
  "ACTIVE",
  "INACTIVE"
};

class MemberWrapper {
public:
  MemberWrapper();

  /// copy constructor, called by containers.
  MemberWrapper(const MemberWrapper&) = default;

  /// copy assignment, called by containers.
  MemberWrapper& operator =(const MemberWrapper& other) = default;

  ~MemberWrapper() = default;

  void setNodeId(uint32_t node_id) {
    member.set_node_id(node_id);
  }

  uint32_t getNodeId() const {
    return member.node_id();
  }

  void setPid(uint32_t pid) {
    member.set_pid(pid);
  }

  uint32_t getPid() const {
    return member.pid();
  }

  void setMember(const idgs::pb::Member& member) {
    this->member = member;
  }

  size_t getPartition(size_t pos) const {
    if (partitionCount.empty() || pos > partitionCount.size()) {
      return 0;
    }
    return partitionCount[pos];
  }

  void setPartitionCount(size_t pos, size_t partition_count);

  uint32_t getPartitionSize() const {
    return partitionCount.size();
  }

  const idgs::pb::Member& getMember() const {
    return member;
  }

  void setId(int32_t id) {
    member.set_id(id);
  }

  int32_t getId() const {
    return member.id();
  }

  void setStatus(idgs::pb::MemberStatus status) {
    member.set_status(status);
  }

  void setIsleading(bool flag) {
    member.mutable_service()->set_leading(flag);
  }

  bool isLeading() const {
    return member.service().leading();
  }

  bool canBalanced() const;

  bool isLocalMember(const idgs::pb::Member& cfg_member) const;

  bool isLocalStore() const {
    return member.service().local_store();
  }

  bool isLeave() const {
    return pb::INACTIVE == getStatus();
  }

  bool isInitial() const {
    return pb::INITIAL == getStatus();
  }

  bool isJoined() const {
    return pb::JOINED == getStatus();
  }

  bool isActive() const {
    return pb::ACTIVE == getStatus();
  }

  bool isTaken() const {
    return pb::TAKEN == getStatus();
  }

  bool isInactive() const {
    return pb::INACTIVE == getStatus();
  }

  bool isPrepared() const {
    return pb::PREPARED == getStatus();
  }

  idgs::pb::MemberStatus getStatus() const {
    return member.status();
  }

  const std::string& getGroupName() const {
    return member.group_name();
  }

  friend std::ostream& operator <<(std::ostream& os, const MemberWrapper& mw);

  std::string toString() const;

  std::string toShortString() const;

private:
  idgs::pb::Member member;
  std::vector<size_t> partitionCount;
}; // end class MemberWrapper
} // end namespace cluster
} // end namespace idgs

