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

public class GroupByIT extends BaseCase {

  public void testGroupBy() {
    StringBuffer sql = new StringBuffer();
    sql.append("  select l_suppkey, \n")
       .append("         count(l_suppkey) cnt, \n")
       .append("         sum(l_suppkey) sum, \n")
       .append("         avg(l_suppkey) avg \n")
       .append("  from   tpch.LINEITEM \n")
       .append("  group  by l_suppkey\n");
    
    System.out.println("run test sql : ");
    System.out.println(sql);

    Connection conn = null;
    
    try {
      conn = DriverManager.getConnection("");
      Statement stmt = conn.createStatement();
      ResultSet rs = stmt.executeQuery(sql.toString());
      
      System.out.println();
      while (rs.next()) {
        Long l_suppkey = rs.getLong("l_suppkey");
        Long cnt = rs.getLong("cnt");
        Double sum = rs.getDouble("sum");
        Double avg = rs.getDouble("avg");
        
        System.out.printf("| %2d | %3d | %7.2f | %5.2f |\n", l_suppkey, cnt, sum, avg);
      }
      System.out.println();
      
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
  
  public void testGroupByAggrConst() {
    StringBuffer sql = new StringBuffer();
    sql.append("  select l_suppkey, \n")
       .append("         count(1) cnt, \n")
       .append("         sum(1) sum \n")
       .append("  from   tpch.LINEITEM \n")
       .append("  group  by l_suppkey\n");
    
    System.out.println("run test sql : ");
    System.out.println(sql);

    Connection conn = null;
    
    try {
      conn = DriverManager.getConnection("");
      Statement stmt = conn.createStatement();
      ResultSet rs = stmt.executeQuery(sql.toString());
      
      System.out.println();
      while (rs.next()) {
        Long l_suppkey = rs.getLong("l_suppkey");
        Long cnt = rs.getLong("cnt");
        Double sum = rs.getDouble("sum");
        
        System.out.printf("| %2d | %3d | %3d |\n", l_suppkey, cnt, sum.longValue());
      }
      System.out.println();
      
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
  
  public void testGroupByWithoutAggr() {
    StringBuffer sql = new StringBuffer();
    sql.append("  select l_suppkey \n")
       .append("  from   tpch.LINEITEM \n")
       .append("  group  by l_suppkey\n");
    
    System.out.println("run test sql : ");
    System.out.println(sql);

    Connection conn = null;
    
    try {
      conn = DriverManager.getConnection("");
      Statement stmt = conn.createStatement();
      ResultSet rs = stmt.executeQuery(sql.toString());
      
      System.out.println();
      while (rs.next()) {
        Long l_suppkey = rs.getLong("l_suppkey");
        
        System.out.printf("| %2d |\n", l_suppkey);
      }
      System.out.println();
      
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
  
  public void testGroupBySubQuery() {
    StringBuffer sql = new StringBuffer();
    sql.append("  select l_suppkey, \n")
       .append("         count(1) cnt \n")
       .append("  from   (select l_suppkey \n")
       .append("          from   tpch.lineitem \n")
       .append("          where  l_discount = 0.05) t \n")
       .append("  group  by l_suppkey\n");
    
    System.out.println("run test sql : ");
    System.out.println(sql);

    Connection conn = null;
    
    try {
      conn = DriverManager.getConnection("");
      Statement stmt = conn.createStatement();
      ResultSet rs = stmt.executeQuery(sql.toString());
      
      System.out.println();
      while (rs.next()) {
        Long l_suppkey = rs.getLong("l_suppkey");
        Long cnt = rs.getLong("cnt");
        
        System.out.printf("| %2d | %3d |\n", l_suppkey, cnt);
      }
      System.out.println();
      
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
  
  public void testGroupByKey() {
    StringBuffer sql = new StringBuffer();
    sql.append("  select c_custkey, \n")
       .append("         count(distinct c_acctbal) cnt, \n")
       .append("         sum(distinct c_acctbal) sum \n")
       .append("  from   tpch.customer \n")
       .append("  group  by c_custkey\n");
    
    System.out.println("run test sql : ");
    System.out.println(sql);

    Connection conn = null;
    
    try {
      conn = DriverManager.getConnection("");
      Statement stmt = conn.createStatement();
      ResultSet rs = stmt.executeQuery(sql.toString());
      
      System.out.println();
      while (rs.next()) {
        Long c_custkey = rs.getLong("c_custkey");
        Long cnt = rs.getLong("cnt");
        Double sum = rs.getDouble("sum");
        
        System.out.printf("| %3d | %3d | %7.2f |\n", c_custkey, cnt, sum);
      }
      System.out.println();
      
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
  
  public void testWholeTable() {
    StringBuffer sql = new StringBuffer();
    sql.append("  select sum(c_acctbal) sum, \n")
       .append("         count(c_custkey) cnt, \n")
       .append("         avg(c_acctbal) avg \n")
       .append("  from   tpch.customer\n");
    
    System.out.println("run test sql : ");
    System.out.println(sql);

    Connection conn = null;
    
    try {
      conn = DriverManager.getConnection("");
      Statement stmt = conn.createStatement();
      ResultSet rs = stmt.executeQuery(sql.toString());
      
      assertTrue(rs.next());
      
      System.out.println();
      Double sum = rs.getDouble("sum");
      Long cnt = rs.getLong("cnt");
      Double avg = rs.getDouble("avg");
      
      System.out.printf("| %f | %d | %f |\n", sum, cnt, avg);
      System.out.println();
      
      assertFalse(rs.next());
      
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
  
  public void testGroupByWithDistinct() {
    StringBuffer sql = new StringBuffer();
    sql.append("  select l_suppkey, \n")
       .append("         sum(distinct l_discount) s1, \n")
       .append("         sum(l_discount) s2, \n")
       .append("         avg(distinct l_quantity) s3, \n")
       .append("         avg(l_quantity) s4, \n")
       .append("         count(distinct l_tax) s5, \n")
       .append("         count(l_tax) s6, \n")
       .append("         sum(l_extendedprice) s7, \n")
       .append("         count(distinct l_suppkey) s8 \n")
       .append("  from   tpch.lineitem \n")
       .append("  where  l_orderkey <= 30 \n")
       .append("  group  by l_suppkey, l_orderkey\n");
    
    System.out.println("run test sql : ");
    System.out.println(sql);

    Connection conn = null;
    
    try {
      conn = DriverManager.getConnection("");
      Statement stmt = conn.createStatement();
      ResultSet rs = stmt.executeQuery(sql.toString());
      
      System.out.println();
      while (rs.next()) {
        Long l_suppkey = rs.getLong("l_suppkey");
        Double s1 = rs.getDouble("s1");
        Double s2 = rs.getDouble("s2");
        Double s3 = rs.getDouble("s3");
        Double s4 = rs.getDouble("s4");
        Long s5 = rs.getLong("s5");
        Long s6 = rs.getLong("s6");
        Double s7 = rs.getDouble("s7");
        Long s8 = rs.getLong("s8");
        
        System.out.printf("| %2d | %4.2f | %4.2f | %5.2f | %5.2f | %d | %d | %8.2f | %d |\n", l_suppkey, s1, s2, s3, s4, s5, s6, s7, s8);
      }
      System.out.println();
      
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
  
  public void testWholeTableWithDistinct() {
    StringBuffer sql = new StringBuffer();
    sql.append("  select sum(distinct l_discount) c1, \n")
       .append("         avg(distinct l_discount) c2, \n")
       .append("         avg(l_discount) c3, \n")
       .append("         sum(distinct l_suppkey) c4, \n")
       .append("         sum(l_discount) c5, \n")
       .append("         sum(l_quantity) c6 \n")
       .append("  from   tpch.lineitem\n");
    
    System.out.println("run test sql : ");
    System.out.println(sql);

    Connection conn = null;
    
    try {
      conn = DriverManager.getConnection("");
      Statement stmt = conn.createStatement();
      ResultSet rs = stmt.executeQuery(sql.toString());
      
      System.out.println();
      while (rs.next()) {
        Double c1 = rs.getDouble("c1");
        Double c2 = rs.getDouble("c2");
        Double c3 = rs.getDouble("c3");
        Double c4 = rs.getDouble("c4");
        Double c5 = rs.getDouble("c5");
        Double c6 = rs.getDouble("c6");
        
        System.out.printf("| %4.2f | %4.2f | %4.2f | %4.2f | %6.2f | %9.2f |\n", c1, c2, c3, c4, c5, c6);
      }
      System.out.println();
      
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
