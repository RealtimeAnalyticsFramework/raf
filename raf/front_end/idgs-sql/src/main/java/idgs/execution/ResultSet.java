/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.execution;

import java.util.ArrayList;
import java.util.List;

import com.google.protobuf.Descriptors.Descriptor;
import com.google.protobuf.DynamicMessage;

public class ResultSet {

  private ResultSetMetadata metadata;
  
  private List<RowData> results;
  
  private int rowNum;
  
  public ResultSet(Descriptor keyDescriptor, Descriptor valueDescriptor) {
    results = new ArrayList<RowData>();
    rowNum = 0;
    metadata = new ResultSetMetadata(keyDescriptor, valueDescriptor);
  }
  
  public void addResult(DynamicMessage key, DynamicMessage value) {
    results.add(new RowData(key, value, metadata));
  }
  
  public RowData getResultData(int index) {
    if (index < 0 || index >= getRowCount()) {
      return null;
    }
    return results.get(index);
  }
  
  public int getRowCount() {
    return results.size();
  }
  
  public List<RowData> getResults(int maxRow) {
    int start = rowNum;
    int end = Math.min(rowNum + maxRow, results.size());
    rowNum += maxRow;
    
    if (start > end) {
      return null;
    }
    
    return results.subList(start, end);
  }
  
  public ResultSetMetadata getResultSetMetadata() {
    return metadata;
  }

}
