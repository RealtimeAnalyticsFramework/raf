
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "partitiontable_balancer.h"

namespace idgs {

namespace cluster {

class NaivePartitionBalancer: public idgs::cluster::PartitionBalancer {

public:

  NaivePartitionBalancer() {

  }

  virtual ~NaivePartitionBalancer() {

  }

  void balance(MembershipTable& members, PartitionTable& partitions, MemberWrapper& member, idgs::pb::DeltaPartitionEvent& evt);

private:

  void balanceWhenJoined(MembershipTable& members, PartitionTable& partitions, MemberWrapper& member, pb::DeltaPartitionEvent& evt);

  void balanceWhenLeft(MembershipTable& members, PartitionTable& partitions, MemberWrapper& member, pb::DeltaPartitionEvent& evt);

  /**
   * Calculate partition count when new member join
   */
  void calculatePartitionCountWhenJoined(MembershipTable& members, MemberWrapper& joined_member, const size_t member_size, const size_t balance_count, const size_t balance_avg_count,
      const size_t balance_mod_count, std::map<size_t, size_t>& delta_partition_count_map,
      std::map<size_t, size_t>& partition_count_map);

  /**
   * Calculate partition count when member leave
   */
  void calculatePartitionCountWhenLeft(MembershipTable& members, MemberWrapper& left_member, const size_t member_size, const size_t balance_count, const size_t balance_avg_count,
      const size_t balance_mod_count, std::map<size_t, size_t>& delta_partition_count_map,
      std::map<size_t, size_t>& partition_count_map);
};

}
}
