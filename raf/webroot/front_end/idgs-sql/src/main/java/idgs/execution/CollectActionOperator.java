/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.execution;

import idgs.client.util.ProtoSerde;
import idgs.client.util.ProtoSerdeFactory;
import idgs.pb.PbRpcMessage.PayloadSerdes;
import idgs.rdd.pb.PbRddAction.CollectActionResult;
import idgs.rdd.pb.PbRddAction.KeyValuesPair;
import idgs.util.ServerConst;

import com.google.protobuf.ByteString;
import com.google.protobuf.DynamicMessage;
import com.google.protobuf.Message;

public class CollectActionOperator extends ActionOperator {
  
  public CollectActionOperator() {
    super(ServerConst.COLLECT_ACTION, CollectActionResult.newBuilder());
  }

  protected String getName() {
    return ServerConst.COLLECT;
  }
  
  @Override
  public void handleResult(Message actionResult, ResultSet resultSet) {
    CollectActionResult result = (CollectActionResult) actionResult;
    ResultSetMetadata metadata = resultSet.getResultSetMetadata();
    DynamicMessage.Builder keyBuilder = metadata.newKeyBuilder();
    DynamicMessage.Builder valueBuilder = metadata.newValueBuilder();

    ProtoSerde protoSerde = ProtoSerdeFactory.createSerde(PayloadSerdes.PB_BINARY_VALUE);
    for (int i = 0; i < result.getPairCount(); ++ i) {
      KeyValuesPair pair = result.getPair(i);
      protoSerde.deserializeFromByteArray(keyBuilder, pair.getKey().toByteArray());
      DynamicMessage key = keyBuilder.build();
      
      for (ByteString valuebuf : pair.getValueList()) {
        protoSerde.deserializeFromByteArray(valueBuilder, valuebuf.toByteArray());
        DynamicMessage value = valueBuilder.build();

        resultSet.addResult(key, value);
      }
    }
  }

}
