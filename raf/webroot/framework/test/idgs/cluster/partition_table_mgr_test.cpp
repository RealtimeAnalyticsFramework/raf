
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include <gtest/gtest.h>

#include "idgs/cluster/cluster_framework.h"

using namespace idgs::cluster;
using namespace idgs::pb;


// init
TEST(partition_table_mgr, init) {
  PartitionManagerActor partition_manager;
  ClusterConfig cfg;
  cfg.set_partition_count(17);
  cfg.set_max_replica_count(2);
  partition_manager.init(&cfg);
}
// toString
TEST(partition_table_mgr, toString) {
  PartitionManagerActor partition_manager;
  ClusterConfig cfg;
  cfg.set_partition_count(17);
  cfg.set_max_replica_count(2);
  partition_manager.init(&cfg);
  DVLOG(1) << partition_manager.toString();
}

// genPartitionTable
TEST(partition_table_mgr, genPartitionTable) {
  PartitionManagerActor partition_manager;
  ClusterConfig cfg;
  cfg.set_partition_count(17);
  cfg.set_max_replica_count(2);
  partition_manager.init(&cfg);
  PartitionTable table;
  partition_manager.genPartitionTable(table);
  DVLOG(1) << std::endl << table.DebugString();
}






