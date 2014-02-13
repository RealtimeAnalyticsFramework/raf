/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.execution;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Iterator;
import java.util.Map.Entry;
import java.util.UUID;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import protubuf.MessageHelper;

import com.google.protobuf.Descriptors.Descriptor;
import com.google.protobuf.Message;

import idgs.client.ClientActorMessage;
import idgs.client.TcpClientInterface;
import idgs.client.TcpClientPool;
import idgs.exception.IdgsException;
import idgs.pb.PbExpr.Expr;
import idgs.pb.PbRpcMessage.MemberIdConst;
import idgs.rdd.pb.PbRddCommon.RddResultCode;
import idgs.rdd.pb.PbRddService.ActionRequest;
import idgs.rdd.pb.PbRddService.ActionResponse;
import idgs.store.pb.PbMetadata.Field;
import idgs.store.pb.PbMetadata.Metadata;
import idgs.store.pb.PbMetadata.MetadataPair;
import idgs.util.TypeUtils;
import idgs.util.ServerConst;

public abstract class ActionOperator extends IdgsOperator {

  private static Log LOG = LogFactory.getLog(ActionOperator.class);
  
  protected ActionRequest.Builder builder;
  
  protected Message.Builder resultBuilder;
  
  protected ResultSet resultSet;
  
  private String opName;
  
  protected ActionOperator(String opName, Message.Builder resultBuilder) {
    this.opName = opName;
    this.resultBuilder = resultBuilder;
  }
  
  @Override
  protected void buildRequest() {
    builder = ActionRequest.newBuilder();
    builder.setActionId(getRddName());
    builder.setActionOpName(opName);
    
    IdgsOperator operator = children.get(0);
    builder.setRddName(operator.getRddName());
    
    Expr filter = operator.getFilterExpr();
    if (operator.getFilterExpr() != null) {
      builder.setFilter(filter);
    }
  }

  @Override
  protected void processOp() throws IdgsException {
    ActionRequest request = builder.build();
    
    ClientActorMessage reqMsg = getDefaultClientActorMessage();
    reqMsg.setOperationName(ServerConst.RDD_ACTION_REQUEST);
    reqMsg.setDestActorId(ServerConst.RDD_SERVICE_ACTOR);
    reqMsg.setDestMemberId(MemberIdConst.ANY_MEMBER_VALUE);
    reqMsg.setPayload(request);
    
    Iterator<Entry<String, Message>> it = params.entrySet().iterator();
    while (it.hasNext()) {
      Entry<String, Message> entry = it.next();
      reqMsg.setAttachment(entry.getKey(), entry.getValue());
    }
    
    if (LOG.isDebugEnabled()) {
      LOG.debug(request.getActionOpName() + " ===> " + reqMsg.toString());
    }
    
    TcpClientInterface client = null;
    ClientActorMessage respMsg = null;
    try {
      client = TcpClientPool.getInstance().getClient();
      respMsg = client.sendRecv(reqMsg);
    } catch (Exception ex) {
      throw new IdgsException("send or receive message error", ex);
    } finally {
      if (client != null) {
        client.close();
      }
    }
    if (respMsg == null) {
      throw new IdgsException("no response found");
    }

    ActionResponse.Builder builder = ActionResponse.newBuilder();
    if (!respMsg.parsePayload(builder)) {
      throw new IdgsException("cannot parse payload of response");
    }
    ActionResponse response = builder.build();
    
    if (response.getResultCode() != RddResultCode.RRC_SUCCESS) {
      throw new IdgsException("execute action error, caused by code " + response.getResultCode());
    }
    
    MetadataPair.Builder metadataBuilder = MetadataPair.newBuilder();
    if (!respMsg.parseAttachment(ServerConst.METADATA, metadataBuilder)) {
      throw new IdgsException("cannot parse metadata of response");
    }
    MetadataPair metadata = metadataBuilder.build();
    
    String keyType = metadata.getKey().getTypeName();
    if (!MessageHelper.isMessageRegistered(keyType)) {
      registerMetadata(metadata.getKey());
    }
    
    String valueType = metadata.getValue().getTypeName();
    if (!MessageHelper.isMessageRegistered(valueType)) {
      registerMetadata(metadata.getValue());
    }
    
    Descriptor keyDescriptor = MessageHelper.getMessageDescriptor(keyType);
    Descriptor valueDescriptor = MessageHelper.getMessageDescriptor(valueType);
    resultSet = new ResultSet(keyDescriptor, valueDescriptor);
    
    if (!respMsg.parseAttachment(ServerConst.ACTION_RESULT, resultBuilder)) {
      throw new IdgsException("cannot parse action result of response");
    }
    Message actionResult = resultBuilder.build();
    
    handleResult(actionResult, resultSet);
  }
  
  private void registerMetadata(Metadata metadata) throws IdgsException {
    StringBuffer sb = new StringBuffer();
    String fullTypeName = metadata.getTypeName();
    int pos = fullTypeName.lastIndexOf(".");
    String package_ = fullTypeName.substring(0, pos);
    String typeName = fullTypeName.substring(pos + 1, fullTypeName.length());
    
    if (package_ != null && !package_.equals("")) {
      sb.append("package ").append(package_).append(";\n");
    }
    sb.append("message ").append(typeName).append(" {\n");
    for (int i = 0; i < metadata.getFieldCount(); ++ i) {
      Field field = metadata.getField(i);
      sb.append("  ").append(field.getLabel().name().toLowerCase().replace("label_", ""))
        .append(" ").append(TypeUtils.dataTypeToProto(field.getType()))
        .append(" ").append(field.getName()).append(" = ")
        .append(field.getNumber()).append(";\n");
    }
    sb.append("}\n");
    
    String fileName = System.getProperty("java.io.tmpdir") + "/" + UUID.randomUUID().toString().replace("-", "_").toUpperCase() + ".proto";
    File file = new File(fileName);
    FileOutputStream fos = null;
    try {
      fos = new FileOutputStream(file);
      fos.write(sb.toString().getBytes());
    } catch (IOException e) {
      throw new IdgsException("cannot make proto file from metadata.", e);
    } finally {
      if (fos != null) {
        try {
          fos.close();
        } catch (IOException e) {
          throw new IdgsException(e);
        }
      }
    }
    
    try {
      MessageHelper.registerMessage(fileName);
    } catch (IdgsException e) {
      throw e;
    } finally {
      if (file.exists()) {
        file.delete();
      }
    }
  }
  
  public ResultSet getResultSet() {
    return resultSet;
  }
  
  protected abstract void handleResult(Message actionResult, ResultSet resultSet);
  
}
