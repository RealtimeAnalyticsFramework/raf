/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.execution;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.google.protobuf.Message;

import idgs.client.ClientActorMessage;
import idgs.client.TcpClientInterface;
import idgs.client.TcpClientPool;
import idgs.exception.IdgsException;
import idgs.pb.PbExpr.DataType;
import idgs.pb.PbExpr.Expr;
import idgs.pb.PbExpr.ExpressionType;
import idgs.pb.PbRpcMessage.MemberIdConst;
import idgs.rdd.pb.PbRddCommon.RddResultCode;
import idgs.rdd.pb.PbRddService.CreateRddRequest;
import idgs.rdd.pb.PbRddService.CreateRddResponse;
import idgs.rdd.pb.PbRddService.FieldNamePair;
import idgs.rdd.pb.PbRddService.InRddInfo;
import idgs.rdd.pb.PbRddService.OutRddInfo;
import idgs.rdd.pb.PbRddService.PersistType;
import idgs.rdd.pb.PbRddService.RddField;
import idgs.util.ServerConst;

public abstract class TransformerOperator extends IdgsOperator {

  private static Log LOG = LogFactory.getLog(TransformerOperator.class);
  
  private static Map<DataType, String> defaultValueMap;
  
  protected CreateRddRequest.Builder builder;
  
  private String opName;
  
  static {
    defaultValueMap = new HashMap<DataType, String>();
    defaultValueMap.put(DataType.BOOL, "false");
    defaultValueMap.put(DataType.BYTES, "");
    defaultValueMap.put(DataType.DOUBLE, "0");
    defaultValueMap.put(DataType.ENUM, "0");
    defaultValueMap.put(DataType.FLOAT, "0");
    defaultValueMap.put(DataType.INT32, "0");
    defaultValueMap.put(DataType.INT64, "0");
    defaultValueMap.put(DataType.STRING, "");
    defaultValueMap.put(DataType.UINT32, "0");
    defaultValueMap.put(DataType.UINT64, "0");
  }
  
  protected TransformerOperator(String opName) {
    this.opName = opName;
  }
  
