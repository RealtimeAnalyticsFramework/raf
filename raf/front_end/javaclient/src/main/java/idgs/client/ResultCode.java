
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.client;

import java.util.Vector;

public enum ResultCode {

	RC_OK(0),
  RC_SUCCESS(0),
  RC_ERROR(1),
  RC_BREAK(2),
  RC_UNKNOWN_OPERATION(3),

  //RPC error codes
  RC_TIMEOUT(4),
  RC_ACTOR_NOT_FOUND(5),
  RC_TCP_ACTOR_NOT_FOUND(6),
  RC_TRANSPORT_TYPE_NOT_SUPPORT(7),
  RC_ENDPOINT_NOT_FOUND(8),
  RC_SCHEDULER_ALREADY_STARTED(9),
  RC_SCHEDULER_ALREADY_STOPPED(10),
  RC_MULTICAST_MESSAGE_ERROR(11),
  RC_QUERY_REMOTE_HOST_ERROR(12),
  RC_DATA_TOO_LARGE_ERROR(13),

  //data store error codes
  RC_CONFIG_FILE_NOT_FOUND(14),
  RC_STORE_NOT_FOUND(15),
  RC_LISTENER_CONFIG_NOT_FOUND(16),
  RC_LISTENER_PARAM_NOT_FOUND(17),
  RC_DATA_NOT_FOUND(18),
  RC_INVALID_KEY(19),
  RC_INVALID_VALUE(20),
  RC_KEY_TYPE_NOT_REGISTERED(21),
  RC_VALUE_TYPE_NOT_REGISTERED(22),
  RC_NOT_SUPPORT(23),
  RC_LOAD_PROTO_ERROR(24),
  RC_PARTITION_NOT_FOUND(25),
  RC_PARTITION_NOT_READY(26),
  RC_NOT_LOCAL_STORE(27),

  // cluster error codes
  RC_CLUSTER_ERR_CFG(28),
  RC_CLUSTER_ERR_CLUSTER_INIT(29),
  RC_CLUSTER_ERR_CPG_INIT(30),
  RC_CLUSTER_ERR_CPG_DISPATCH(31),
  RC_CLUSTER_ERR_CPG_GET_HANDLE(32),
  RC_CLUSTER_ERR_PARSE_CONFIG_FILE(33),
  RC_CLUSTER_ERR_INIT(34),
  RC_CLUSTER_ERR_JOIN(35),
  RC_CLUSTER_ERR_MULTICAST(36),
  RC_CLUSTER_ERR_SERIALIZE_MSG(37),
  RC_CLUSTER_ERR_DESERIALIZE_MSG(38),

  // framework error codes
  RC_DATA_CONVERT_ERROR(39),
  RC_INVALID_MESSAGE(40),
  RC_JSON_PARSE_ERROR(41),
  RC_JSON_MESSAGE_NOT_MATCH(42),
  RC_MESSAGE_FIELD_NOT_FOUND(43),
  RC_FILE_NOT_FOUND(44),
  RC_FILE_READ_ERROR(45),
  RC_FILE_TOO_BIG(46),

  //client error codes
  RC_CLIENT_SOCKET_IS_ALREADY_OPEN(47),
  RC_CLIENT_SOCKET_IS_ALREADY_CLOSED(48),
  RC_CLIENT_ALL_SERVERS_NOT_AVAILABLE(49),
  RC_CLIENT_SERVER_IS_NOT_AVAILABLE(50),
  RC_SYNTAX_ERROR(51),
  RC_COMMAND_ALREADY_PARSED(52),
  RC_COMMAND_NOT_BE_PARSED(53),
  RC_COMMAND_RPC_MSG_ALREADY_PARSED(54),
  RC_CLIENT_NO_THREAD_IS_AVAILABLE(55),
  RC_CLIENT_POOL_IS_AREADY_INIT(56),
  RC_CLIENT_POOL_IS_NOT_INIT(57),
  RC_NO_CLIENT_IS_AVAILABLE(58),
  RC_ASYNCH_CLIENT_CONNECT_ERROR(59),

  RC_RDD_IS_NOT_PREARED(60),
  RC_RDD_IS_NOT_READY(61),
  RC_RDD_REQUIRE_LOCAL_STORE(62),

