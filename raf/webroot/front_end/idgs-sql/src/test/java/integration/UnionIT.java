/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package integration;

import idgs.IdgsCliDriver;
import idgs.execution.ResultSet;
import junit.framework.TestCase;

public class UnionIT extends TestCase {

  public void testUnion() {
    int supCount = 0, ordCount = 0;
    String sql = "select s_comment comment from tpch.supplier";
    
    try {
      ResultSet resultSet = IdgsCliDriver.run(sql);
      assertNotNull(resultSet);
      
      supCount = resultSet.getRowCount();
    } catch (Exception e) {
      e.printStackTrace();
    }
    
    sql = "select o_comment comment from tpch.ORDERS";
    
    try {
      ResultSet resultSet = IdgsCliDriver.run(sql);
      assertNotNull(resultSet);
      
      ordCount = resultSet.getRowCount();
    } catch (Exception e) {
      e.printStackTrace();
    }
    
    StringBuffer sqlBuf = new StringBuffer();
    sqlBuf.append("  select * \n")
          .append("  from (select s_comment comment \n")
          .append("        from   tpch.SUPPLIER \n")
          .append("        union all \n")
          .append("        select o_comment comment \n")
          .append("        from   tpch.orders) t\n");
    sql = sqlBuf.toString();

    System.out.println("run test sql : ");
    System.out.println(sql);
    
    try {
      ResultSet resultSet = IdgsCliDriver.run(sql);
      assertNotNull(resultSet);
      
      assertEquals(supCount + ordCount, resultSet.getRowCount());
    } catch (Exception e) {
      e.printStackTrace();
    }
  }
  
}
