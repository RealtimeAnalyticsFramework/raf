
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "datastore_const.h"

namespace idgs {
namespace store {

const std::string STORE_MODULE_DESCRIPTOR_NAME = "store";
const std::string STORE_MODULE_DESCRIPTOR_DESCRIPTION = "Service store actor descriptor";

const std::string STORE_ATTACH_KEY = "key";
const std::string STORE_ATTACH_VALUE = "value";
const std::string STORE_ATTACH_LISTENER = "STORE_ATTACH_LISTENER";

/// Actor id of handle crud operation.
const std::string ACTORID_STORE_SERVCIE = "store.service";
const std::string DATA_AGGREGATOR_ACTOR = "DataAggregatorActor";
const std::string DATA_SIZE_AGGREGATOR_ACTOR = "DataSizeAggregatorActor";
const std::string DATA_REPLICATED_STORE_SYNC_ACTOR = "ReplicatedStoreSyncStatefulActor";
const std::string LISTENER_MANAGER = "ListenerManager";

/// Option names of actor 'DATA_STORE_ACTOR'
const std::string OP_INSERT = "insert";
const std::string OP_UPDATE = "update";
const std::string OP_GET = "get";
const std::string OP_DELETE = "delete";
const std::string OP_COUNT = "count";
const std::string OP_TRUNCATE = "truncate";

const std::string DATA_STORE_LOCAL_INSERT = "DATA_STORE_LOCAL_INSERT";
const std::string DATA_STORE_LOCAL_UPDATE = "DATA_STORE_LOCAL_UPDATE";
const std::string DATA_STORE_LOCAL_GET = "DATA_STORE_LOCAL_GET";
const std::string DATA_STORE_LOCAL_REMOVE = "DATA_STORE_LOCAL_REMOVE";
const std::string DATA_STORE_LOCAL_SIZE = "DATA_STORE_LOCAL_SIZE";
const std::string LOCAL_DATA_CLEAR = "LOCAL_DATA_CLEAR";

const std::string DATA_STORE_INSERT_RESPONSE = "DATA_STORE_INSERT_RESPONSE";
const std::string DATA_STORE_UPDATE_RESPONSE = "DATA_STORE_UPDATE_RESPONSE";
const std::string DATA_STORE_GET_RESPONSE = "DATA_STORE_GET_RESPONSE";
const std::string DATA_STORE_REMOVE_RESPONSE = "DATA_STORE_REMOVE_RESPONSE";
const std::string DATA_STORE_SIZE_RESPONSE = "DATA_STORE_SIZE_RESPONSE";
const std::string DATA_CLEAR_RESPONSE = "DATA_CLEAR_RESPONSE";

/// Actor id of handle sync data operation.
const std::string DATA_STORE_SYNC_ACTOR = "DATA_STORE_SYNC_ACTOR";

/// Option names of actor 'DATA_STORE_SYNC_ACTOR'
const std::string DATA_STORE_SYNC_REQUEST = "DATA_STORE_SYNC_REQUEST";
const std::string DATA_STORE_SYNC = "DATA_STORE_SYNC";
const std::string DATA_STORE_SYNC_RESPONSE = "DATA_STORE_SYNC_RESPONSE";

/// Option names of actor 'LISTENER_MANAGER'

} //namespace store
} // namespace idgs
