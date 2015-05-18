/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.execution;

import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import protubuf.MessageHelper;

import com.google.protobuf.ByteString;
import com.google.protobuf.Descriptors.Descriptor;
import com.google.protobuf.Descriptors.FieldDescriptor;
import com.google.protobuf.DynamicMessage;

import idgs.client.ClientActorMessage;
import idgs.exception.IdgsException;
import idgs.store.pb.PbStoreConfig.StoreConfig;
import idgs.store.pb.PbStoreService.InsertRequest;
import idgs.store.pb.PbStoreService.InsertResponse;
import idgs.store.pb.PbStoreService.StoreResultCode;
import idgs.util.ServerConst;

public class InsertOperator extends IdgsOperator {

  private static Log LOG = LogFactory.getLog(InsertOperator.class);
  
  private StoreConfig storeConfig;
  
  private DynamicMessage key;
  
  private DynamicMessage value;
  
  public InsertOperator(StoreConfig storeConfig) {
    this.storeConfig = storeConfig;
  }
  
  public void parseRow(DynamicMessage result) throws IdgsException {
    if (value == null) {
      throw new IdgsException("Data would be insert is null");
    }
    
    String keyType = storeConfig.getKeyType();
    String valueType = storeConfig.getValueType();
    
    Descriptor keyDescriptor = MessageHelper.getMessageDescriptor(keyType);
    Descriptor valueDescriptor = MessageHelper.getMessageDescriptor(valueType);
    Descriptor resultDescritpor = result.getDescriptorForType();
    
    if (keyDescriptor != key.getDescriptorForType()) {
      throw new IdgsException("Wrong object type used with protocol message reflection with key type " + keyType);
    }
    
    if (valueDescriptor != value.getDescriptorForType()) {
      throw new IdgsException("Wrong object type used with protocol message reflection with value type " + valueType);
    }
    
    List<FieldDescriptor> keyFields = keyDescriptor.getFields();
    List<FieldDescriptor> valueFields = valueDescriptor.getFields();
    List<FieldDescriptor> resultFields = resultDescritpor.getFields();
    
    if (keyFields.size() + valueFields.size() != resultFields.size()) {
      throw new IdgsException("Data and schema of store " + storeConfig.getName() + " cannot matched");
    }
    
    DynamicMessage.Builder keyBuilder = DynamicMessage.newBuilder(keyDescriptor);
    DynamicMessage.Builder valueBuilder = DynamicMessage.newBuilder(valueDescriptor);
    
    int fieldIndex = 0;
    for (int i = 0; i < keyFields.size(); ++ i, ++ fieldIndex) {
      FieldDescriptor field = keyFields.get(i);
      FieldDescriptor resultField = resultFields.get(fieldIndex);
      if (field.getJavaType() != resultField.getJavaType()) {
        throw new IdgsException("Wrong object type used with protocol message reflection, field " 
            + field.getName() + " + need type " + field.getJavaType().toString());
      }
      Object value = result.getField(resultField);
      keyBuilder.setField(field, value);
    }
    
    for (int i = 0; i < valueFields.size(); ++ i, ++ fieldIndex) {
      FieldDescriptor field = valueFields.get(i);
      FieldDescriptor resultField = resultFields.get(fieldIndex);
      if (field.getJavaType() != resultField.getJavaType()) {
        throw new IdgsException("Wrong object type used with protocol message reflection, field " 
            + field.getName() + " + need type " + field.getJavaType().toString());
      }
      Object value = result.getField(resultField);
      keyBuilder.setField(field, value);
    }
    
    key = keyBuilder.build();
    value = valueBuilder.build();
    
    params.put(ServerConst.STORE_ATTACH_KEY, key);
    params.put(ServerConst.STORE_ATTACH_VALUE, value);
  }
  
