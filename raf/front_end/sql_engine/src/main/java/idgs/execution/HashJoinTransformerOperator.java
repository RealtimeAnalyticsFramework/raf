/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.execution;

import idgs.rdd.pb.PbRddTransform.JoinField;
import idgs.rdd.pb.PbRddTransform.JoinRequest;
import idgs.rdd.pb.PbRddTransform.JoinType;
import idgs.util.ServerConst;

public class HashJoinTransformerOperator extends TransformerOperator {

  private JoinRequest.Builder builder;
  
  public HashJoinTransformerOperator() {
    super(ServerConst.HASH_JOIN_TRANSFORMER);
    builder = JoinRequest.newBuilder();
  }
  
  protected String getName() {
    return ServerConst.HASHJOIN;
  }
  
  public void setJoinField(int pos, int index, String lfield) {
    JoinField.Builder fieldBuilder = null;
    if (index >= builder.getJoinFieldCount()) {
      fieldBuilder = builder.addJoinFieldBuilder();
    } else {
      fieldBuilder = builder.getJoinFieldBuilder(index);
    }

    if (pos % 2 == 0) {
      fieldBuilder.setLJoinField(lfield);
    } else {
      fieldBuilder.setRJoinField(lfield);
    }
  }
  
  public void setJoinType(JoinType type) {
    builder.setType(type);
  }
  
  @Override
  protected void buildRequest() {
    super.buildRequest();
    JoinRequest request = builder.build();
    params.put(ServerConst.JOIN_PARAM, request);
  }

}
