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

import com.google.protobuf.Descriptors.Descriptor;
import com.google.protobuf.DynamicMessage;
import com.google.protobuf.Descriptors.FieldDescriptor;

public class ResultSet {

  private Descriptor keyDescriptor;
  
  private Descriptor valueDescriptor;
  
  private Map<String, FieldDescriptor> keyFieldDescriptors;
  
  private Map<String, FieldDescriptor> valueFieldDescriptors;
  
  private List<ResultData> results;
  
  private int rowNum;
  
  public ResultSet(Descriptor keyDescriptor, Descriptor valueDescriptor) {
    this.keyDescriptor = keyDescriptor;
    this.valueDescriptor = valueDescriptor;
    
    results = new ArrayList<ResultData>();
    rowNum = 0;
    
    int keyFieldSize = keyDescriptor.getFields().size();
    keyFieldDescriptors = new HashMap<String, FieldDescriptor>();
    for (int i = 0; i < keyFieldSize; ++ i) {
      FieldDescriptor desc = keyDescriptor.getFields().get(i);
      keyFieldDescriptors.put(desc.getName(), desc);
    }

    int valueFieldSize = valueDescriptor.getFields().size();
    valueFieldDescriptors = new HashMap<String, FieldDescriptor>();
    for (int i = 0; i < valueFieldSize; ++ i) {
      FieldDescriptor desc = valueDescriptor.getFields().get(i);
      valueFieldDescriptors.put(desc.getName(), desc);
    }
  }
  
  public void addResult(DynamicMessage key, DynamicMessage value) {
    results.add(new ResultData(key, value, keyFieldDescriptors, valueFieldDescriptors));
  }
  
  public ResultData getResultData(int index) {
    if (index < 0 || index >= getRowCount()) {
      return null;
    }
    return results.get(index);
  }
  
  public int getRowCount() {
    return results.size();
  }
  
  public List<ResultData> getResults(int maxRow) {
    int start = rowNum;
    int end = Math.min(rowNum + maxRow, results.size());
    rowNum += maxRow;
    
    if (start > end) {
      return null;
    }
    
    return results.subList(start, end);
  }

  public Descriptor getKeyMetadata() {
    return keyDescriptor;
  }

  public Descriptor getValueMetadata() {
    return valueDescriptor;
  }
  
  public DynamicMessage.Builder newKeyBuilder() {
    if (keyDescriptor == null) {
      return null;
    } else {
      return DynamicMessage.newBuilder(keyDescriptor);
    }
  }
  
  public DynamicMessage.Builder newValueBuilder() {
    if (valueDescriptor == null) {
      return null;
    } else {
      return DynamicMessage.newBuilder(valueDescriptor);
    }
  }
  
}
