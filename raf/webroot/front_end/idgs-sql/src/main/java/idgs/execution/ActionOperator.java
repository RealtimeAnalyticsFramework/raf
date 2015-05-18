/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.execution;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import protubuf.MessageHelper;

import com.google.protobuf.DescriptorProtos.FileDescriptorProto;
import com.google.protobuf.Descriptors.Descriptor;
import com.google.protobuf.Message;

import idgs.client.ClientActorMessage;
import idgs.exception.IdgsException;
import idgs.pb.PbExpr.Expr;
import idgs.rdd.pb.PbRddCommon.RddResultCode;
import idgs.rdd.pb.PbRddService.ActionRequest;
import idgs.rdd.pb.PbRddService.ActionResponse;
import idgs.util.ServerConst;

public abstract class ActionOperator extends IdgsOperator {

  private static Log LOG = LogFactory.getLog(ActionOperator.class);
  
  protected Message.Builder resultBuilder;
  
  protected ResultSet resultSet;
  
  private String opName;
  
  private long limit;
  
  protected ActionOperator(String opName, Message.Builder resultBuilder) {
    this.opName = opName;
    this.resultBuilder = resultBuilder;
    limit = -1;
  }
  
  protected abstract void handleResult(Message actionResult, ResultSet resultSet);
  
  @Override
  protected ClientActorMessage buildRequest() throws IdgsException {
    ActionRequest.Builder builder = ActionRequest.newBuilder();
    builder.setActionId(getRddName());
    builder.setActionOpName(opName);
    if (limit > -1) {
      builder.setLimit(limit);
    }
    
    IdgsOperator operator = children.get(0);
    builder.setRddName(operator.getRddName());
    setRddName(operator.getRddName());
    
    Expr filter = operator.getFilterExpr();
    if (operator.getFilterExpr() != null) {
      builder.setFilter(filter);
    }
    
    ActionRequest request = builder.build();
    
    ClientActorMessage requestMsg = buildRddRequestMessage(ServerConst.RDD_ACTION_REQUEST, request);
    if (LOG.isDebugEnabled()) {
      LOG.debug(request.getActionOpName() + " ===> " + requestMsg.toString());
    }
    
    return requestMsg;
  }

  @Override
  protected void processResponse(ClientActorMessage responseMsg) throws IdgsException {
    LOG.debug("Got action response for RDD "+ getRddName() + ".");
    ActionResponse.Builder actionResponseBuilder = ActionResponse.newBuilder();
    if (!responseMsg.parsePayload(actionResponseBuilder)) {
      String err = "RDD " + getRddName() + " cannot parse payload of response";
      LOG.error(err);
      throw new IdgsException(err);
    }
    ActionResponse response = actionResponseBuilder.build();
    
    if (response.getResultCode() != RddResultCode.RRC_SUCCESS) {
      String err = "RDD " + getRddName() + " execute action error, caused by " + response.getResultCode().toString();
      LOG.error(err);
      throw new IdgsException(err);
    }
    
    FileDescriptorProto.Builder keyMetadataBuilder = FileDescriptorProto.newBuilder();
    if (!responseMsg.parseAttachment(ServerConst.KEY_METADATA, keyMetadataBuilder)) {
      String err = "RDD " + getRddName() + " cannot parse key metadata of response";
      LOG.error(err);
      throw new IdgsException(err);
    }
    FileDescriptorProto keyMetadata = keyMetadataBuilder.build();
    
    FileDescriptorProto.Builder valueMetadataBuilder = FileDescriptorProto.newBuilder();
    if (!responseMsg.parseAttachment(ServerConst.VALUE_METADATA, valueMetadataBuilder)) {
      String err = "RDD " + getRddName() + " cannot parse value metadata of response";
      LOG.error(err);
      throw new IdgsException(err);
    }
    FileDescriptorProto valueMetadata = valueMetadataBuilder.build();
    
    String keyType = keyMetadata.getMessageType(0).getName();
    String keyPackage = keyMetadata.getPackage();
    if (keyPackage != null && !keyPackage.equals("")) {
      keyType = keyPackage + "." + keyType;
    }
    if (!MessageHelper.isMessageRegistered(keyType)) {
      MessageHelper.registerMessage(keyMetadata);
    }

    String valueType = valueMetadata.getMessageType(0).getName();
    String valuePackage = valueMetadata.getPackage();
    if (valuePackage!= null && !valuePackage.equals("")) {
      valueType = valuePackage + "." + valueType;
    }
    if (!MessageHelper.isMessageRegistered(valueType)) {
      MessageHelper.registerMessage(valueMetadata);
    }
    
    Descriptor keyDescriptor = MessageHelper.getMessageDescriptor(keyType);
    Descriptor valueDescriptor = MessageHelper.getMessageDescriptor(valueType);
    resultSet = new ResultSet(keyDescriptor, valueDescriptor);
    
    if (!responseMsg.parseAttachment(ServerConst.ACTION_RESULT, resultBuilder)) {
      String err = "RDD " + getRddName() + " cannot parse action result of response";
      LOG.error(err);
      throw new IdgsException(err);
    }
    Message actionResult = resultBuilder.build();
    
    handleResult(actionResult, resultSet);
  }
  
  public void setLimit(long limit) {
    this.limit = limit;
  }
  
  public ResultSet getResultSet() {
    return resultSet;
  }
  
}