  @Override
  protected void buildRequest() {
    if (children.size() == 0) {
      return;
    }
    
    builder = CreateRddRequest.newBuilder();
    
    builder.setTransformerOpName(opName);
    
    OutRddInfo.Builder outBuilder = builder.getOutRddBuilder();
    outBuilder.setRddName(getRddName());
    outBuilder.setDataType(PersistType.ORDERED);

    List<Map<String, FieldNamePair>> inKeyFields = new ArrayList<Map<String, FieldNamePair>>();
    Map<String, String> keyFields = new HashMap<String, String>();
    Map<String, String> valueFields = new HashMap<String, String>();
    List<FieldNamePair> keyFieldList = new ArrayList<FieldNamePair>();
    List<FieldNamePair> valueFieldList = new ArrayList<FieldNamePair>();
    for (int i = 0; i < children.size(); ++ i) {
      IdgsOperator childOp = children.get(i);
      InRddInfo.Builder inBuilder = builder.addInRddBuilder();
      inBuilder.setRddName(childOp.getRddName());
      
      Expr filter = childOp.getFilterExpr();
      if (filter != null) {
        inBuilder.setFilterExpr(filter);
      }
      
      Map<String, FieldNamePair> inKeyField = new HashMap<String, FieldNamePair>();
      List<FieldNamePair> outputKeyField = childOp.getOutputKeyFields();
      if (outputKeyField != null) {
        for (FieldNamePair field : outputKeyField) {
          inBuilder.addOutFields(field);
          inKeyField.put(field.getFieldAlias(), field);
          if (!keyFields.containsKey(field.getFieldAlias())) {
            keyFields.put(field.getFieldAlias(), null);
            keyFieldList.add(field);
          }
        }
      }
      inKeyFields.add(inKeyField);
      
      List<FieldNamePair> value = childOp.getOutputValueFields();
      if (value != null) {
        for (FieldNamePair field : value) {
          inBuilder.addOutFields(field);
          if (!valueFields.containsKey(field.getFieldAlias())) {
            valueFields.put(field.getFieldAlias(), null);
            valueFieldList.add(field);
          }
        }
      }
    }
    
    if (children.size() > 1) {
      for (int i = 0; i < inKeyFields.size(); ++ i) {
        Map<String, FieldNamePair> in = inKeyFields.get(i);
        if (in.size() != keyFieldList.size()) {
          for (FieldNamePair field : keyFieldList) {
            if (!in.containsKey(field.getFieldAlias())) {
              FieldNamePair.Builder fieldBuilder = FieldNamePair.newBuilder();
              fieldBuilder.setFieldAlias(field.getFieldAlias());
              fieldBuilder.setFieldType(field.getFieldType());
              fieldBuilder.getExprBuilder().setType(ExpressionType.CONST);
              fieldBuilder.getExprBuilder().setValue(defaultValueMap.get(field.getFieldType()));
              builder.getInRddBuilder(i).addOutFields(fieldBuilder);
            }
          }
        }
      }
    }
    
    if (keyFieldList.isEmpty() && children.size() == 1) {
      if (keyType == null) {
        keyType = children.get(0).getKeyType();
      }
    } else {
      keyType = genTypeName(children.get(0).getKeyType(), "KEY");
      for (FieldNamePair field : keyFieldList) {
        RddField.Builder fieldBuilder = outBuilder.addKeyFieldsBuilder();
        fieldBuilder.setFieldName(field.getFieldAlias());
        fieldBuilder.setFieldType(field.getFieldType());
      }
    }
    
    if (valueFieldList.isEmpty() && children.size() == 1) {
      if (valueType == null) {
        valueType = children.get(0).getValueType();
      }
    } else {
      valueType = genTypeName(children.get(0).getValueType(), "VALUE");
      for (FieldNamePair field : valueFieldList) {
        RddField.Builder fieldBuilder = outBuilder.addValueFieldsBuilder();
        fieldBuilder.setFieldName(field.getFieldAlias());
        fieldBuilder.setFieldType(field.getFieldType());
      }
    }
    
    outBuilder.setKeyTypeName(keyType);
    outBuilder.setValueTypeName(valueType);
  }
  
  @Override
  protected void processOp() throws IdgsException {
    TcpClientInterface client = null;
    
    try {
      client = TcpClientPool.getInstance().getClient();
      
      CreateRddRequest request = builder.build();
      
      ClientActorMessage reqMsg = getDefaultClientActorMessage();
      reqMsg.setOperationName(ServerConst.CREATE_RDD);
      reqMsg.setDestActorId(ServerConst.RDD_SERVICE_ACTOR);
      reqMsg.setDestMemberId(MemberIdConst.ANY_MEMBER_VALUE);
      reqMsg.setPayload(request);
      
      Iterator<Entry<String, Message>> it = params.entrySet().iterator();
      while (it.hasNext()) {
        Entry<String, Message> entry = it.next();
        reqMsg.setAttachment(entry.getKey(), entry.getValue());
      }
      
      if (LOG.isDebugEnabled()) {
        LOG.debug(request.getTransformerOpName() + " ===> " + reqMsg.toString());
      }
      
      ClientActorMessage respMsg = client.sendRecv(reqMsg);
      if (respMsg == null) {
        throw new IdgsException("no response found");
      }
      
      CreateRddResponse.Builder builder = CreateRddResponse.newBuilder();
      respMsg.parsePayload(builder);
      CreateRddResponse response = builder.build();
      
      if (response.getResultCode() != RddResultCode.RRC_SUCCESS) {
        throw new IdgsException("create transformer error, caused by code " + response.getResultCode());
      }
    } catch (Exception ex) {
      throw new IdgsException("send or receive message error", ex);
    } finally {
      if (client != null) {
        client.close();
      }
    }
  }
  
  private String genTypeName(String lastType, String prefix) {
    int pos = lastType.lastIndexOf(".");
    String package_ = null;
    if (pos == -1) {
      package_ = lastType;
    } else {
      package_ = lastType.substring(0, pos);
    }
    
    return package_ + "." + prefix + "_TYPE_" + getRddName();
  }
  
}
