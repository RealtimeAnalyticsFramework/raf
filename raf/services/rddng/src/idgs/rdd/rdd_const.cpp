
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "idgs/rdd/rdd_const.h"

namespace idgs {
namespace rdd {


// module info
const char RDD_MODULE_DESCRIPTOR_NAME[] = "rdd";
const char RDD_MODULE_DESCRIPTOR_DESCRIPTION[] = "Service rdd Actor Descriptors";
const char RDD_DYNAMIC_PROTO_PATH[] = "/tmp/idgs/proto/";

// actor name
const char RDD_SERVICE_ACTOR[] = "rdd.service";
const char PAIR_STORE_DELEGATE_RDD_ACTOR[] = "rdd.pair_delegate";
const char PAIR_STORE_DELEGATE_RDD_PARTITION[] = "rdd.pair_delegate_partition";
const char PAIR_RDD_ACTOR[] = "rdd.pair_rdd";
const char PAIR_RDD_PARTITION[] = "rdd.pair_rdd_partition";
const char RDD_INTERNAL_SERVICE_ACTOR[] = "rdd.internal_service";

// out message
const char CREATE_STORE_DELEGATE_RDD[] = "CREATE_STORE_DELEGATE_RDD";
const char CREATE_RDD[] = "CREATE_RDD";
const char CREATE_RDD_RESPONSE[] = "CREATE_RDD_RESPONSE";

const char RDD_ACTION_REQUEST[] = "RDD_ACTION_REQUEST";
const char RDD_ACTION_RESPONSE[] = "RDD_ACTION_RESPONSE";

const char RDD_DESTROY[] = "RDD_DESTROY";
const char RDD_DESTROY_RESPONSE[] = "RDD_DESTROY_RESPONSE";

const char OID_LIST_RDD[] = "list_rdd";


// internal message
const char CREATE_DELEGATE_PARTITION[] = "CREATE_DELEGATE_PARTITION";
const char CREATE_RDD_PARTITION[] = "CREATE_RDD_PARTITION";
const char CREATE_RDD_PARTITION_RESPONSE[] = "CREATE_RDD_PARTITION_RESPONSE";
const char PARTITION_CREATED[] = "PARTITION_CREATED";
const char RDD_PARTITION_PREPARED[] = "RDD_PARTITION_PREPARED";
const char RDD_STATE_SYNC[] = "RDD_STATE_SYNC";
const char PERSIST_TYPE_SYNC[] = "PERSIST_TYPE_SYNC";
const char RDD_TRANSFORM_PREPARED[] = "RDD_TRANSFORM_PREPARED";
const char MEMBER_CHANGE_EVENT[] = "MEMBER_CHANGE_EVENT";

const char RDD_TRANSFORM[] = "RDD_TRANSFORM";
const char RE_PARTITION[] = "RE_PARTITION";
const char PARTITION_STORE[] = "PARTITION_STORE";
const char PARTITION_TRANSFORM_COMPLETE[] = "PARTITION_TRANSFORM_COMPLETE";
const char CHECK_PARTITION_READY[] = "CHECK_PARTITION_READY";
const char PARTITION_READY[] = "PARTITION_READY";
const char OP_DOWNSTREAM_REMOVED[] = "DOWNSTREAM_REMOVED";

const char UPSTREAM_RDD_READY[] = "UPSTREAM_RDD_READY";
const char UPSTREAM_RDD_ERROR[] = "UPSTREAM_RDD_ERROR";

const char ACTION_MESSAGE_PROCESS[] = "ACTION_MESSAGE_PROCESS";

const char REMOVE_RDD_LOCAL[] = "REMOVE_RDD_LOCAL";

// name of transform
const char DEFAULT_OPERATOR[] = "DEFAULT_OPERATOR";
const char FILTER_TRANSFORMER[] = "FILTER_TRANSFORMER";
const char UNION_TRANSFORMER[] = "UNION_TRANSFORMER";
const char GROUP_TRANSFORMER[] = "GROUP_TRANSFORMER";
const char HASH_JOIN_TRANSFORMER[] = "HASH_JOIN_TRANSFORMER";
const char REDUCE_TRANSFORMER[] = "REDUCE_TRANSFORMER";
const char REDUCE_BY_KEY_TRANSFORMER[] = "REDUCE_BY_KEY_TRANSFORMER";
//const char PARALLEL_REDUCEVALUE_TRANSFORMER[] = "PARALLEL_REDUCEVALUE_TRANSFORMER";
const char PARALLEL_REDUCEVALUE_TRANSFORMER[] = "REDUCE_BY_KEY_TRANSFORMER";
const char RE_PARTITION_KEY[] = "RE_PARTITION_KEY";
const char RE_PARTITION_VALUE[] = "RE_PARTITION_VALUE";

/// greater than this size, using tbb/parallel_for
const char ENV_PARALLEL_REDUCE_VALUE_SIZE[] = "parallel_reduce_value_size";

// name of action
const char COUNT_ACTION[] = "COUNT_ACTION";
const char SUM_ACTION[] = "SUM_ACTION";
const char LOOKUP_ACTION[] = "LOOKUP_ACTION";
const char COLLECT_ACTION[] = "COLLECT_ACTION";
const char EXPORT_ACTION[] = "EXPORT_ACTION";
const char TOP_N_ACTION[] = "TOP_N_ACTION";

// name of attachment
const char TRANSFORMER_PARAM[] = "TRANSFORMER_PARAM";
const char ACTION_PARAM[] = "ACTION_PARAM";
const char ACTION_RESULT[] = "ACTION_RESULT";

// metadata
const char KEY_METADATA[] = "KEY_METADATA";
const char VALUE_METADATA[] = "VALUE_METADATA";

const char RDD_STORE_LISTENER[] = "RDD_STORE_LISTENER";



} // namespace rdd
} // namespace idgs