  /// other errors
  RC_PARSE_LINE_ERROR(63),

  RC_MAX(300); // the last code, do not append max than this code
	
	ResultCode(int code) {
		this.code = code;
	}
	
	private int code;
	
	public int getCode() {
		return this.code;
	}
	
	public String getErrorDescription(ResultCode code) {
		return ResultCodeDescription.getInstance().getErrorDescription(code);
	}
	
	public String dumpErrorDescription() {
		return ResultCodeDescription.getInstance().toString();
	}
	
	static class ResultCodeDescription {
		
	    private Vector<String> messages = new Vector<String>(RC_MAX.getCode());

	    public static ResultCodeDescription instance = new ResultCodeDescription();
	    
	    public static ResultCodeDescription getInstance() {
	    	return instance;
	    }
	    
	    ResultCodeDescription() {

	      messages.add(RC_SUCCESS.getCode()														 , "Success");
	      messages.add(RC_ERROR.getCode()															 , "Unknown error");
	      messages.add(RC_BREAK.getCode() 														 , "Break by caller");
	      messages.add(RC_UNKNOWN_OPERATION.getCode()                  , "Unknown operation");

	      //RPC error codes
	      messages.add(RC_TIMEOUT.getCode()                            , "Request time out");
	      messages.add(RC_ACTOR_NOT_FOUND.getCode()                    , "Actor not found");
	      messages.add(RC_TCP_ACTOR_NOT_FOUND.getCode()                , "tcp actor not found");
	      messages.add(RC_TRANSPORT_TYPE_NOT_SUPPORT.getCode()         , "transport type not found");
	      messages.add(RC_ENDPOINT_NOT_FOUND.getCode()                 , "could not find the endpoint");
	      messages.add(RC_SCHEDULER_ALREADY_STARTED.getCode()          , "ScheduledMessageService is already started");
	      messages.add(RC_SCHEDULER_ALREADY_STOPPED.getCode()          , "ScheduledMessageService is already stopped");
	      messages.add(RC_MULTICAST_MESSAGE_ERROR.getCode()            , "Failed to deliver multicast message");
	      messages.add(RC_QUERY_REMOTE_HOST_ERROR.getCode()            , "query the remote host error, please check network");
	      messages.add(RC_DATA_TOO_LARGE_ERROR.getCode()               , "The network data exceed the max data length ");

	      // data store error codes
	      messages.add(RC_CONFIG_FILE_NOT_FOUND.getCode()              , "Config file not found");
	      messages.add(RC_STORE_NOT_FOUND.getCode()                    , "Store not found");
	      messages.add(RC_LISTENER_CONFIG_NOT_FOUND.getCode()          , "Listener config not found");
	      messages.add(RC_LISTENER_PARAM_NOT_FOUND.getCode()           , "Parameter of listerer not found");
	      messages.add(RC_DATA_NOT_FOUND.getCode()                     , "Data not found");
	      messages.add(RC_INVALID_KEY.getCode()                        , "Key is invalid");
	      messages.add(RC_INVALID_VALUE.getCode()                      , "Value is invalid");
	      messages.add(RC_KEY_TYPE_NOT_REGISTERED.getCode()            , "Key type is not registered");
	      messages.add(RC_VALUE_TYPE_NOT_REGISTERED.getCode()          , "Value type is not registered");
	      messages.add(RC_NOT_SUPPORT.getCode()                        , "Not support");
	      messages.add(RC_LOAD_PROTO_ERROR.getCode()                   , "Error in load proto file.");
	      messages.add(RC_PARTITION_NOT_FOUND.getCode()                , "Partition not found.");
	      messages.add(RC_PARTITION_NOT_READY.getCode()                , "Partition is not ready.");
	      messages.add(RC_NOT_LOCAL_STORE.getCode()                    , "This member is not local store.");

	      // cluster error codes
	      messages.add(RC_CLUSTER_ERR_CFG.getCode()                    , "cluster config error");
	      messages.add(RC_CLUSTER_ERR_CLUSTER_INIT.getCode()           , "cluster initialize error");
	      messages.add(RC_CLUSTER_ERR_CPG_INIT.getCode()               , "CPG service initialize error");
	      messages.add(RC_CLUSTER_ERR_CPG_DISPATCH.getCode()           , "CPG service dispatch error");
	      messages.add(RC_CLUSTER_ERR_CPG_GET_HANDLE.getCode()         , "Parse member configuration file error");
	      messages.add(RC_CLUSTER_ERR_PARSE_CONFIG_FILE.getCode()      , "Parse member configuration file error");
	      messages.add(RC_CLUSTER_ERR_INIT.getCode()                   , "Member initialize error");
	      messages.add(RC_CLUSTER_ERR_JOIN.getCode()                   , "Member join group error");
	      messages.add(RC_CLUSTER_ERR_MULTICAST.getCode()              , "Multicast message error");
	      messages.add(RC_CLUSTER_ERR_SERIALIZE_MSG.getCode()          , "serialize message error");
	      messages.add(RC_CLUSTER_ERR_DESERIALIZE_MSG.getCode()        , "deserialize message error");

	      // framework error codes
	      messages.add(RC_DATA_CONVERT_ERROR.getCode()                 , "Error in convert data.");
	      messages.add(RC_INVALID_MESSAGE.getCode()                    , "Invalid message.");
	      messages.add(RC_JSON_PARSE_ERROR.getCode()                   , "Failed to parse JSON.");
	      messages.add(RC_JSON_MESSAGE_NOT_MATCH.getCode()             , "Json and protobuf are not matched.");
	      messages.add(RC_MESSAGE_FIELD_NOT_FOUND.getCode()            , "Message field is not found");
	      messages.add(RC_FILE_NOT_FOUND.getCode()                     , "File not found.");
	      messages.add(RC_FILE_READ_ERROR.getCode()                    , "File read error.");
	      messages.add(RC_FILE_TOO_BIG.getCode()                       , "File is too big.");

	      //client error codes
	      messages.add(RC_CLIENT_SOCKET_IS_ALREADY_OPEN.getCode()    	 , "The client socket is already opened");
	      messages.add(RC_CLIENT_SOCKET_IS_ALREADY_CLOSED.getCode()  	 , "The client socket is already closed");
	      messages.add(RC_CLIENT_ALL_SERVERS_NOT_AVAILABLE.getCode() 	 , "All the server is not available for the client");
	      messages.add(RC_CLIENT_SERVER_IS_NOT_AVAILABLE.getCode()   	 , "Server is not available");
	      messages.add(RC_SYNTAX_ERROR.getCode()                     	 , "command syntax error");
	      messages.add(RC_COMMAND_ALREADY_PARSED.getCode()           	 , "command has been passed");
	      messages.add(RC_COMMAND_NOT_BE_PARSED.getCode()            	 , "command is not been passed");
	      messages.add(RC_COMMAND_RPC_MSG_ALREADY_PARSED.getCode()   	 , "command rpc message has already been passed");
	      messages.add(RC_CLIENT_NO_THREAD_IS_AVAILABLE.getCode()    	 , "No client's thread is available");
	      messages.add(RC_CLIENT_POOL_IS_AREADY_INIT.getCode()       	 , "Client pool already load the configuration");
	      messages.add(RC_CLIENT_POOL_IS_NOT_INIT.getCode()          	 , "Client pool is not initialized");
	      messages.add(RC_NO_CLIENT_IS_AVAILABLE.getCode()           	 , "No client is available");
	      messages.add(RC_ASYNCH_CLIENT_CONNECT_ERROR.getCode()      	 , "Asynch client connect error");

	      // rdd
	      messages.add(RC_RDD_IS_NOT_PREARED.getCode()               	 , "This rdd is not preared.");
	      messages.add(RC_RDD_IS_NOT_READY.getCode()                   , "This rdd is not ready.");
	      messages.add(RC_RDD_REQUIRE_LOCAL_STORE.getCode()            , "Rdd must depend on local store service.");

	      /// others
	      messages.add(RC_PARSE_LINE_ERROR.getCode()                   , "Parse line error.");

	    }

	    public String getErrorDescription(ResultCode code) {
	      return messages.get(code.getCode());
	    }

	    public String toString() {
	      StringBuffer buf = new StringBuffer();
	      int i = 0;
	      for(String s : messages) {
	        buf.append(i).append(" = ").append(s).append("\n");
	      }
	      return buf.toString();
	    }
	  }
}
