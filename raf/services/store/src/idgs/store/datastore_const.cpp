
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "datastore_const.h"

namespace idgs {
namespace store {


const char STORE_MODULE_DESCRIPTOR_NAME[] = "store";
const char STORE_MODULE_DESCRIPTOR_DESCRIPTION[] = "Service store actor descriptor";

const char OPERATION_NAME[] = "operation";
const char STORE_ATTACH_KEY[] = "key";
const char STORE_ATTACH_VALUE[] = "value";
const char STORE_ATTACH_LAST_VALUE[] = "last_value";
const char STORE_ATTACH_VERSION_VALUE[] = "version_value";
const char STORE_ATTACH_LISTENER[] = "STORE_ATTACH_LISTENER";

/// Actor id of handle CRUD operation.
const char ACTORID_STORE_SERVCIE[] = "store.service";
const char DATA_AGGREGATOR_ACTOR[] = "store.aggregator";
const char LISTENER_MANAGER[] = "store.listener_manager";

/// Option names of actor 'DATA_STORE_ACTOR'
const char OP_INSERT[] = "insert";
const char OP_UPDATE[] = "update";
const char OP_GET[] = "get";
const char OP_DELETE[] = "delete";
const char OP_COUNT[] = "count";
const char OP_TRUNCATE[] = "truncate";

const char OP_INTERNAL_INSERT[] = "OP_INTERNAL_INSERT";
const char OP_INTERNAL_UPDATE[] = "OP_INTERNAL_UPDATE";
const char OP_INTERNAL_GET[] = "OP_INTERNAL_GET";
const char OP_INTERNAL_DELETE[] = "OP_INTERNAL_DELETE";
const char OP_INTERNAL_COUNT[] = "OP_INTERNAL_COUNT";
const char OP_INTERNAL_TRUNCATE[] = "OP_INTERNAL_TRUNCATE";

const char OP_INSERT_RESPONSE[] = "OP_INSERT_RESPONSE";
const char OP_UPDATE_RESPONSE[] = "OP_UPDATE_RESPONSE";
const char OP_GET_RESPONSE[] = "OP_GET_RESPONSE";
const char OP_DELETE_RESPONSE[] = "OP_DELETE_RESPONSE";
const char OP_COUNT_RESPONSE[] = "OP_COUNT_RESPONSE";
const char OP_TRUNCATE_RESPONSE[] = "OP_TRUNCATE_RESPONSE";

/// Option names of actor 'LISTENER_MANAGER'

/// Actor ID of migration actor
const char MIGRATION_SOURCE_ACTOR[] = "store.migration_source";
const char MIGRATION_TARGET_ACTOR[] = "store.migration_target";
const char STORE_MIGRATION_SOURCE_ACTOR[] = "store.store_migration_source";
const char STORE_MIGRATION_TARGET_ACTOR[] = "store.store_migration_target";

/// Operation names of actor 'XXX_MIGRATION_ACTOR', for data migration
const char MIGRATION_REQUEST[] = "MIGRATION_REQUEST";
const char MIGRATION_DATA[] = "MIGRATION_DATA";
const char STORE_MIGRATION_COMPLETE[] = "STORE_MIGRATION_COMPLETE";
const char PARTITION_MIGRATION_COMPLETE[] = "PARTITION_MIGRATION_COMPLETE";
const char CANCEL_MIGRATION[] = "CANCEL_MIGRATION";
const char SOURCE_MEMBER_LEAVE[] = "SOURCE_MEMBER_LEAVE";

/// Actor ID of synchronization actor
const char SYNC_TARGET_ACTOR[] = "store.sync_target";
const char STORE_SYNC_TARGET_ACTOR[] = "store.store_sync_target";
const char SYNC_SOURCE_ACTOR[] = "store.sync_source";
const char STORE_SYNC_SOURCE_ACTOR[] = "store.store_sync_source";

/// Operation names of actor 'XXX_REPLICATION_ACTOR', for data migration
const char SYNC_REQUEST[] = "SYNC_REQUEST";
const char SYNC_DATA[] = "SYNC_DATA";
const char SYNC_COMPLETE[] = "SYNC_COMPLETE";

/// Actor id of store schema
const char ACTORID_SCHEMA_SERVCIE[] = "store.schema_service";
const char DATA_STORE_SCHEMA_AGGR_ACTOR[] = "store.schema_aggregator";

/// Operation names of DATA_STORE_SCHEMA_XXX_ACTOR
const char OP_SHOWSTORES[] = "OP_SHOWSTORES";
const char OP_SHOWSTORES_RESPONSE[] = "OP_SHOWSTORES_RESPONSE";
const char OP_CREATE_SCHEMA[] = "OP_CREATE_SCHEMA";
const char OP_LOCAL_CREATE_SCHEMA[] = "OP_LOCAL_CREATE_SCHEMA";
const char OP_CREATE_SCHEMA_RESPONSE[] = "OP_CREATE_SCHEMA_RESPONSE";
const char OP_DROP_SCHEMA[] = "OP_DROP_SCHEMA";
const char OP_LOCAL_DROP_SCHEMA[] = "OP_LOCAL_DROP_SCHEMA";
const char OP_DROP_SCHEMA_RESPONSE[] = "OP_DROP_SCHEMA_RESPONSE";
const char OP_CREATE_STORE[] = "OP_CREATE_STORE";
const char OP_LOCAL_CREATE_STORE[] = "OP_LOCAL_CREATE_STORE";
const char OP_CREATE_STORE_RESPONSE[] = "OP_CREATE_STORE_RESPONSE";
const char OP_DROP_STORE[] = "OP_DROP_STORE";
const char OP_LOCAL_DROP_STORE[] = "OP_LOCAL_DROP_STORE";
const char OP_DROP_STORE_RESPONSE[] = "OP_DROP_STORE_RESPONSE";

const char OP_DESTROY[] = "DESTROY";

// listener
const char BACKUP_STORE_LISTENER[] = "backup";

} //namespace store
} // namespace idgs