  public void parseRow(String row) throws IdgsException {
    if (row == null || row.isEmpty()) {
      throw new IdgsException("No data found.");
    }
    
    String keyType = storeConfig.getKeyType();
    String valueType = storeConfig.getValueType();
    String sepertor = storeConfig.getFieldSeperator();
    String[] fields = row.split(sepertor);
    
    if (!MessageHelper.isMessageRegistered(keyType)) {
      throw new IdgsException("Key " + keyType + " is not registered yet.");
    }
    
    if (!MessageHelper.isMessageRegistered(valueType)) {
      throw new IdgsException("Value " + valueType + " is not registered yet.");
    }
    
    Descriptor keyDescriptor = MessageHelper.getMessageDescriptor(keyType);
    Descriptor valueDescriptor = MessageHelper.getMessageDescriptor(valueType);
    
    List<FieldDescriptor> keyFields = keyDescriptor.getFields();
    List<FieldDescriptor> valueFields = valueDescriptor.getFields();
    
    if (keyFields.size() + valueFields.size() != fields.length) {
      throw new IdgsException("Data and schema of store " + storeConfig.getName() + " cannot matched");
    }
    
    DynamicMessage.Builder keyBuilder = DynamicMessage.newBuilder(keyDescriptor);
    DynamicMessage.Builder valueBuilder = DynamicMessage.newBuilder(valueDescriptor);
    
    int fieldIndex = 0;
    for (int i = 0; i < keyFields.size(); ++ i, ++ fieldIndex) {
      FieldDescriptor field = keyFields.get(i);
      if (fields[fieldIndex].isEmpty() && !field.isRequired()) {
        continue;
      }
      Object value = getFieldValue(field, fields[fieldIndex]);
      keyBuilder.setField(field, value);
    }
    
    for (int i = 0; i < valueFields.size(); ++ i, ++ fieldIndex) {
      FieldDescriptor field = valueFields.get(i);
      if (fields[fieldIndex].isEmpty() && !field.isRequired()) {
        continue;
      }
      Object value = getFieldValue(field, fields[fieldIndex]);
      valueBuilder.setField(field, value);
    }
    
    key = keyBuilder.build();
    value = valueBuilder.build();
    
    params.put(ServerConst.STORE_ATTACH_KEY, key);
    params.put(ServerConst.STORE_ATTACH_VALUE, value);
  }
  
  private Object getFieldValue(FieldDescriptor field, String stringValue) throws IdgsException {
    Object value = null;
    String errMsg = "Wrong object type used with protocol message reflection, field " 
          + field.getName() + " + need type " + field.getJavaType().toString();
    
    switch (field.getJavaType()) {
    case INT:
      if (!isNumeric(stringValue)) {
        throw new IdgsException(errMsg);
      }
      value = Integer.valueOf(stringValue);
      break;
    case LONG:
      if (!isNumeric(stringValue)) {
        throw new IdgsException(errMsg);
      }
      value = Long.valueOf(stringValue);
      break;
    case FLOAT:
      if (!isNumeric(stringValue)) {
        throw new IdgsException(errMsg);
      }
      value = Float.valueOf(stringValue);
      break;
    case DOUBLE:
      if (!isNumeric(stringValue)) {
        throw new IdgsException(errMsg);
      }
      value = Double.valueOf(stringValue);
      break;
    case BOOLEAN:
      value = Boolean.valueOf(stringValue);
      break;
    case STRING:
      value = stringValue;
      break;
    case BYTE_STRING:
      value = ByteString.copyFrom(stringValue.getBytes());
      break;
    case ENUM:
      if (!isNumeric(stringValue)) {
        value = field.getEnumType().findValueByName(stringValue);
      } else {
        value = field.getEnumType().findValueByNumber(Integer.valueOf(stringValue));
      }
      
      if (value == null) {
        throw new IdgsException(errMsg);
      }
      break;
    case MESSAGE:
      throw new IdgsException("Not supported Message type yet.");
    default:
      break;
    }
    
    return value;
  }
  
  private boolean isNumeric(String str) {
    return str.matches("^[-+]?(([0-9]+)([.]([0-9]+))?|([.]([0-9]+))?)$");
  }
  
  @Override
  protected String getName() {
    return "insert";
  }
  
  @Override
  protected ClientActorMessage buildRequest() throws IdgsException {
    if (key == null || value == null) {
      throw new IdgsException("Invalid data.");
    }
    
    InsertRequest.Builder builder = InsertRequest.newBuilder();
    builder.setStoreName(storeConfig.getName());
    InsertRequest request = builder.build();
    
    ClientActorMessage requestMsg = buildStoreRequestMessage(ServerConst.INSERT_STORE_REQUEST, request);
    if (LOG.isDebugEnabled()) {
      LOG.debug("insert into store " + storeConfig.getName() + " ===> " + requestMsg.toString());
    }
    
    return requestMsg;
  }

  @Override
  protected void processResponse(ClientActorMessage responseMsg) throws IdgsException {
    InsertResponse.Builder builder = InsertResponse.newBuilder();
    responseMsg.parsePayload(builder);
    InsertResponse response = builder.build();
    
    if (response.getResultCode() != StoreResultCode.SRC_SUCCESS) {
      throw new IdgsException("insert data error, caused by " + response.getResultCode().toString());
    }
  }

}
