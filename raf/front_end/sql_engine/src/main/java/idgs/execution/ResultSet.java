/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.execution;

import java.text.DecimalFormat;
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
  
  private String[] keyFields;
  
  private String[] valueFields;
  
  private Map<String, FieldDescriptor> keyFieldDescriptors;
  
  private Map<String, FieldDescriptor> valueFieldDescriptors;
  
  private List<ResultData> results;
  
  private int[] colLength;
  
  private int rowNum;
  
  public ResultSet(Descriptor keyDescriptor, Descriptor valueDescriptor) {
    this.keyDescriptor = keyDescriptor;
    this.valueDescriptor = valueDescriptor;
    
    results = new ArrayList<ResultData>();
    rowNum = 0;
    
    int keyFieldSize = keyDescriptor.getFields().size();
    keyFields = new String[keyFieldSize];
    keyFieldDescriptors = new HashMap<String, FieldDescriptor>();
    for (int i = 0; i < keyFieldSize; ++ i) {
      FieldDescriptor desc = keyDescriptor.getFields().get(i);
      keyFields[i] = desc.getName();
      keyFieldDescriptors.put(desc.getName(), desc);
    }

    int valueFieldSize = valueDescriptor.getFields().size();
    colLength = new int[valueFieldSize];
    valueFields = new String[valueFieldSize];
    valueFieldDescriptors = new HashMap<String, FieldDescriptor>();
    for (int i = 0; i < valueFieldSize; ++ i) {
      FieldDescriptor desc = valueDescriptor.getFields().get(i);
      valueFields[i] = desc.getName();
      valueFieldDescriptors.put(desc.getName(), desc);
      colLength[i] = valueFields[i].length();
    }
  }
  
  public void addResult(DynamicMessage key, DynamicMessage value) {
    results.add(new ResultData(key, value, keyFieldDescriptors, valueFieldDescriptors));
  }
  
  public ResultData getResultData(int index) {
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
  
  public int getRowNum() {
    return rowNum;
  }
  
  private String formatString(String value, int maxLength) {
    if (value.length() >= maxLength) {
      return value;
    } else {
      String space = "";
      for (int i = 0; i < maxLength - value.length(); ++ i) {
        space = space + " ";
      }
      return value + space;
    }
  }
  
  private String formatNumber(String value, int maxLength) {
    if (value.length() >= maxLength) {
      return value;
    } else {
      String space = "";
      for (int i = 0; i < maxLength - value.length(); ++ i) {
        space = " " + space;
      }
      return space + value;
    }
  }
  
  public String printResults() {
    return printResults(getRowCount());
  }

  public String printResults(int top) {
    int index = 0;
    for (ResultData result : results) {
      if (index < top) {
        for (int i = 0; i < valueDescriptor.getFields().size(); ++ i) {
          
          FieldDescriptor desc = valueDescriptor.getFields().get(i);
          Object v = result.getValue().getField(desc);
          String s = null;
          
          if (v == null) {
            s = "null";
          } else if (v instanceof Integer) {
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
          colLength[i] = Math.max(colLength[i], s.length());
        }
      }
      
      ++ index;
    }
    
    StringBuffer sb = new StringBuffer();
    StringBuffer line = new StringBuffer();

    line.append("+");
    for (int i = 0; i < valueFields.length; ++ i) {
      line.append("-");
      for (int j = 0; j < colLength[i]; ++ j) {
        line.append("-");
      }
      line.append("-").append("+");
    }
    line.append("\n");
    
    sb.append(line);
    
    sb.append("|");
    for (int i = 0; i < valueFields.length; ++ i) {
      sb.append(" ").append(formatString(valueFields[i], colLength[i])).append(" |");
    }
    sb.append("\n");
    sb.append(line);
    
    index = 0;
    for (ResultData result : results) {
      if (index < top) {
        sb.append("|");
        for (int i = 0; i < valueDescriptor.getFields().size(); ++ i) {
          FieldDescriptor desc = valueDescriptor.getFields().get(i);
          Object v = result.getValue().getField(desc);
          String s = null;
          
          if (v == null) {
            s = formatString("null", colLength[i]);
          } else if (v instanceof Integer) {
            s = formatNumber(new DecimalFormat("#").format((Integer) v), colLength[i]);
          } else if (v instanceof Long) {
            s = formatNumber(new DecimalFormat("#").format((Long) v), colLength[i]);
          } else if (v instanceof Float) {
            s = formatNumber(new DecimalFormat("#.#").format((Float) v), colLength[i]);
          } else if (v instanceof Double) {
            s = formatNumber(new DecimalFormat("#.#").format((Double) v), colLength[i]);
          } else {
            s = formatString(v.toString(), colLength[i]);
          }
          
          sb.append(" ").append(s).append(" |");
        }
        sb.append("\n");
      }
      
      ++ index;
    }
    sb.append(line);
    
    return sb.toString();
  }
  
}
