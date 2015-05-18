/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.execution;

import java.text.DecimalFormat;

import com.google.protobuf.DynamicMessage;
import com.google.protobuf.Descriptors.FieldDescriptor;

public class RowData {

  private DynamicMessage key;
  
  private DynamicMessage value;
  
  private ResultSetMetadata metadata;
  
  public RowData(DynamicMessage key, DynamicMessage value, ResultSetMetadata metadata) {
    this.key = key;
    this.value = value;
    this.metadata = metadata;
  }
  
  public DynamicMessage getKey() {
    return key;
  }

  public DynamicMessage getValue() {
    return value;
  }
  
  public Object getFieldValue(String fieldName) {
    FieldDescriptor field = metadata.findField(fieldName);
    boolean iskey = metadata.isKeyField(fieldName);

    if (iskey) {
      return key.getField(field);
    } else  {
      return value.getField(field);
    }
  }
  
  @Override
  public String toString() {
    StringBuffer sb = new StringBuffer();
    
    for (FieldDescriptor desc : value.getDescriptorForType().getFields()) {
      Object v = value.getField(desc);
      String s = null;
      
      if (v instanceof Integer) {
        s = new DecimalFormat("#").format((Integer) v);
      } else if (v instanceof Long) {
        s = new DecimalFormat("#").format((Long) v);
      } else if (v instanceof Float) {
        s = new DecimalFormat("#.#").format((Float) v);
      } else if (v instanceof Double) {
        s = new DecimalFormat("#.#").format((Double) v);
      } else {
        s = v.toString();
      }
      
      if (sb.length() > 0) {
        sb.append(", ");
      }
      sb.append(desc.getName() + " : " + s);
    }
    
    return sb.toString();
  }
  
}
