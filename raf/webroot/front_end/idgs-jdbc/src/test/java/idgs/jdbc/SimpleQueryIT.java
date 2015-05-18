/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.jdbc;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import java.sql.Statement;

public class SimpleQueryIT extends BaseCase {

  public void testSimpleQuery() {
    Connection conn = null;
    try {
      conn = DriverManager.getConnection("");
      Statement stmt = conn.createStatement();
      ResultSet rs = stmt.executeQuery("select * from tpch.orders where o_orderstatus = 'F'");
      ResultSetMetaData metadata = rs.getMetaData();
      int colCount = metadata.getColumnCount();
      assertEquals(9, colCount);
      assertEquals("o_orderkey", metadata.getColumnName(1));
      assertEquals("bigint", metadata.getColumnTypeName(1));
      
      assertEquals("o_custkey", metadata.getColumnName(2));
      assertEquals("bigint", metadata.getColumnTypeName(2));
      
      assertEquals("o_orderstatus", metadata.getColumnName(3));
      assertEquals("string", metadata.getColumnTypeName(3));
      
      assertEquals("o_totalprice", metadata.getColumnName(4));
      assertEquals("double", metadata.getColumnTypeName(4));
      
      assertEquals("o_orderdate", metadata.getColumnName(5));
      assertEquals("string", metadata.getColumnTypeName(5));
      
      assertEquals("o_orderpriority", metadata.getColumnName(6));
      assertEquals("string", metadata.getColumnTypeName(6));
      
      assertEquals("o_clerk", metadata.getColumnName(7));
      assertEquals("string", metadata.getColumnTypeName(7));
      
      assertEquals("o_shippriority", metadata.getColumnName(8));
      assertEquals("int", metadata.getColumnTypeName(8));
      
      assertEquals("o_comment", metadata.getColumnName(9));
      assertEquals("string", metadata.getColumnTypeName(9));
      
      while (rs.next()) {
        assertEquals(rs.getLong("o_orderkey"), rs.getLong(1));
        assertEquals(rs.getLong("o_custkey"), rs.getLong(2));
        assertEquals(rs.getString("o_orderstatus"), rs.getString(3));
        assertEquals(rs.getDouble("o_totalprice"), rs.getDouble(4));
        assertEquals(rs.getString("o_orderdate"), rs.getString(5));
        assertEquals(rs.getString("o_orderpriority"), rs.getString(6));
        assertEquals(rs.getString("o_clerk"), rs.getString(7));
        assertEquals(rs.getInt("o_shippriority"), rs.getInt(8));
        assertEquals(rs.getString("o_comment"), rs.getString(9));
        
        String status = rs.getString("o_orderstatus");
        assertNotNull(status);
        assertEquals("F", status);
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
