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

public class TpchQ6IT extends BaseCase {

  public void testTpchQ6() {
    StringBuffer sql = new StringBuffer();
    sql.append("  SELECT sum(l_extendedprice * l_discount) price\n")
       .append("  FROM   tpch.lineitem\n")
       .append("  WHERE  l_shipdate >= '1994-01-01'\n")
       .append("         AND l_shipdate < '1995-01-01'\n")
       .append("         AND l_discount between 0.05 AND 0.07\n")
       .append("         AND l_quantity < 24\n");
    
    System.out.println(sql.toString());
    Connection conn = null;
    try {
      conn = DriverManager.getConnection("");
      Statement stmt = conn.createStatement();
      ResultSet rs = stmt.executeQuery(sql.toString());
      
      rs.next();
      Double price = rs.getDouble("price");
      assertEquals(77949.918600, price, 0.00000001);
      System.out.printf("| 10.4f |", price);
      
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
