
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "idgs/rdd/rdd_const.h"

namespace idgs {
namespace rdd {

// module info
const std::string RDD_MODULE_DESCRIPTOR_NAME = "rdd";
const std::string RDD_MODULE_DESCRIPTOR_DESCRIPTION = "Service rdd Actor Descriptors";
const std::string RDD_DYNAMIC_PROTO_PATH = "/tmp/idgs/proto/";

// actor name
const std::string RDD_SERVICE_ACTOR = "rdd.service";
const std::string PAIR_STORE_DELEGATE_RDD_ACTOR = "rdd.pair_delegate";
const std::string PAIR_STORE_DELEGATE_RDD_PARTITION = "rdd.pair_delegate_partition";
const std::string PAIR_RDD_ACTOR = "rdd.pair_rdd";
const std::string PAIR_RDD_PARTITION = "rdd.pair_rdd_partition";
const std::string RDD_INTERNAL_SERVICE_ACTOR = "rdd.internal_service";

// out message
const std::string CREATE_STORE_DELEGATE_RDD = "CREATE_STORE_DELEGATE_RDD";
const std::string CREATE_RDD = "CREATE_RDD";
const std::string CREATE_RDD_RESPONSE = "CREATE_RDD_RESPONSE";

const std::string RDD_ACTION_REQUEST = "RDD_ACTION_REQUEST";
const std::string RDD_ACTION_RESPONSE = "RDD_ACTION_RESPONSE";

const std::string RDD_DESTROY = "RDD_DESTROY";
const std::string RDD_DESTROY_RESPONSE = "RDD_DESTROY_RESPONSE";

const std::string OID_LIST_RDD = "list_rdd";


// internal message
const std::string CREATE_DELEGATE_PARTITION = "CREATE_DELEGATE_PARTITION";
const std::string CREATE_RDD_PARTITION = "CREATE_RDD_PARTITION";
const std::string CREATE_RDD_PARTITION_RESPONSE = "CREATE_RDD_PARTITION_RESPONSE";
const std::string PARTITION_CREATED = "PARTITION_CREATED";
const std::string RDD_PARTITION_PREPARED = "RDD_PARTITION_PREPARED";
const std::string RDD_STATE_SYNC = "RDD_STATE_SYNC";
const std::string PERSIST_TYPE_SYNC = "PERSIST_TYPE_SYNC";
const std::string RDD_TRANSFORM_PREPARED = "RDD_TRANSFORM_PREPARED";
const std::string MEMBER_CHANGE_EVENT = "MEMBER_CHANGE_EVENT";

const std::string RDD_TRANSFORM = "RDD_TRANSFORM";
const std::string RE_PARTITION = "RE_PARTITION";
const std::string PARTITION_STORE = "PARTITION_STORE";
const std::string PARTITION_TRANSFORM_COMPLETE = "PARTITION_TRANSFORM_COMPLETE";
const std::string CHECK_PARTITION_READY = "CHECK_PARTITION_READY";
const std::string PARTITION_READY = "PARTITION_READY";
const std::string OP_DOWNSTREAM_REMOVED = "DOWNSTREAM_REMOVED";

const std::string UPSTREAM_RDD_READY = "UPSTREAM_RDD_READY";
const std::string UPSTREAM_RDD_ERROR = "UPSTREAM_RDD_ERROR";

const std::string ACTION_MESSAGE_PROCESS = "ACTION_MESSAGE_PROCESS";

const std::string REMOVE_RDD_LOCAL = "REMOVE_RDD_LOCAL";

// name of transform
const std::string DEFAULT_OPERATOR = "DEFAULT_OPERATOR";
const std::string FILTER_TRANSFORMER = "FILTER_TRANSFORMER";
const std::string UNION_TRANSFORMER = "UNION_TRANSFORMER";
const std::string GROUP_TRANSFORMER = "GROUP_TRANSFORMER";
const std::string HASH_JOIN_TRANSFORMER = "HASH_JOIN_TRANSFORMER";
const std::string REDUCE_TRANSFORMER = "REDUCE_TRANSFORMER";
const std::string REDUCE_BY_KEY_TRANSFORMER = "REDUCE_BY_KEY_TRANSFORMER";
//const std::string PARALLEL_REDUCEVALUE_TRANSFORMER = "PARALLEL_REDUCEVALUE_TRANSFORMER";
const std::string PARALLEL_REDUCEVALUE_TRANSFORMER = "REDUCE_BY_KEY_TRANSFORMER";
const std::string RE_PARTITION_KEY = "RE_PARTITION_KEY";
const std::string RE_PARTITION_VALUE = "RE_PARTITION_VALUE";

/// greater than this size, using tbb/parallel_for
const char* ENV_PARALLEL_REDUCE_VALUE_SIZE = "parallel_reduce_value_size";

// name of action
const std::string COUNT_ACTION = "COUNT_ACTION";
const std::string SUM_ACTION = "SUM_ACTION";
const std::string LOOKUP_ACTION = "LOOKUP_ACTION";
const std::string COLLECT_ACTION = "COLLECT_ACTION";
const std::string EXPORT_ACTION = "EXPORT_ACTION";
const std::string TOP_N_ACTION = "TOP_N_ACTION";

// name of attachment
const std::string TRANSFORMER_PARAM = "TRANSFORMER_PARAM";
const std::string ACTION_PARAM = "ACTION_PARAM";
const std::string ACTION_RESULT = "ACTION_RESULT";

// metadata
const std::string KEY_METADATA = "KEY_METADATA";
const std::string VALUE_METADATA = "VALUE_METADATA";

const std::string RDD_STORE_LISTENER = "RDD_STORE_LISTENER";

} // namespace rdd
} // namespace idgs
