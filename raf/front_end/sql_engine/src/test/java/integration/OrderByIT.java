/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package integration;

import idgs.IdgsCliDriver;
import idgs.execution.ResultData;
import idgs.execution.ResultSet;
import junit.framework.TestCase;

public class OrderByIT extends TestCase {

  public void testOrderByAscTable() {
    String sql = "select * from customer t order by c_name";

    try {
      ResultSet resultSet = IdgsCliDriver.run(sql);
      assertNotNull(resultSet);
      assertTrue(resultSet.getRowCount() > 0);
      
      for (int i = 0; i < resultSet.getRowCount(); ++ i) {
        ResultData data1 = resultSet.getResultData(i);
        String c_name1 = (String) data1.getFieldValue("c_name");

        if (i + 1 < resultSet.getRowCount()) {
          ResultData data2 = resultSet.getResultData(i + 1);
          String c_name2 = (String) data2.getFieldValue("c_name");
          
          assertTrue(c_name1.compareTo(c_name2) <= 0);
        }
      }
    } catch (Exception e) {
      e.printStackTrace();
    }
  }
  
  public void testOrderByDescTable() {
    String sql = "select * from customer order by c_name desc";
    
    try {
      ResultSet resultSet = IdgsCliDriver.run(sql);;
      assertNotNull(resultSet);
      assertTrue(resultSet.getRowCount() > 0);
      
      for (int i = 0; i < resultSet.getRowCount(); ++ i) {
        ResultData data1 = resultSet.getResultData(i);
        String c_name1 = (String) data1.getFieldValue("c_name");

        if (i + 1 < resultSet.getRowCount()) {
          ResultData data2 = resultSet.getResultData(i + 1);
          String c_name2 = (String) data2.getFieldValue("c_name");
          
          assertTrue(c_name1.compareTo(c_name2) >= 0);
        }
      }
    } catch (Exception e) {
      e.printStackTrace();
    }
  }
  
}
