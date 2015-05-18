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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import idgs.client.ClientActorMessage;
import idgs.exception.IdgsException;
import idgs.parse.ExprFactory;
import idgs.pb.PbExpr.DataType;
import idgs.pb.PbExpr.Expr;
import idgs.rdd.pb.PbRddCommon.RddResultCode;
import idgs.rdd.pb.PbRddService.CreateRddRequest;
import idgs.rdd.pb.PbRddService.CreateRddResponse;
import idgs.rdd.pb.PbRddService.FieldNamePair;
import idgs.rdd.pb.PbRddService.InRddInfo;
import idgs.rdd.pb.PbRddService.OutRddInfo;
import idgs.rdd.pb.PbRddService.RddField;
import idgs.util.ServerConst;

public abstract class TransformerOperator extends IdgsOperator {

  private static Log LOG = LogFactory.getLog(TransformerOperator.class);
  
  private static Map<DataType, String> defaultValueMap;
  
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
  protected ClientActorMessage buildRequest() throws IdgsException {
    if (children == null || children.isEmpty()) {
      String err = opName + "transfomer " + getRddName() + " has no dependency RDD.";
      LOG.error(err);
      throw new IdgsException(err);
    }
    
    CreateRddRequest.Builder builder = CreateRddRequest.newBuilder();
    
    builder.setTransformerOpName(opName);
    
    OutRddInfo.Builder outBuilder = builder.getOutRddBuilder();
    outBuilder.setRddName(getRddName());

    String useKey = null, useValue = null;
    List<Map<String, FieldNamePair>> inKeyFields = new ArrayList<Map<String, FieldNamePair>>();
    Map<String, String> keyFields = new HashMap<String, String>();
    Map<String, String> valueFields = new HashMap<String, String>();
    List<FieldNamePair> keyFieldList = new ArrayList<FieldNamePair>();
    List<FieldNamePair> valueFieldList = new ArrayList<FieldNamePair>();
    for (int i = 0; i < children.size(); ++ i) {
      IdgsOperator childOp = children.get(i);
      if (useKey == null) {
        useKey = childOp.getKeyType();
      } else if (!useKey.equals(childOp.getKeyType())) {
        useKey = null;
      }
      
      if (useValue == null) {
        useValue = childOp.getValueType();
      } else if (!useValue.equals(childOp.getValueType())) {
        useValue = null;
      }
      
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
              fieldBuilder.getExprBuilder().setName(ExprFactory.CONST);
              fieldBuilder.getExprBuilder().setValue(defaultValueMap.get(field.getFieldType()));
              builder.getInRddBuilder(i).addOutFields(fieldBuilder);
            }
          }
        }
      }
    }
    
    if (keyFieldList.isEmpty() && useKey != null) {
      if (keyType == null) {
        keyType = useKey;
      }
    } else {
      keyType = ServerConst.DM_PACKAGE + "." + ServerConst.DM_KEY + "_" + getRddName();
      for (FieldNamePair field : keyFieldList) {
        RddField.Builder fieldBuilder = outBuilder.addKeyFieldsBuilder();
        fieldBuilder.setFieldName(field.getFieldAlias());
        fieldBuilder.setFieldType(field.getFieldType());
      }
    }
    
    if (valueFieldList.isEmpty() && useValue != null) {
      if (valueType == null) {
        valueType = useValue;
      }
    } else {
      valueType = ServerConst.DM_PACKAGE + "." + ServerConst.DM_VALUE + "_" + getRddName();
      for (FieldNamePair field : valueFieldList) {
        RddField.Builder fieldBuilder = outBuilder.addValueFieldsBuilder();
        fieldBuilder.setFieldName(field.getFieldAlias());
        fieldBuilder.setFieldType(field.getFieldType());
      }
    }
    
    outBuilder.setKeyTypeName(keyType);
    outBuilder.setValueTypeName(valueType);
    
    CreateRddRequest request = builder.build();
    
    ClientActorMessage requestMsg = buildRddRequestMessage(ServerConst.CREATE_RDD, request);
    if (LOG.isDebugEnabled()) {
      LOG.debug(request.getTransformerOpName() + " ===> " + requestMsg.toString());
    }
    
    return requestMsg;
  }
  
  @Override
  protected void processResponse(ClientActorMessage responseMsg) throws IdgsException {
    LOG.debug("Got transfomer response for RDD " + getRddName() + ".");
    CreateRddResponse.Builder builder = CreateRddResponse.newBuilder();
    responseMsg.parsePayload(builder);
    CreateRddResponse response = builder.build();
    
    if (response.getResultCode() != RddResultCode.RRC_SUCCESS) {
      String err = "create RDD " + getRddName() + " error, caused by " + response.getResultCode().toString();
      LOG.error(err);
      throw new IdgsException(err);
    }
  }
  
}

