/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package integration;

import idgs.IdgsCliDriver;
import idgs.execution.RowData;
import idgs.execution.ResultSet;
import junit.framework.TestCase;

public class HashJoinIT extends TestCase {

  public void testHashJoin() {
    StringBuffer sql = new StringBuffer();
    sql.append("  select o.o_orderkey, \n")
       .append("         l.l_orderkey, \n")
       .append("         o.o_orderstatus, \n")
       .append("         l.l_discount \n")
       .append("  from   tpch.orders o \n")
       .append("         left outer join tpch.lineitem l \n")
       .append("                      on o.o_orderkey = l.l_orderkey \n")
       .append("  where  o.o_orderstatus = 'F' \n")
       .append("         and l.l_discount between 0.05 and 0.07\n");

    System.out.println("run test sql : ");
    System.out.println(sql.toString());
    
    try {
      ResultSet resultSet = IdgsCliDriver.run(sql.toString());
      assertNotNull(resultSet);
      
      for (int i = 0; i < resultSet.getRowCount(); ++ i) {
        RowData data = resultSet.getResultData(i);
        assertEquals(data.getFieldValue("o_orderkey"), data.getFieldValue("l_orderkey"));
        assertEquals("F", data.getFieldValue("o_orderstatus"));
        Object discountObj = data.getFieldValue("l_discount");
        assertTrue(discountObj instanceof Double);
        Double discount = (Double) discountObj;
        assertTrue(discount >= 0.05 && discount <= 0.07);
      }
    } catch (Exception e) {
      e.printStackTrace();
    }
  }
  
}
