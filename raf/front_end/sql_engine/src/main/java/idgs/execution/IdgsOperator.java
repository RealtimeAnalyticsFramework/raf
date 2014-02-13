/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.execution;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

import com.google.protobuf.Message;

import idgs.client.ClientActorMessage;
import idgs.exception.IdgsException;
import idgs.pb.PbExpr.Expr;
import idgs.pb.PbRpcMessage.MemberIdConst;
import idgs.pb.PbRpcMessage.PayloadSerdes;
import idgs.pb.PbRpcMessage.TransportChannel;
import idgs.rdd.pb.PbRddService.FieldNamePair;

public abstract class IdgsOperator {
  
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
    String randName = UUID.randomUUID().toString().toUpperCase().replaceAll("-", "_");
    rddName = getName() + "_" + randName;
  }
  
  protected abstract String getName();
  
  protected abstract void buildRequest();
  
  protected abstract void processOp() throws IdgsException;
  
  public void process() throws IdgsException {
    buildRequest();
    processOp();
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
  
  public List<FieldNamePair> getOutputKeyFields() {
    return outputKeyFields;
  }
  
  public void setOutputValueFields(List<FieldNamePair> outputValueFields) {
    this.outputValueFields = outputValueFields;
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
  
  public ClientActorMessage getDefaultClientActorMessage() {
    ClientActorMessage message = new ClientActorMessage();
    message.setSourceActorId("client_actor_id");
    message.setSourceMemberId(MemberIdConst.CLIENT_MEMBER_VALUE);
    message.setChannel(TransportChannel.TC_TCP);
    message.setSerdesType(PayloadSerdes.PB_BINARY);
    
    return message;
  }
  
  public void addAttachment(String name, Message value) {
    params.put(name, value);
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
