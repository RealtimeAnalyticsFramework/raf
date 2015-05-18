/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.execution;

import java.net.SocketTimeoutException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import java.util.Map.Entry;

import com.google.protobuf.Message;

import idgs.client.ClientActorMessage;
import idgs.client.TcpClientInterface;
import idgs.client.TcpClientPool;
import idgs.exception.IdgsException;
import idgs.pb.PbExpr.Expr;
import idgs.pb.PbRpcMessage.MemberIdConst;
import idgs.pb.PbRpcMessage.PayloadSerdes;
import idgs.pb.PbRpcMessage.TransportChannel;
import idgs.rdd.pb.PbRddCommon.RddResultCode;
import idgs.rdd.pb.PbRddInternal.RddResponse;
import idgs.rdd.pb.PbRddService.DestroyRddRequest;
import idgs.rdd.pb.PbRddService.FieldNamePair;
import idgs.util.ServerConst;

public abstract class IdgsOperator implements Cloneable {
  
  private static final String tab = "  ";
  
  protected Map<String, Message> params;
  
  protected List<IdgsOperator> children;
  
  private String rddName;
  
  private List<FieldNamePair> outputKeyFields;
  
  private List<FieldNamePair> outputValueFields;
  
  private Expr filterExpr;
  
  protected String keyType;
  
  protected String valueType;
  
  public IdgsOperator() {
    params = new HashMap<String, Message>();
    genRddName();
  }
  
  protected abstract String getName();
  
  protected abstract ClientActorMessage buildRequest() throws IdgsException;
  
  protected abstract void processResponse(ClientActorMessage responseMsg) throws IdgsException;
  
  public IdgsOperator clone() {
    Object o = null;
    try {
      o = super.clone();
    } catch (CloneNotSupportedException e) {
      e.printStackTrace();
    }
    
    IdgsOperator op = (IdgsOperator) o;
    op.genRddName();
    return (IdgsOperator) op;
  }
  
  public void genRddName() {
    String randName = UUID.randomUUID().toString().toUpperCase().replaceAll("-", "_");
    rddName = getName() + "_" + randName;
  }

  public void process() throws IdgsException {
    ClientActorMessage requestMsg = buildRequest();
    ClientActorMessage responseMsg = sendRecv(requestMsg);
    processResponse(responseMsg);
  }
  
  private ClientActorMessage sendRecv(ClientActorMessage requestMsg) throws IdgsException {
    if (requestMsg == null) {
      throw new IdgsException("Invalid request message.");
    }
    
    TcpClientInterface client = TcpClientPool.getInstance().getClient();
    if (client == null) {
      throw new IdgsException("Cannot connect to server. ");
    }
    
    ClientActorMessage responseMsg = null;
    try {
      responseMsg = client.sendRecv(requestMsg, ServerConst.timeout);
    } catch (SocketTimeoutException ex) {
      throw new IdgsException("Timeout for RDD " + rddName + " when running " + getName() + " operator.", ex);
    } catch (Exception ex) {
      throw new IdgsException("Send or receive message error", ex);
    } finally {
      if (client != null) {
        client.close();
      }
    }
    
    if (responseMsg == null) {
      throw new IdgsException("No response found for RDD " + rddName);
    }
    
    return responseMsg;
  }
  
  public void destroy() throws IdgsException {
    DestroyRddRequest.Builder builder = DestroyRddRequest.newBuilder();
    builder.setRddName(rddName);

    DestroyRddRequest request = builder.build();
    ClientActorMessage requestMsg = buildRddRequestMessage(ServerConst.RDD_DESTROY, request);
    
    ClientActorMessage responseMsg = sendRecv(requestMsg);
    
    RddResponse.Builder responseBuilder = RddResponse.newBuilder();
    if (!responseMsg.parsePayload(responseBuilder)) {
      String err = "cannot parse payload of response for destroy of RDD " + getRddName();
      throw new IdgsException(err);
    }
    
    RddResponse response = responseBuilder.build();
    if (response.getResultCode() != RddResultCode.RRC_SUCCESS) {
      String err = "execute destory RDD " + getRddName() + " error, caused by " + response.getResultCode().toString();
      throw new IdgsException(err);
    }
  }
  
