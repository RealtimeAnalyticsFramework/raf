
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once
#include <string>

namespace idgs {
namespace cluster {

// actor id and operation name
extern const std::string AID_MEMBER; //= "cluster.member";
extern const std::string MEMBER; //= "MEMBER";
extern const std::string MEMBER_STATUS; //= "MEMBER_STATUS";
extern const std::string WHOLE_MEMBERSHIP_TABLE; //= "WHOLE_MEMBERSHIP_TABLE";
extern const std::string DELTA_MEMBER_AND_JOIN_POSITION; //= "DELTA_MEMBER_AND_JOIN_POSITION";

extern const std::string AID_PARTITION; //= "cluster.partition";
extern const std::string WHOLE_PARTITION_TABLE; //= "WHOLE_PARTITION_TABLE";
extern const std::string DELTA_PARTITIONS; //= "DELTA_PARTITIONS";
extern const std::string CLUSTER_MODULE_DESCRIPTOR_NAME; //= "cluster";
extern const std::string CLUSTER_MODULE_DESCRIPTOR_DESCRIPTION; //= "Cluster's Actor Descriptors";

// environment variables
extern const char* ENV_VAR_THREAD_COUNT; //= "idgs_thread_count";
extern const char* ENV_VAR_IO_THREAD_COUNT; //= "idgs_io_thread_count";
extern const char* ENV_VAR_MAX_IDLE_THREAD; //= "idgs_max_idle_thread";
extern const char* ENV_VAR_REPARTITION_BATCH; //= "idgs_repartition_batch";

extern const char* ENV_VAR_MTU; //= "idgs_mtu";
extern const char* ENV_VAR_TCP_BATCH; //= "idgs_tcp_batch";
extern const char* ENV_VAR_PARTITION_COUNT; //= "idgs_partition_count";

extern const char* ENV_VAR_IP; //= "idgs_member_ip";
extern const char* ENV_VAR_PORT; //= "idgs_member_port";
extern const char* ENV_VAR_INNERIP; //= "idgs_member_innerIp";
extern const char* ENV_VAR_INNERPORT; //= "idgs_member_innerPort";

extern const char* ENV_VAR_LOAD_FACTOR; //= "idgs_member_load_factor";
extern const char* ENV_VAR_IS_LOCAL_STORE; //= "idgs_member_service_local_store";
extern const char* ENV_VAR_IS_DIST_COMPUTING; //= "idgs_member_service_dist_computing";
extern const char* ENV_VAR_IS_CLIENT_AGENT; //= "idgs_member_service_client_agent";
extern const char* ENV_VAR_IS_ADMIN_CONSOLE; //= "idgs_member_service_admin_console";
extern const char* ENV_VAR_MODULE_PREFIX; //= "idgs_modules_";

} // end namespace cluster
} // end namespace idgs

