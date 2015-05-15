
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "idgs/pb/cluster_config.pb.h"

namespace idgs {

namespace cluster {

class MemberWrapper {
public:
  MemberWrapper();
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

  size_t getPartitionCount(size_t pos);

  void resetPartitionCount() {
    totalPartitionCount = 0;
    for (auto& c: partitionCount) {
      c = 0;
    }
  }
  size_t getTotalPartitionCount () const {
    return totalPartitionCount;
  }

  void increasePartitionCount(size_t pos) {
    ++totalPartitionCount;
    setPartitionCount(pos, getPartitionCount(pos) + 1);
  }
  void decreasePartitionCount(size_t pos) {
    --totalPartitionCount;
    setPartitionCount(pos, getPartitionCount(pos) - 1);
  }

  uint32_t getPartitionReplicas() const {
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

  void setState(idgs::pb::MemberState status) {
    member.set_state(status);
  }
  idgs::pb::MemberState getState() const {
    return member.state();
  }

  void setFlags(uint64_t flags) {
    member.set_flags(flags);
  }

  void setLeading(bool flag) {
    if (flag) {
      member.set_flags(member.flags() | static_cast<uint64_t>(idgs::pb::MF_LEADING));
    } else {
      member.set_flags(member.flags() & (~static_cast<uint64_t>(idgs::pb::MF_LEADING)));
    }
  }
  bool isLeading() const {
    return member.flags() & static_cast<uint64_t>(idgs::pb::MF_LEADING);
  }

  friend std::ostream& operator <<(std::ostream& os, const MemberWrapper& mw);

  std::string toString() const;
  std::string toShortString() const;

  bool isAvailable() const;
  bool isLocalMember(const idgs::pb::Member& cfg_member) const;
  bool isLocalStore() const {
    return member.service().local_store();
  }

  void setExpectPartCount(int c) {
    expect_part_count = c;
  }
  int getExpectPartCount() const {
    return expect_part_count;
  }

private:
  void setPartitionCount(size_t pos, size_t partition_count);

  idgs::pb::Member member;
  std::vector<size_t> partitionCount;
  size_t totalPartitionCount;
  int expect_part_count;      // expected part count for primary replica; weight / weight_per_parttion.
}; // end class MemberWrapper
} // end namespace cluster
} // end namespace idgs

