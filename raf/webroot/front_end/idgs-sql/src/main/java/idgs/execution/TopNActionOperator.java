/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.execution;

import idgs.client.ClientActorMessage;
import idgs.client.util.ProtoSerde;
import idgs.client.util.ProtoSerdeFactory;
import idgs.exception.IdgsException;
import idgs.pb.PbRpcMessage.PayloadSerdes;
import idgs.rdd.pb.PbRddAction.KeyValuePair;
import idgs.rdd.pb.PbRddAction.OrderField;
import idgs.rdd.pb.PbRddAction.TopNActionRequest;
import idgs.rdd.pb.PbRddAction.TopNActionResult;
import idgs.rdd.pb.PbRddService.FieldNamePair;
import idgs.util.ServerConst;

import com.google.protobuf.DynamicMessage;
import com.google.protobuf.Message;

public class TopNActionOperator extends ActionOperator {
  
  private TopNActionRequest.Builder topNBuilder;
  
  public TopNActionOperator() {
    super(ServerConst.TOP_N_ACTION, TopNActionResult.newBuilder());
    topNBuilder = TopNActionRequest.newBuilder();
  }
  
  protected String getName() {
    return ServerConst.TOPN;
  }
  
  public void setLimit(long limit) {
    setLimit(limit);
    topNBuilder.setTopN(limit);
  }
  
  public void addOrderField(FieldNamePair field, boolean desc) {
    OrderField.Builder orderField = topNBuilder.addOrderFieldBuilder();
    orderField.setDesc(desc);
    orderField.setFieldName(field.getFieldAlias());
    orderField.setFieldType(field.getFieldType());
    orderField.setExpr(field.getExpr());
  }
  
  @Override
  public void handleResult(Message actionResult, ResultSet resultSet) {
    TopNActionResult result = (TopNActionResult) actionResult;
    ResultSetMetadata metadata = resultSet.getResultSetMetadata();
    DynamicMessage.Builder keyBuilder = metadata.newKeyBuilder();
    DynamicMessage.Builder valueBuilder = metadata.newValueBuilder();

    ProtoSerde protoSerde = ProtoSerdeFactory.createSerde(PayloadSerdes.PB_BINARY_VALUE);
    for (int i = 0; i < result.getPairCount(); ++ i) {
      KeyValuePair pair = result.getPair(i);
      
      protoSerde.deserializeFromByteArray(keyBuilder, pair.getKey().toByteArray());
      DynamicMessage key = keyBuilder.build();
      
      protoSerde.deserializeFromByteArray(valueBuilder, pair.getValue().toByteArray());
      DynamicMessage value = valueBuilder.build();

      resultSet.addResult(key, value);
    }
  }

  @Override
  protected ClientActorMessage buildRequest() throws IdgsException {
    TopNActionRequest request = topNBuilder.build();
    params.put(ServerConst.ACTION_PARAM, request);
    return super.buildRequest();
  }

}
