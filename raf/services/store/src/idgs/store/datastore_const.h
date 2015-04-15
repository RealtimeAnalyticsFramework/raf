
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

extern const std::string OPERATION_NAME; // = "operation";
extern const std::string STORE_ATTACH_KEY; //= "key";
extern const std::string STORE_ATTACH_VALUE; //= "value";
extern const std::string STORE_ATTACH_LAST_VALUE; //= "value";
extern const std::string STORE_ATTACH_VERSION_VALUE; //= "value";
extern const std::string STORE_ATTACH_LISTENER; //= "STORE_ATTACH_LISTENER";

/// Actor id of handle CRUD operation.
extern const std::string ACTORID_STORE_SERVCIE; //= "store.service";
extern const std::string DATA_AGGREGATOR_ACTOR; //= "store.aggregator";
extern const std::string LISTENER_MANAGER; //= "store.listener_manager";

/// Option names of actor 'DATA_STORE_ACTOR'
extern const std::string OP_INSERT; //= "insert";
extern const std::string OP_UPDATE; //= "update";
extern const std::string OP_GET; //= "get";
extern const std::string OP_DELETE; //= "delete";
extern const std::string OP_COUNT; //= "count";
extern const std::string OP_TRUNCATE; //= "truncate";

extern const std::string OP_INTERNAL_INSERT; //= "DATA_STORE_LOCAL_INSERT";
extern const std::string OP_INTERNAL_UPDATE; //= "DATA_STORE_LOCAL_UPDATE";
extern const std::string OP_INTERNAL_GET; //= "DATA_STORE_LOCAL_GET";
extern const std::string OP_INTERNAL_DELETE; //= "DATA_STORE_LOCAL_REMOVE";
extern const std::string OP_INTERNAL_COUNT; //= "DATA_STORE_LOCAL_COUNT";
extern const std::string OP_INTERNAL_TRUNCATE; //= "DATA_STORE_LOCAL_TRUNCATE";

extern const std::string OP_INSERT_RESPONSE; //= "DATA_STORE_INSERT_RESPONSE";
extern const std::string OP_UPDATE_RESPONSE; //= "DATA_STORE_UPDATE_RESPONSE";
extern const std::string OP_GET_RESPONSE; //= "DATA_STORE_GET_RESPONSE";
extern const std::string OP_DELETE_RESPONSE; //= "DATA_STORE_REMOVE_RESPONSE";
extern const std::string OP_COUNT_RESPONSE; //= "DATA_STORE_COUNT_RESPONSE";
extern const std::string OP_TRUNCATE_RESPONSE; //= "DATA_CLEAR_RESPONSE";

/// Option names of actor 'LISTENER_MANAGER'

/// Actor ID of migration actor
extern const std::string MIGRATION_SOURCE_ACTOR; // = "store.migration_source";
extern const std::string MIGRATION_TARGET_ACTOR; // = "store.migration_target";
extern const std::string STORE_MIGRATION_SOURCE_ACTOR; // = "store.store_migration_source";
extern const std::string STORE_MIGRATION_TARGET_ACTOR; // = "store.store_migration_target";

/// Operation names of actor 'XXX_MIGRATION_ACTOR', for data migration
extern const std::string MIGRATION_REQUEST; // = "MIGRATION_REQUEST";
extern const std::string MIGRATION_DATA; //= "MIGRATION_DATA";
extern const std::string STORE_MIGRATION_COMPLETE; //= "STORE_MIGRATION_COMPLETE";
extern const std::string PARTITION_MIGRATION_COMPLETE; // = "PARTITION_MIGRATION_COMPLETE";
extern const std::string CANCEL_MIGRATION; // = "CANCEL_MIGRATION";
extern const std::string SOURCE_MEMBER_LEAVE; // = "SOURCE_MEMBER_LEAVE";

/// Actor ID of synchronization actor
extern const std::string SYNC_TARGET_ACTOR; // = "store.sync_target";
extern const std::string STORE_SYNC_TARGET_ACTOR; // = "store.store_sync_target";
extern const std::string SYNC_SOURCE_ACTOR; // = "store.sync_source";
extern const std::string STORE_SYNC_SOURCE_ACTOR; // = "store.store_sync_source";

/// Operation names of actor 'XXX_REPLICATION_ACTOR', for data migration
extern const std::string SYNC_REQUEST; // = "SYNC_REQUEST";
extern const std::string SYNC_DATA; // = "SYNC_DATA";
extern const std::string SYNC_COMPLETE; // = "SYNC_COMPLETE";

/// Actor id of store schema
extern const std::string ACTORID_SCHEMA_SERVCIE; //= "store.schema_service";
extern const std::string DATA_STORE_SCHEMA_AGGR_ACTOR; //= "store.schema_aggregator";

/// Operation names of DATA_STORE_SCHEMA_XXX_ACTOR
extern const std::string OP_SHOWSTORES; //= "OP_SHOWSTORES";
extern const std::string OP_SHOWSTORES_RESPONSE; //= "OP_SHOWSTORES_RESPONSE";
extern const std::string OP_CREATE_SCHEMA; //= "OP_CREATE_SCHEMA";
extern const std::string OP_LOCAL_CREATE_SCHEMA; //= "OP_LOCAL_CREATE_SCHEMA";
extern const std::string OP_CREATE_SCHEMA_RESPONSE; //= "OP_CREATE_SCHEMA_RESPONSE";
extern const std::string OP_DROP_SCHEMA; //= "OP_DROP_SCHEMA";
extern const std::string OP_LOCAL_DROP_SCHEMA; //= "OP_LOCAL_DROP_SCHEMA";
extern const std::string OP_DROP_SCHEMA_RESPONSE; //= "OP_DROP_SCHEMA_RESPONSE";
extern const std::string OP_CREATE_STORE; //= "OP_CREATE_STORE";
extern const std::string OP_LOCAL_CREATE_STORE; //= "OP_LOCAL_CREATE_STORE";
extern const std::string OP_CREATE_STORE_RESPONSE; //= "OP_CREATE_STORE_RESPONSE";
extern const std::string OP_DROP_STORE; //= "OP_DROP_STORE";
extern const std::string OP_LOCAL_DROP_STORE; //= "OP_LOCAL_DROP_STORE";
extern const std::string OP_DROP_STORE_RESPONSE; //= "OP_DROP_STORE_RESPONSE";

extern const std::string OP_DESTROY; // = "DESTROY";

// listener
extern const std::string BACKUP_STORE_LISTENER; // = "backup";

} //namespace store
} // namespace idgs
