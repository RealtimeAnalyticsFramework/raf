
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
extern const std::string RDD_MODULE_DESCRIPTOR_NAME ;  // ="rdd";
extern const std::string RDD_MODULE_DESCRIPTOR_DESCRIPTION ;  // ="Service rdd Actor Descriptors";
extern const std::string RDD_DYNAMIC_PROTO_PATH ;  // ="services/rdd/proto/";

extern const std::string RDD_NAME_KEY ;  // ="idgs.rdd.pb.RddNameKey";
extern const std::string RDD_NAME ;  // ="idgs.rdd.pb.RddName";

// actor name
extern const std::string RDD_SERVICE_ACTOR ;  // ="RDD_SERVICE_ACTOR";
extern const std::string STORE_DELEGATE_RDD_ACTOR ;  // ="StoreDelegateRddActor";
extern const std::string STORE_DELEGATE_RDD_PARTITION ;  // ="StoreDelegateRddPartition";
extern const std::string RDD_ACTOR ;  // ="RddActor";
extern const std::string RDD_PARTITION ;  // ="RddPartition";
extern const std::string RDD_INTERNAL_SERVICE_ACTOR ;  // ="RDD_INTERNAL";

// rdd message
extern const std::string RDD_TRANSFORM ;  // ="RDD_TRANSFORM";
extern const std::string RDD_PARTITION_ACTION ;  // ="RDD_PARTITION_ACTION";
extern const std::string RDD_PARTITION_READY ;  // ="RDD_PARTITION_READY";
extern const std::string RDD_PARTITION_PROCESS ;  // ="RDD_PARTITION_PROCESS";
extern const std::string GET_PARTITION_ACTOR ;  // ="GET_PARTITION_ACTOR";
extern const std::string PARTITION_PREPARED_REQUEST ;  // ="PARTITION_PREPARED_REQUEST";
extern const std::string PARTITION_PREPARED_RESPONSE ;  // ="PARTITION_PREPARED_RESPONSE";
extern const std::string GET_PARTITION_ACTOR_RESPONSE ;  // ="GET_PARTITION_ACTOR_RESPONSE";
extern const std::string RDD_PARTITION_ACTION_RESPONSE ;  // ="RDD_PARTITION_ACTION_RESPONSE";

extern const std::string SEND_RDD_INFO ;  // ="SEND_RDD_INFO";
extern const std::string SEND_RDD_INFO_RESPONSE ;  // ="SEND_RDD_INFO_RESPONSE";
extern const std::string RDD_READY ;  // ="RDD_READY";
extern const std::string RDD_STATE_REQUEST ;  // ="RDD_STATE_REQUEST";
extern const std::string RDD_STATE_RESPONSE ;  // ="RDD_STATE_RESPONSE";
extern const std::string RECEIVE_DEPENDING_RDD ;  // ="RECEIVE_DEPENDING_RDD";

extern const std::string CREATE_DELEGATE_PARTITION ;  // ="CREATE_DELEGATE_PARTITION";
extern const std::string CREATE_RDD_PARTITION ;  // ="CREATE_RDD_PARTITION";
extern const std::string CREATE_RDD_PARTITION_RESPONSE ;  // ="CREATE_RDD_PARTITION_RESPONSE";
extern const std::string CREATE_STORE_DELEGATE_RDD ;  // ="CREATE_STORE_DELEGATE_RDD";
extern const std::string CREATE_RDD ;  // ="CREATE_RDD";
extern const std::string CREATE_RDD_RESPONSE ;  // ="CREATE_RDD_RESPONSE";
extern const std::string RE_PARTITION_MIGRATE ;  // ="RE_PARTITION_MIGRATE";
extern const std::string CHECK_PARTITION_READY ;  // ="CHECK_PARTITION_READY";
extern const std::string PARTITION_TRANSFORM_COMPLETE ;  // ="PARTITION_TRANSFORM_COMPLETE";
extern const std::string TRANSFORM_REQUEST ;  // ="TRANSFORM_REQUEST";

extern const std::string RDD_DESTROY ;  // ="RDD_DESTROY";
extern const std::string RDD_PARTITION_DESTROY ;  // ="RDD_PARTITION_DESTROY";

// action message
extern const std::string ACTION_PARTITION_READY ;  // ="ACTION_PARTITION_READY";
extern const std::string ACTION_MESSAGE_PROCESS ;  // ="ACTION_MESSAGE_PROCESS";

extern const std::string RDD_ACTION_REQUEST ;  // ="RDD_ACTION_REQUEST";
extern const std::string RDD_ACTION_RESPONSE ;  // ="RDD_ACTION_RESPONSE";

// name of transform
extern const std::string FILTER_TRANSFORMER ;  // ="FILTER_TRANSFORMER";
extern const std::string UNION_TRANSFORMER ;  // ="UNION_TRANSFORMER";
extern const std::string GROUP_TRANSFORMER ;  // ="GROUP_TRANSFORMER";
extern const std::string HASH_JOIN_TRANSFORMER ;  // ="HASH_JOIN_TRANSFORMER";
extern const std::string REDUCE_TRANSFORMER ;  // ="REDUCE_TRANSFORMER";
extern const std::string REDUCEVALUE_TRANSFORMER ;  // ="REDUCEVALUE_TRANSFORMER";
extern const std::string PARALLEL_REDUCEVALUE_TRANSFORMER ;  // ="PARALLEL_REDUCEVALUE_TRANSFORMER";
extern const std::string RE_PARTITION_KEY_DATA ;  // ="RE_PARTITION_KEY_DATA";
extern const std::string RE_PARTITION_VALUE_DATA ;  // ="RE_PARTITION_VALUE_DATA";

/// greater than this size, using tbb/parallel_for
extern const char* ENV_PARALLEL_REDUCE_VALUE_SIZE ;  // ="parallel_reduce_value_size";

// name of action
extern const std::string COUNT_ACTION ;  // ="COUNT_ACTION";
extern const std::string SUM_ACTION ;  // ="SUM_ACTION";
extern const std::string LOOKUP_ACTION ;  // ="LOOKUP_ACTION";
extern const std::string COLLECT_ACTION ;  // ="COLLECT_ACTION";
extern const std::string EXPORT_ACTION ;  // ="EXPORT_ACTION";
extern const std::string TOP_N_ACTION ;  // ="TOP_N_ACTION";

// name of attachment
extern const std::string TRANSFORMER_PARAM ;  // ="TRANSFORMER_PARAM";
extern const std::string ACTION_PARAM ;  // ="ACTION_PARAM";
extern const std::string JOIN_PARAM ;  // ="JOIN_PARAM";
extern const std::string ACTION_RESULT ;  // ="ACTION_RESULT";

// other
extern const std::string TARGET_KEY ;  // ="TARGET_KEY";
extern const std::string TARGET_VALUE ;  // ="TARGET_VALUE";

extern const std::string RDD_METADATA ;  // ="METADATA";

} // namespace rdd
} // namespace idgs
