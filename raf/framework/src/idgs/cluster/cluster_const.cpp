
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "cluster_const.h"

namespace idgs {
namespace cluster {
//
// module name and descriptor
//
const char CLUSTER_MODULE_DESCRIPTOR_NAME[] = "cluster";
const char CLUSTER_MODULE_DESCRIPTOR_DESCRIPTION[] = "Cluster's Actor Descriptors";

//
// Actor id and operation names
//
const char AID_MEMBER[] = "cluster.member";
const size_t AID_MEMBER_LENGTH = sizeof(AID_MEMBER)/sizeof(char);

const char OID_MEMBER[] = "MEMBER";
const char OID_WHOLE_MEMBERSHIP_TABLE[] = "WHOLE_MEMBERSHIP_TABLE";
const char OID_DELTA_MEMBER_AND_JOIN_POSITION[] = "DELTA_MEMBER_AND_JOIN_POSITION";
const char OID_CPG_CONFIG_CHANGE[] = "CPG_CONFIG_CHANGE";

const char OID_MEMBER_FLAGS[] = "FLAGS";
const char OID_MEMBER_STATUS[] = "MEMBER_STATUS";
const char OID_LIST_MEMBERS[] = "list_members";
const char OID_GET_CLUSTER_CFG[] = "get_cluster_cfg";

const char AID_PARTITION[] = "cluster.partition";
const size_t AID_PARTITION_LENGTH = sizeof(AID_PARTITION)/sizeof(char);
const char OID_WHOLE_PARTITION_TABLE[] = "WHOLE_PARTITION_TABLE";
const char OID_DELTA_PARTITIONS[] = "DELTA_PARTITIONS";
const char OID_PARTITION_STATE_CHANGED[] = "PARTITION_STATE_CHANGED";
const char OID_LIST_PARTITIONS[] = "list_partitions";

//
// Environment variables to override configuration file.
//
const char ENV_VAR_GROUP[] =  "idgs_group";
const char ENV_VAR_PARTITION_COUNT[] =  "idgs_partition_count";

const char ENV_VAR_WEIGHT[] =  "idgs_weight";
const char ENV_VAR_THREAD_COUNT[] =   "idgs_thread_count";
const char ENV_VAR_IO_THREAD_COUNT[] =  "idgs_io_thread_count";
const char ENV_VAR_MAX_IDLE_THREAD[] =  "idgs_max_idle_thread";

const char ENV_VAR_MTU[] =  "idgs_mtu";
const char ENV_VAR_BATCH_MESSAGE[] =  "idgs_tcp_batch";
const char ENV_VAR_BATCH_BUFFER[] =  "idgs_tcp_buffer";

const char ENV_VAR_PUBLIC_HOST[] =  "idgs_public_host";
const char ENV_VAR_PUBLIC_PORT[] =  "idgs_public_port";
const char ENV_VAR_INNER_HOST[] =  "idgs_inner_host";
const char ENV_VAR_INNER_PORT[] =  "idgs_inner_port";

const char ENV_VAR_LOCAL_STORE[] =  "idgs_local_store";
const char ENV_VAR_MODULE_PREFIX[] =  "idgs_modules_";

} // namespace cluster
} // namespace idgs

