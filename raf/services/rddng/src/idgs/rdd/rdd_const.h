
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <string>

namespace idgs {
namespace rdd {


// module info
extern const char RDD_MODULE_DESCRIPTOR_NAME[] ;  // ="rdd";
extern const char RDD_MODULE_DESCRIPTOR_DESCRIPTION[] ;  // ="Service rdd Actor Descriptors";
extern const char RDD_DYNAMIC_PROTO_PATH[] ;  // ="services/rdd/proto/";

// actor name
extern const char RDD_SERVICE_ACTOR[] ;  // ="rdd.service";
extern const char PAIR_STORE_DELEGATE_RDD_ACTOR[] ;  // ="rdd.pair_delegate_rdd";
extern const char PAIR_STORE_DELEGATE_RDD_PARTITION[] ;  // ="rdd.pair_delegate_rdd_partition";
extern const char PAIR_RDD_ACTOR[] ;  // ="rdd.pair_rdd";
extern const char PAIR_RDD_PARTITION[] ;  // ="rdd.pair_rdd_partition";
extern const char RDD_INTERNAL_SERVICE_ACTOR[] ;  // ="rdd.internal_service";

// out message
extern const char CREATE_STORE_DELEGATE_RDD[] ;  // ="CREATE_STORE_DELEGATE_RDD";
extern const char CREATE_RDD[] ;  // ="CREATE_RDD";
extern const char CREATE_RDD_RESPONSE[] ;  // ="CREATE_RDD_RESPONSE";

extern const char RDD_ACTION_REQUEST[] ;  // ="RDD_ACTION_REQUEST";
extern const char RDD_ACTION_RESPONSE[] ;  // ="RDD_ACTION_RESPONSE";

extern const char RDD_DESTROY[] ;  // ="DESTROY";
extern const char RDD_DESTROY_RESPONSE[] ;  // ="RDD_DESTROY_RESPONSE";

extern const char OID_LIST_RDD[] ;  // = "list_rdd";

// internal message
extern const char CREATE_DELEGATE_PARTITION[] ;  // ="CREATE_DELEGATE_PARTITION";
extern const char CREATE_RDD_PARTITION[] ;  // ="CREATE_RDD_PARTITION";
extern const char CREATE_RDD_PARTITION_RESPONSE[] ;  // ="CREATE_RDD_PARTITION_RESPONSE";
extern const char PARTITION_CREATED[] ; // = "PARTITION_CREATED";
extern const char RDD_PARTITION_PREPARED[] ; // = "RDD_PARTITION_PREPARED";
extern const char RDD_STATE_SYNC[] ; // = "RDD_STATE_SYNC";
extern const char PERSIST_TYPE_SYNC[] ; // = "PERSIST_TYPE_SYNC";
extern const char RDD_TRANSFORM_PREPARED[] ; // = "RDD_TRANSFORM_PREPARED";
extern const char MEMBER_CHANGE_EVENT[] ; // = "MEMBER_CHANGE_EVENT";

extern const char RDD_TRANSFORM[] ;  // ="RDD_TRANSFORM";
extern const char RE_PARTITION[] ;  // ="RE_PARTITION";
extern const char PARTITION_STORE[] ; // = "PARTITION_STORE";
extern const char PARTITION_TRANSFORM_COMPLETE[] ;  // ="PARTITION_TRANSFORM_COMPLETE";
extern const char CHECK_PARTITION_READY[] ;  // ="CHECK_PARTITION_READY";
extern const char PARTITION_READY[] ;  // ="PARTITION_READY";
extern const char OP_DOWNSTREAM_REMOVED[] ;  // ="DOWNSTREAM_REMOVED";

extern const char UPSTREAM_RDD_READY[]; // = "UPSTREAM_RDD_READY";
extern const char UPSTREAM_RDD_ERROR[]; // = "UPSTREAM_RDD_ERROR";

extern const char ACTION_MESSAGE_PROCESS[] ;  // ="ACTION_MESSAGE_PROCESS";

extern const char REMOVE_RDD_LOCAL[] ; // = "REMOVE_RDD_LOCAL";

// name of transform
extern const char DEFAULT_OPERATOR[] ; // = "DEFAULT_OPERATOR";
extern const char FILTER_TRANSFORMER[] ;  // ="FILTER_TRANSFORMER";
extern const char UNION_TRANSFORMER[] ;  // ="UNION_TRANSFORMER";
extern const char GROUP_TRANSFORMER[] ;  // ="GROUP_TRANSFORMER";
extern const char HASH_JOIN_TRANSFORMER[] ;  // ="HASH_JOIN_TRANSFORMER";
extern const char REDUCE_TRANSFORMER[] ;  // ="REDUCE_TRANSFORMER";
extern const char REDUCE_BY_KEY_TRANSFORMER[] ;  // ="REDUCEVALUE_TRANSFORMER";
extern const char PARALLEL_REDUCEVALUE_TRANSFORMER[] ;  // ="PARALLEL_REDUCEVALUE_TRANSFORMER";
extern const char RE_PARTITION_KEY[] ;  // ="RE_PARTITION_KEY";
extern const char RE_PARTITION_VALUE[] ;  // ="RE_PARTITION_VALUE";

/// greater than this size, using tbb/parallel_for
extern const char ENV_PARALLEL_REDUCE_VALUE_SIZE[] ;  // ="parallel_reduce_value_size";

// name of action
extern const char COUNT_ACTION[] ;  // ="COUNT_ACTION";
extern const char SUM_ACTION[] ;  // ="SUM_ACTION";
extern const char LOOKUP_ACTION[] ;  // ="LOOKUP_ACTION";
extern const char COLLECT_ACTION[] ;  // ="COLLECT_ACTION";
extern const char EXPORT_ACTION[] ;  // ="EXPORT_ACTION";
extern const char TOP_N_ACTION[] ;  // ="TOP_N_ACTION";

// name of attachment
extern const char TRANSFORMER_PARAM[] ;  // ="TRANSFORMER_PARAM";
extern const char ACTION_PARAM[] ;  // ="ACTION_PARAM";
extern const char ACTION_RESULT[] ;  // ="ACTION_RESULT";

// metadata
extern const char KEY_METADATA[] ;  // ="KEY_METADATA";
extern const char VALUE_METADATA[] ; // = "VALUE_METADATA";

extern const char RDD_STORE_LISTENER[] ; // = "RDD_STORE_LISTENER";

} // namespace rdd
} // namespace idgs
