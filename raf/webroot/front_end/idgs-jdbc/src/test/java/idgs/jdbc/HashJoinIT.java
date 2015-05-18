/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.jdbc;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

public class HashJoinIT extends BaseCase {

  public void testHashJoinSQL() {
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
    
    Connection conn = null;
    try {
      conn = DriverManager.getConnection("");
      Statement stmt = conn.createStatement();
      ResultSet rs = stmt.executeQuery(sql.toString());

      int rowCount = 0;
      while (rs.next()) {
        Long o_orderkey = rs.getLong("o_orderkey");
        Long l_orderkey = rs.getLong("l_orderkey");
        String o_orderstatus = rs.getString("o_orderstatus");
        Double l_discount = rs.getDouble("l_discount");
        
        assertEquals("F", o_orderstatus);
        assertEquals(o_orderkey, l_orderkey);
        assertTrue(l_discount >= 0.05 && l_discount <= 0.07);
        
        if (rowCount < 20) {
          System.out.printf("| %4d | %4d | %s | %3.2f |\n", o_orderkey, l_orderkey, o_orderstatus, l_discount);
        }
        
        ++ rowCount;
      }
      System.out.println("fetch 20 rows");
      
      rs.close();
      stmt.close();
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    } finally {
      if (conn != null) {
        try {
          conn.close();
        } catch (SQLException e) {
          e.printStackTrace();
        }
      }
    }
  }
  
}
