/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package printer;

import java.text.DecimalFormat;
import java.util.List;

import com.google.protobuf.Descriptors.Descriptor;
import com.google.protobuf.Descriptors.FieldDescriptor;

import idgs.execution.ResultSetMetadata;
import idgs.execution.RowData;
import idgs.execution.ResultSet;

public class DefaultResultPrinter implements ResultPrinter {
  
  private int MAX_SIZE = 500; 

  private static DecimalFormat integerFormat = new DecimalFormat("#");
  
  private static DecimalFormat floatFormat = new DecimalFormat("#.####");
  
  private static DecimalFormat doubleFormat = new DecimalFormat("#.########");
  
  private ResultSet resultSet;
  
  public DefaultResultPrinter(ResultSet resultSet) {
    this.resultSet = resultSet;
  }
  
  public void setMaxSize(int maxSize) {
    this.MAX_SIZE = maxSize;
  }
  
  @Override
  public String printResults() {
    return printResults(resultSet.getRowCount());
  }

  @Override
  public String printResults(int rows) {
    return printResults(0, rows);
  }
  
  public String printResults(int start, int rows) {
    StringBuffer sb = new StringBuffer();
    StringBuffer line = new StringBuffer();
    
    if (rows > MAX_SIZE) {
      sb.append("Print top " + MAX_SIZE + " data.\n");
      rows = MAX_SIZE;
    }
    
    ResultSetMetadata metadata = resultSet.getResultSetMetadata();
    Descriptor valueDescriptor = metadata.getValueMetadata();
    List<FieldDescriptor> fields = valueDescriptor.getFields();
    int fieldSize = fields.size();
    int[] colLength = new int[fieldSize];
    String[] schemas = new String[fieldSize];
    
    for (int i = 0; i < fieldSize; ++ i) {
      FieldDescriptor desc = fields.get(i);
      schemas[i] = desc.getName();
      colLength[i] = schemas[i].length();
    }
    
    for (int index = start; index < start + rows; ++ index) {
      RowData result = resultSet.getResultData(index);
      if (result == null) {
        break;
      }
      
      for (int i = 0; i < fieldSize; ++ i) {
        FieldDescriptor desc = fields.get(i);
        Object v = result.getValue().getField(desc);
        String s = formatValue(v, -1);
        colLength[i] = Math.max(colLength[i], s.length());
      }
    }
    
    line.append("+");
    for (int i = 0; i < schemas.length; ++ i) {
      line.append("-");
      for (int j = 0; j < colLength[i]; ++ j) {
        line.append("-");
      }
      line.append("-").append("+");
    }
    line.append("\n");
    
    sb.append(line);
    
    sb.append("|");
    for (int i = 0; i < schemas.length; ++ i) {
      sb.append(" ").append(rpad(schemas[i], colLength[i])).append(" |");
    }
    sb.append("\n");
    sb.append(line);
    
    for (int index = start; index < start + rows; ++ index) {
      RowData result = resultSet.getResultData(index);
      if (result == null) {
        break;
      }
      
      sb.append("|");
      for (int i = 0; i < fieldSize; ++ i) {
        FieldDescriptor desc = fields.get(i);
        Object v = result.getValue().getField(desc);
        String s = formatValue(v, colLength[i]);
        
        sb.append(" ").append(s).append(" |");
      }
      sb.append("\n");
    }
    sb.append(line);
    
    return sb.toString();
  }

  private String formatValue(Object value, int length) {
    String s = null;

    if (value == null) {
      s = rpad("null", length);
    } else if (value instanceof Integer) {
      s = lpad(integerFormat.format((Integer) value), length);
    } else if (value instanceof Long) {
      s = lpad(integerFormat.format((Long) value), length);
    } else if (value instanceof Float) {
      s = lpad(floatFormat.format((Float) value), length);
    } else if (value instanceof Double) {
      s = lpad(doubleFormat.format((Double) value), length);
    } else {
      s = rpad(value.toString(), length);
    }

    return s;
  }

  private String rpad(String value, int length) {
    if (value.length() >= length) {
      return value;
    } else {
      String space = "";
      for (int i = 0; i < length - value.length(); ++i) {
        space = space + " ";
      }
      return value + space;
    }
  }

  private String lpad(String value, int length) {
    if (value.length() >= length) {
      return value;
    } else {
      String space = "";
      for (int i = 0; i < length - value.length(); ++i) {
        space = " " + space;
      }
      return space + value;
    }
  }

}
