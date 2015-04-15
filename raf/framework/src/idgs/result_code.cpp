
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/result_code.h"

using namespace std;

namespace idgs {
  class ResultCodeDescription {
  private:
    vector<string> messages;

  public:
    ResultCodeDescription() {
      messages.resize(RC_MAX);

      messages[RC_SUCCESS]                            = "Success";
      messages[RC_ERROR]                              = "Unknown error";
      messages[RC_BREAK]                              = "Break by caller";
      messages[RC_UNKNOWN_OPERATION]                  = "Unknown operation";

      //RPC error codes
      messages[RC_TIMEOUT]                            = "Request time out";
      messages[RC_ACTOR_NOT_FOUND]                    = "Actor not found";
      messages[RC_TCP_ACTOR_NOT_FOUND]                = "tcp actor not found";
      messages[RC_TRANSPORT_TYPE_NOT_SUPPORT]         = "transport type not found";
      messages[RC_ENDPOINT_NOT_FOUND]                 = "could not find the endpoint";
      messages[RC_SCHEDULER_ALREADY_STARTED]          = "ScheduledMessageService is already started";
      messages[RC_SCHEDULER_ALREADY_STOPPED]          = "ScheduledMessageService is already stopped";
      messages[RC_MULTICAST_MESSAGE_ERROR]            = "Failed to deliver multicast message";
      messages[RC_QUERY_REMOTE_HOST_ERROR]            = "query the remote host error, please check network";
      messages[RC_DATA_TOO_LARGE_ERROR]               = "The network data exceed the max data length ";

      // data store error codes
      messages[RC_CONFIG_FILE_NOT_FOUND]              = "Config file not found";
      messages[RC_SCHEMA_NOT_FOUND]                   = "Schema not found";
      messages[RC_STORE_NOT_FOUND]                    = "Store not found";
      messages[RC_STORE_EXISTED]                      = "Store has already existed.";
      messages[RC_LISTENER_CONFIG_NOT_FOUND]          = "Listener config not found";
      messages[RC_LISTENER_PARAM_NOT_FOUND]           = "Parameter of listerer not found";
      messages[RC_DATA_NOT_FOUND]                     = "Data not found";
      messages[RC_INVALID_KEY]                        = "Key is invalid";
      messages[RC_INVALID_VALUE]                      = "Value is invalid";
      messages[RC_KEY_TYPE_NOT_REGISTERED]            = "Key type is not registered";
      messages[RC_VALUE_TYPE_NOT_REGISTERED]          = "Value type is not registered";
      messages[RC_NOT_SUPPORT]                        = "Not support";
      messages[RC_LOAD_PROTO_ERROR]                   = "Error in load proto file.";
      messages[RC_PARTITION_NOT_FOUND]                = "Partition not found.";
      messages[RC_PARTITION_NOT_READY]                = "Partition is not ready.";
      messages[RC_NOT_LOCAL_STORE]                    = "This member is not local store.";
      messages[RC_INVALID_STORE_LISTENER]             = "Listener of store is invalide.";
      messages[RC_INVALID_BACKUP_COUNT]               = "Backup count of store must be less then max count.";
      messages[RC_PARSE_CONFIG_ERROR]                 = "Parse config file or content error";

      // cluster error codes
      messages[RC_CLUSTER_ERR_CFG]                    = "cluster config error";
      messages[RC_CLUSTER_ERR_CLUSTER_INIT]           = "cluster initialize error";
      messages[RC_CLUSTER_ERR_CPG_INIT]               = "CPG service initialize error";
      messages[RC_CLUSTER_ERR_CPG_DISPATCH]           = "CPG service dispatch error";
      messages[RC_CLUSTER_ERR_CPG_GET_HANDLE]         = "Parse member configuration file error";
      messages[RC_CLUSTER_ERR_PARSE_CONFIG_FILE]      = "Parse member configuration file error";
      messages[RC_CLUSTER_ERR_INIT]                   = "Member initialize error";
      messages[RC_CLUSTER_ERR_JOIN]                   = "Member join group error";
      messages[RC_CLUSTER_ERR_MULTICAST]              = "Multicast message error";
      messages[RC_CLUSTER_ERR_SERIALIZE_MSG]          = "serialize message error";
      messages[RC_CLUSTER_ERR_DESERIALIZE_MSG]        = "deserialize message error";

      // framework error codes
      messages[RC_DATA_CONVERT_ERROR]                 = "Error in convert data.";
      messages[RC_INVALID_MESSAGE]                    = "Invalid message.";
      messages[RC_JSON_PARSE_ERROR]                   = "Failed to parse JSON.";
      messages[RC_JSON_MESSAGE_NOT_MATCH]             = "Json and protobuf are not matched.";
      messages[RC_MESSAGE_FIELD_NOT_FOUND]            = "Message field is not found";
      messages[RC_FILE_NOT_FOUND]                     = "File not found.";
      messages[RC_FILE_READ_ERROR]                    = "File read error.";
      messages[RC_FILE_TOO_BIG]                       = "File is too big.";

      //client error codes
      messages[RC_CLIENT_SOCKET_IS_ALREADY_OPEN]    = "The client socket is already opened";
      messages[RC_CLIENT_SOCKET_IS_ALREADY_CLOSED]  = "The client socket is already closed";
      messages[RC_CLIENT_ALL_SERVERS_NOT_AVAILABLE] = "All the server is not available for the client";
      messages[RC_CLIENT_SERVER_IS_NOT_AVAILABLE]   = "Server is not available";
      messages[RC_SYNTAX_ERROR]                     = "command syntax error";
      messages[RC_COMMAND_ALREADY_PARSED]           = "command has been passed";
      messages[RC_COMMAND_NOT_BE_PARSED]            = "command is not been passed";
      messages[RC_COMMAND_RPC_MSG_ALREADY_PARSED]   = "command rpc message has already been passed";
      messages[RC_CLIENT_NO_THREAD_IS_AVAILABLE]    = "No client's thread is available";
      messages[RC_CLIENT_POOL_IS_AREADY_INIT]       = "Client pool already load the configuration";
      messages[RC_CLIENT_POOL_IS_NOT_INIT]          = "Client pool is not initialized";
      messages[RC_NO_CLIENT_IS_AVAILABLE]           = "No client is available";
      messages[RC_ASYNCH_CLIENT_CONNECT_ERROR]      = "Asynch client connect error";

      // rdd
      messages[RC_RDD_IS_NOT_PREARED]               = "This rdd is not preared.";
      messages[RC_RDD_IS_NOT_READY]                 = "This rdd is not ready.";
      messages[RC_RDD_REQUIRE_LOCAL_STORE]          = "Rdd must depend on local store service.";

      /// others
      messages[RC_PARSE_LINE_ERROR]                 = "Parse line error.";

    }

    const std::string& getErrorDescription(ResultCode code) const {
      return messages[code];
    }

    std::string toString() const {
      stringstream ss;
      int i = 0;
      for(vector<string>::const_iterator itr = messages.begin(); itr != messages.end(); ++itr, ++i) {
        ss << i << " = " << *itr << std::endl;
      }
      return ss.str();
    }
  };


  const std::string& getErrorDescription(ResultCode code) {
    static ResultCodeDescription desc;
    const std::string& msg = desc.getErrorDescription(code);
    return msg;
  }

  std::string dumpErrorDescription() {
    static ResultCodeDescription desc;
    std::string msg = desc.toString();
    return msg;
  }

} // namespace idgs





