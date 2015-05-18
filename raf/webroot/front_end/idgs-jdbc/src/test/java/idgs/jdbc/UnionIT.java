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

import junit.framework.TestCase;

public class UnionIT extends TestCase {

  private final String driver = "idgs.jdbc.IdgsJdbcDriver";
  
  public void testUnionSQL() {
    StringBuffer sql = new StringBuffer();
    sql.append("  select * \n")
       .append("  from (select 'supplier' c1, s_acctbal c2 \n")
       .append("        from   tpch.SUPPLIER \n")
       .append("        union all \n")
       .append("        select 'orders' c1, o_totalprice c2 \n")
       .append("        from   tpch.orders) t\n");

    System.out.println("run test sql : ");
    System.out.println(sql.toString());
    
    Connection conn = null;
    try {
      Class.forName(driver);
      conn = DriverManager.getConnection("");
      Statement stmt = conn.createStatement();
      ResultSet rs = stmt.executeQuery(sql.toString());
      
      int rowCount = 0;
      while (rs.next()) {
        if (rowCount < 20) {
          String c1 = rs.getString("c1");
          Double c2 = rs.getDouble("c2");
          System.out.printf("| %8s | %9.2f |\n", c1, c2);
        }
        ++ rowCount;
      }
      System.out.println("fetch 20 rows");
      assertEquals(1510, rowCount);
      
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
