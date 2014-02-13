/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.util;

public class ServerConst {

  // operator name
  public static final String COLLECT = "COLLECT";
  public static final String TOPN = "TOPN";
  public static final String DELEGATE = "DELEGATE";
  public static final String FILTER = "FILTER";
  public static final String GROUP = "GROUP";
  public static final String HASHJOIN = "HASHJOIN";
  public static final String REDUCEBYKEY = "REDUCEBYKEY";
  public static final String REDUCE = "REDUCE";
  public static final String UNION = "UNION";
  
  // RDD
  // actor id
  public static final String RDD_SERVICE_ACTOR = "RDD_SERVICE_ACTOR";

  // operation name
  public static final String STORE_DELEGATE = "STORE_DELEGATE";
  public static final String CREATE_STORE_DELEGATE_RDD = "CREATE_STORE_DELEGATE_RDD";
  public static final String CREATE_RDD = "CREATE_RDD";
  public static final String RDD_ACTION_REQUEST = "RDD_ACTION_REQUEST";
  public static final String RDD_ACTION_RESPONSE = "RDD_ACTION_RESPONSE";

  // transformer
  public static final String FILTER_TRANSFORMER = "FILTER_TRANSFORMER";
  public static final String UNION_TRANSFORMER = "UNION_TRANSFORMER";
  public static final String GROUP_TRANSFORMER = "GROUP_TRANSFORMER";
  public static final String HASH_JOIN_TRANSFORMER = "HASH_JOIN_TRANSFORMER";
  public static final String REDUCE_TRANSFORMER = "REDUCE_TRANSFORMER";
  public static final String REDUCEVALUE_TRANSFORMER = "REDUCEVALUE_TRANSFORMER";
  public static final String PARALLEL_REDUCEVALUE_TRANSFORMER = "REDUCEVALUE_TRANSFORMER";

  // action
  public static final String COUNT_ACTION = "COUNT_ACTION";
  public static final String SUM_ACTION = "SUM_ACTION";
  public static final String LOOKUP_ACTION = "LOOKUP_ACTION";
  public static final String COLLECT_ACTION = "COLLECT_ACTION";
  public static final String EXPORT_ACTION = "EXPORT_ACTION";
  public static final String TOP_N_ACTION = "TOP_N_ACTION";

  // attachment
  public static final String TRANSFORMER_PARAM = "TRANSFORMER_PARAM";
  public static final String ACTION_PARAM = "ACTION_PARAM";
  public static final String ACTION_RESULT = "ACTION_RESULT";
  public static final String JOIN_PARAM = "JOIN_PARAM";

  public static final String METADATA = "METADATA";
  
  // store
  // actor id
  // operation name
  // attachment
  
}
