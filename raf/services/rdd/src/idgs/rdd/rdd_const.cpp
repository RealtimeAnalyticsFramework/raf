
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
const std::string RDD_DYNAMIC_PROTO_PATH = "services/rdd/proto/";

const std::string RDD_NAME_KEY = "idgs.rdd.pb.RddNameKey";
const std::string RDD_NAME = "idgs.rdd.pb.RddName";

// actor name
const std::string RDD_SERVICE_ACTOR = "RDD_SERVICE_ACTOR";
const std::string STORE_DELEGATE_RDD_ACTOR = "StoreDelegateRddActor";
const std::string STORE_DELEGATE_RDD_PARTITION = "StoreDelegateRddPartition";
const std::string RDD_ACTOR = "RddActor";
const std::string RDD_PARTITION = "RddPartition";
const std::string RDD_INTERNAL_SERVICE_ACTOR = "RDD_INTERNAL";

// rdd message
const std::string RDD_TRANSFORM = "RDD_TRANSFORM";
const std::string RDD_PARTITION_ACTION = "RDD_PARTITION_ACTION";
const std::string RDD_PARTITION_READY = "RDD_PARTITION_READY";
const std::string RDD_PARTITION_PROCESS = "RDD_PARTITION_PROCESS";
const std::string GET_PARTITION_ACTOR = "GET_PARTITION_ACTOR";
const std::string PARTITION_PREPARED_REQUEST = "PARTITION_PREPARED_REQUEST";
const std::string PARTITION_PREPARED_RESPONSE = "PARTITION_PREPARED_RESPONSE";
const std::string GET_PARTITION_ACTOR_RESPONSE = "GET_PARTITION_ACTOR_RESPONSE";
const std::string RDD_PARTITION_ACTION_RESPONSE = "RDD_PARTITION_ACTION_RESPONSE";

const std::string SEND_RDD_INFO = "SEND_RDD_INFO";
const std::string SEND_RDD_INFO_RESPONSE = "SEND_RDD_INFO_RESPONSE";
const std::string RDD_READY = "RDD_READY";
const std::string RDD_STATE_REQUEST = "RDD_STATE_REQUEST";
const std::string RDD_STATE_RESPONSE = "RDD_STATE_RESPONSE";
const std::string RECEIVE_DEPENDING_RDD = "RECEIVE_DEPENDING_RDD";

const std::string CREATE_DELEGATE_PARTITION = "CREATE_DELEGATE_PARTITION";
const std::string CREATE_RDD_PARTITION = "CREATE_RDD_PARTITION";
const std::string CREATE_RDD_PARTITION_RESPONSE = "CREATE_RDD_PARTITION_RESPONSE";
const std::string CREATE_STORE_DELEGATE_RDD = "CREATE_STORE_DELEGATE_RDD";
const std::string CREATE_RDD = "CREATE_RDD";
const std::string CREATE_RDD_RESPONSE = "CREATE_RDD_RESPONSE";
const std::string RE_PARTITION_MIGRATE = "RE_PARTITION_MIGRATE";
const std::string CHECK_PARTITION_READY = "CHECK_PARTITION_READY";
const std::string PARTITION_TRANSFORM_COMPLETE = "PARTITION_TRANSFORM_COMPLETE";
const std::string TRANSFORM_REQUEST = "TRANSFORM_REQUEST";

const std::string RDD_DESTROY = "RDD_DESTROY";
const std::string RDD_PARTITION_DESTROY = "RDD_PARTITION_DESTROY";

// action message
const std::string ACTION_PARTITION_READY = "ACTION_PARTITION_READY";
const std::string ACTION_MESSAGE_PROCESS = "ACTION_MESSAGE_PROCESS";

const std::string RDD_ACTION_REQUEST = "RDD_ACTION_REQUEST";
const std::string RDD_ACTION_RESPONSE = "RDD_ACTION_RESPONSE";

// name of transform
const std::string FILTER_TRANSFORMER = "FILTER_TRANSFORMER";
const std::string UNION_TRANSFORMER = "UNION_TRANSFORMER";
const std::string GROUP_TRANSFORMER = "GROUP_TRANSFORMER";
const std::string HASH_JOIN_TRANSFORMER = "HASH_JOIN_TRANSFORMER";
const std::string REDUCE_TRANSFORMER = "REDUCE_TRANSFORMER";
const std::string REDUCEVALUE_TRANSFORMER = "REDUCEVALUE_TRANSFORMER";
//const std::string PARALLEL_REDUCEVALUE_TRANSFORMER = "PARALLEL_REDUCEVALUE_TRANSFORMER";
const std::string PARALLEL_REDUCEVALUE_TRANSFORMER = "REDUCEVALUE_TRANSFORMER";
const std::string RE_PARTITION_KEY_DATA = "RE_PARTITION_KEY_DATA";
const std::string RE_PARTITION_VALUE_DATA = "RE_PARTITION_VALUE_DATA";

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
const std::string JOIN_PARAM = "JOIN_PARAM";
const std::string ACTION_RESULT = "ACTION_RESULT";

// other
const std::string TARGET_KEY = "TARGET_KEY";
const std::string TARGET_VALUE = "TARGET_VALUE";

const std::string RDD_METADATA = "METADATA";

} // namespace rdd
} // namespace idgs
