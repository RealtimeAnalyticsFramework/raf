
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once
#include "idgs/idgslogging.h"
#include "idgs/util/enum_def.h"

namespace idgs {

#define CHECK_RC(rc) \
  do { \
    ::idgs::ResultCode local_ugly_name = (rc); \
    if (local_ugly_name != idgs::RC_OK) { \
      LOG(WARNING) << "Error: " << local_ugly_name << "(" << idgs::getErrorDescription(local_ugly_name) << ")"; \
      return local_ugly_name; \
    } \
   } while(0)

#define TEST_CHECK_RC(rc, desc) \
  do { \
    ::idgs::ResultCode result_code = (rc); \
    if (result_code != idgs::RC_OK) { \
      LOG(ERROR) << "Error ID: " << result_code << ", Cause: " << idgs::getErrorDescription(result_code) << ", description: " << desc; \
      ASSERT_EQ(idgs::RC_OK, result_code); \
    } \
   } while(0)

  DEF_ENUM(ResultCode,
    RC_OK      = 0,
    RC_SUCCESS = 0,
    RC_ERROR,
    RC_BREAK,
    RC_UNKNOWN_OPERATION,

    //RPC error codes
    RC_TIMEOUT,
    RC_ACTOR_NOT_FOUND,
    RC_TCP_ACTOR_NOT_FOUND,
    RC_TRANSPORT_TYPE_NOT_SUPPORT,
    RC_ENDPOINT_NOT_FOUND,
    RC_SCHEDULER_ALREADY_STARTED,
    RC_SCHEDULER_ALREADY_STOPPED,
    RC_MULTICAST_MESSAGE_ERROR,
    RC_QUERY_REMOTE_HOST_ERROR,
    RC_DATA_TOO_LARGE_ERROR,

    //data store error codes
    RC_CONFIG_FILE_NOT_FOUND,
    RC_STORE_NOT_FOUND,
    RC_LISTENER_CONFIG_NOT_FOUND,
    RC_LISTENER_PARAM_NOT_FOUND,
    RC_DATA_NOT_FOUND,
    RC_INVALID_KEY,
    RC_INVALID_VALUE,
    RC_KEY_TYPE_NOT_REGISTERED,
    RC_VALUE_TYPE_NOT_REGISTERED,
    RC_NOT_SUPPORT,
    RC_LOAD_PROTO_ERROR,
    RC_PARTITION_NOT_FOUND,
    RC_PARTITION_NOT_READY,
    RC_NOT_LOCAL_STORE,
    RC_INVALID_STORE_LISTENER,

    // cluster error codes
    RC_CLUSTER_ERR_CFG,
    RC_CLUSTER_ERR_CLUSTER_INIT,
    RC_CLUSTER_ERR_CPG_INIT,
    RC_CLUSTER_ERR_CPG_DISPATCH,
    RC_CLUSTER_ERR_CPG_GET_HANDLE,
    RC_CLUSTER_ERR_PARSE_CONFIG_FILE,
    RC_CLUSTER_ERR_INIT,
    RC_CLUSTER_ERR_JOIN,
    RC_CLUSTER_ERR_MULTICAST,
    RC_CLUSTER_ERR_SERIALIZE_MSG,
    RC_CLUSTER_ERR_DESERIALIZE_MSG,

    // framework error codes
    RC_DATA_CONVERT_ERROR,
    RC_INVALID_MESSAGE,
    RC_JSON_PARSE_ERROR,
    RC_JSON_MESSAGE_NOT_MATCH,
    RC_MESSAGE_FIELD_NOT_FOUND,
    RC_FILE_NOT_FOUND,
    RC_FILE_READ_ERROR,
    RC_FILE_TOO_BIG,

    //client error codes
    RC_CLIENT_SOCKET_IS_ALREADY_OPEN,
    RC_CLIENT_SOCKET_IS_ALREADY_CLOSED,
    RC_CLIENT_ALL_SERVERS_NOT_AVAILABLE,
    RC_CLIENT_SERVER_IS_NOT_AVAILABLE,
    RC_SYNTAX_ERROR,
    RC_COMMAND_ALREADY_PARSED,
    RC_COMMAND_NOT_BE_PARSED,
    RC_COMMAND_RPC_MSG_ALREADY_PARSED,
    RC_CLIENT_NO_THREAD_IS_AVAILABLE,
    RC_CLIENT_POOL_IS_AREADY_INIT,
    RC_CLIENT_POOL_IS_NOT_INIT,
    RC_NO_CLIENT_IS_AVAILABLE,
    RC_ASYNCH_CLIENT_CONNECT_ERROR,

    RC_RDD_IS_NOT_PREARED,
    RC_RDD_IS_NOT_READY,
    RC_RDD_REQUIRE_LOCAL_STORE,

    /// other errors
    RC_PARSE_LINE_ERROR,

    RC_MAX,                 // the last code
  // do NOT append code here.
  );

  const std::string& getErrorDescription(ResultCode code);

  std::string dumpErrorDescription();
} // namespace idgs {
