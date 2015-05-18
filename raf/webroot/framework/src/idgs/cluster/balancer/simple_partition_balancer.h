/*
 Copyright (c) <2012>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#pragma once
#include "partition_balancer.h"

namespace idgs {
namespace cluster {

class SimplePartitionBalancer : public idgs::cluster::PartitionBalancer{
public:
  SimplePartitionBalancer();
  virtual ~SimplePartitionBalancer();

public:
  /**
   * Balance partition table when member join or leave
   * @param members membership table
   * @param partitions partition table
   * @param mid join/leave member
   * @param evt created delta partition event
   */
  virtual int balance(MembershipTable& members, PartitionTable& partitions, int mid, idgs::pb::DeltaPartitionEvent& evt) override;

private:
  ///
  /// a very simple rebalancer, which just re-construct the whole partition table.
  ///
  int simple_balance();

  ///
  /// rebalance when member leave
  ///
  int balanceLeave();

  ///
  /// rebalance when member leave
  ///
  int balanceJoin();

  ///
  /// adjust partition table: swap replicas if necessary
  ///
  void adjust();

  /// check whether partitions[part][pos] can be set to mid
  bool checkMember (int part, int pos, int mid);
  /// set partitions[part][pos] to mid, move the original mid if possible
  /// update actual part count in member table
  void setMember (int part, int pos, int mid);
  /// swap mid between pos1 and pos2
  /// update actual part count in member table
  void swapMember (int part, int pos1, int pos2);

private:
  PartitionTable *old_partitions;
  PartitionTable new_partitions;
  MembershipTable* members;
  MemberWrapper*   member;          // the join/leave member
  int total_avaialbe_member_count;
};

} // namespace cluster
} // namespace idgs
