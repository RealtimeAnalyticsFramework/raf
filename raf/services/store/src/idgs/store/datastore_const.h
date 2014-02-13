
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <string>

namespace idgs {
namespace store {

extern const std::string STORE_MODULE_DESCRIPTOR_NAME; //= "store";
extern const std::string STORE_MODULE_DESCRIPTOR_DESCRIPTION; //= "Service store actor descriptor";

extern const std::string STORE_ATTACH_KEY; //= "key";
extern const std::string STORE_ATTACH_VALUE; //= "value";
extern const std::string STORE_ATTACH_LISTENER; //= "STORE_ATTACH_LISTENER";

/// Actor id of handle crud operation.
extern const std::string ACTORID_STORE_SERVCIE; //= "DATA_STORE_ACTOR";
extern const std::string DATA_AGGREGATOR_ACTOR; //= "DataAggregatorActor";
extern const std::string DATA_SIZE_AGGREGATOR_ACTOR; //= "DataSizeAggregatorActor";
extern const std::string DATA_REPLICATED_STORE_SYNC_ACTOR; //= "ReplicatedStoreSyncStatefulActor";
extern const std::string LISTENER_MANAGER; //= "ListenerManager";

/// Option names of actor 'DATA_STORE_ACTOR'
extern const std::string OP_INSERT; //= "DATA_STORE_INSERT";
extern const std::string OP_UPDATE; //= "DATA_STORE_UPDATE";
extern const std::string OP_GET; //= "DATA_STORE_GET";
extern const std::string OP_DELETE; //= "DATA_STORE_REMOVE";
extern const std::string OP_COUNT; //= "DATA_STORE_SIZE";
extern const std::string OP_TRUNCATE; //= "DATA_CLEAR";

extern const std::string DATA_STORE_LOCAL_INSERT; //= "DATA_STORE_LOCAL_INSERT";
extern const std::string DATA_STORE_LOCAL_UPDATE; //= "DATA_STORE_LOCAL_UPDATE";
extern const std::string DATA_STORE_LOCAL_GET; //= "DATA_STORE_LOCAL_GET";
extern const std::string DATA_STORE_LOCAL_REMOVE; //= "DATA_STORE_LOCAL_REMOVE";
extern const std::string DATA_STORE_LOCAL_SIZE; //= "DATA_STORE_LOCAL_SIZE";
extern const std::string LOCAL_DATA_CLEAR; //= "LOCAL_DATA_CLEAR";

extern const std::string DATA_STORE_INSERT_RESPONSE; //= "DATA_STORE_INSERT_RESPONSE";
extern const std::string DATA_STORE_UPDATE_RESPONSE; //= "DATA_STORE_UPDATE_RESPONSE";
extern const std::string DATA_STORE_GET_RESPONSE; //= "DATA_STORE_GET_RESPONSE";
extern const std::string DATA_STORE_REMOVE_RESPONSE; //= "DATA_STORE_REMOVE_RESPONSE";
extern const std::string DATA_STORE_SIZE_RESPONSE; //= "DATA_STORE_SIZE_RESPONSE";
extern const std::string DATA_CLEAR_RESPONSE; //= "DATA_CLEAR_RESPONSE";

/// Actor id of handle sync data operation.
extern const std::string DATA_STORE_SYNC_ACTOR; //= "DATA_STORE_SYNC_ACTOR";

/// Option names of actor 'DATA_STORE_SYNC_ACTOR'
extern const std::string DATA_STORE_SYNC_REQUEST; //= "DATA_STORE_SYNC_REQUEST";
extern const std::string DATA_STORE_SYNC; //= "DATA_STORE_SYNC";
extern const std::string DATA_STORE_SYNC_RESPONSE; //= "DATA_STORE_SYNC_RESPONSE";

/// Option names of actor 'LISTENER_MANAGER'

} //namespace store
} // namespace idgs
