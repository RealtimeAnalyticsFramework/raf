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
import java.util.ArrayList;
import java.util.List;

public class OrderByIT extends BaseCase {

  public void testOrderBySQL() {
    StringBuffer sql = new StringBuffer();
    sql.append("  select * \n")
       .append("  from   tpch.customer t\n")
       .append("  order  by c_name\n");
    
    System.out.println("run test sql : ");
    System.out.println(sql.toString());
    
    Connection conn = null;
    try {
      conn = DriverManager.getConnection("");
      Statement stmt = conn.createStatement();
      ResultSet rs = stmt.executeQuery(sql.toString());
      
      List<String> res = new ArrayList<String>();
      while (rs.next()) {
        String c_name = rs.getString("c_name");
        res.add(c_name);
        if (res.size() <= 20) {
          System.out.println("| " + c_name + " |");
        }
      }
      System.out.println("fetch 20 rows");
      
      for (int i = 0; i < res.size(); ++ i) {
        if (i + 1 < res.size()) {
          String c_name1 = res.get(i);
          String c_name2 = res.get(i + 1);
          assertTrue(c_name1.compareTo(c_name2) < 0);
        }
      }
      
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
  
  public void testOrderByDescSQL() {
    String driver = "idgs.jdbc.IdgsJdbcDriver";
    
    StringBuffer sql = new StringBuffer();
    sql.append("  select * \n")
       .append("  from   tpch.customer t\n")
       .append("  order  by c_name desc\n");
    
    System.out.println("run test sql : ");
    System.out.println(sql.toString());
    
    Connection conn = null;
    try {
      Class.forName(driver);
      conn = DriverManager.getConnection("");
      Statement stmt = conn.createStatement();
      ResultSet rs = stmt.executeQuery(sql.toString());
      
      List<String> res = new ArrayList<String>();
      while (rs.next()) {
        String c_name = rs.getString("c_name");
        res.add(c_name);
        if (res.size() <= 20) {
          System.out.println("| " + c_name + " |");
        }
      }
      System.out.println("fetch 20 rows");
      
      for (int i = 0; i < res.size(); ++ i) {
        if (i + 1 < res.size()) {
          String c_name1 = res.get(i);
          String c_name2 = res.get(i + 1);
          assertTrue(c_name1.compareTo(c_name2) > 0);
        }
      }
      
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