  public void addChildOperator(IdgsOperator operator) {
    if (children == null) {
      children = new ArrayList<IdgsOperator>();
    }
    children.add(operator);
  }
  
  public void setChildrenOperators(List<IdgsOperator> children) {
    this.children = children;
  }
  
  public List<IdgsOperator> getChildrenOperators() {
    return children;
  }
  
  public void clearOperator() {
    children.clear();
  }
  
  public void setOutputKeyFields(List<FieldNamePair> outputKeyFields) {
    this.outputKeyFields = outputKeyFields;
  }
  
  public void addOutputKeyFields(FieldNamePair outputKeyField) {
    if (outputKeyFields == null) {
      outputKeyFields = new ArrayList<FieldNamePair>(); 
    }
    this.outputKeyFields.add(outputKeyField);
  }
  
  public List<FieldNamePair> getOutputKeyFields() {
    return outputKeyFields;
  }
  
  public void setOutputValueFields(List<FieldNamePair> outputValueFields) {
    this.outputValueFields = outputValueFields;
  }
  
  public void addOutputValueFields(FieldNamePair outputValueField) {
    if (outputValueFields == null) {
      outputValueFields = new ArrayList<FieldNamePair>(); 
    }
    this.outputValueFields.add(outputValueField);
  }
  
  public List<FieldNamePair> getOutputValueFields() {
    return outputValueFields;
  }
  
  public void setFilterExpr(Expr expr) {
    this.filterExpr = expr;
  }
  
  public Expr getFilterExpr() {
    return filterExpr;
  }
  
  public ClientActorMessage buildStoreRequestMessage(String operationName, Message payload) {
    return buildRequestMessage(operationName, ServerConst.STORE_SERVICE_ACTOR, payload);
  }
  
  public ClientActorMessage buildRddRequestMessage(String operationName, Message payload) {
    return buildRequestMessage(operationName, ServerConst.RDD_SERVICE_ACTOR, payload);
  }
  
  public ClientActorMessage buildRequestMessage(String operationName, String destActorId, Message payload) {
    ClientActorMessage message = new ClientActorMessage();
    message.setOperationName(operationName);
    message.setSourceActorId("client_actor_id");
    message.setSourceMemberId(MemberIdConst.CLIENT_MEMBER_VALUE);
    message.setDestMemberId(MemberIdConst.ANY_MEMBER_VALUE);
    message.setDestActorId(destActorId);
    message.setChannel(TransportChannel.TC_TCP);
    message.setSerdesType(PayloadSerdes.PB_BINARY);
    message.setPayload(payload);
    
    if (!params.isEmpty()) {
      Iterator<Entry<String, Message>> it = params.entrySet().iterator();
      while (it.hasNext()) {
        Entry<String, Message> entry = it.next();
        message.setAttachment(entry.getKey(), entry.getValue());
      }
    }
    
    return message;
  }
  
  public void addAttachment(String name, Message value) {
    params.put(name, value);
  }
  
  public void setRddName(String rddName) {
    this.rddName = rddName;
  }
  
  public String getRddName() {
    return rddName;
  }

  @Override
  public String toString() {
    StringBuffer sb = new StringBuffer();
    
    sb.append(getClass().getSimpleName()).append("[").append(rddName).append("]");
    
    if (children != null && !children.isEmpty()) {
      sb.append("(");
      for (IdgsOperator op : children) {
        sb.append(op).append(" ");
      }
      sb.append(")");
    }
    
    return sb.toString();
  }
  
  public String toTreeString() {
    return toTreeString("");
  }
  
  private String toTreeString(String space) {
    StringBuffer sb = new StringBuffer();
    
    sb.append(space).append(getClass().getSimpleName()).append("[").append(rddName).append("]\n");
    
    if (children != null && !children.isEmpty()) {
      for (IdgsOperator op : children) {
        sb.append(op.toTreeString(space + tab));
      }
    }
    
    return sb.toString();
  }
  
  public String getKeyType() {
    return keyType;
  }

  public String getValueType() {
    return valueType;
  }

  public void setKeyType(String keyType) {
    this.keyType = keyType;
  }

  public void setValueType(String valueType) {
    this.valueType = valueType;
  }
  
}
