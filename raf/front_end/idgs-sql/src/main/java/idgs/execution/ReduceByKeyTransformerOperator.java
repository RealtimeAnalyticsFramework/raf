/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.execution;

import idgs.client.ClientActorMessage;
import idgs.exception.IdgsException;
import idgs.rdd.pb.PbRddTransform.ReduceByKeyField;
import idgs.rdd.pb.PbRddTransform.ReduceByKeyRequest;
import idgs.util.ServerConst;

public class ReduceByKeyTransformerOperator extends TransformerOperator {

  private ReduceByKeyRequest.Builder builder;
  
  public ReduceByKeyTransformerOperator() {
    super(ServerConst.REDUCE_BY_KEY_TRANSFORMER);
    builder = ReduceByKeyRequest.newBuilder();
  }
  
  protected String getName() {
    return ServerConst.REDUCEBYKEY;
  }
  
  public void addReduceField(String fieldAlias, String type, boolean distinct) {
    ReduceByKeyField.Builder field = builder.addFieldsBuilder();
    field.setFieldAlias(fieldAlias);
    field.setType((type.toUpperCase()));
    field.setDistinct(distinct);
  }

  @Override
  protected ClientActorMessage buildRequest() throws IdgsException {
    ReduceByKeyRequest request = builder.build();
    params.put(ServerConst.TRANSFORMER_PARAM, request);
    return super.buildRequest();
  }
  
}
