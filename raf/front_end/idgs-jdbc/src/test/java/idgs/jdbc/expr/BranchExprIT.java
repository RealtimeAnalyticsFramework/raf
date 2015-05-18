/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.jdbc.expr;

import idgs.jdbc.BaseCase;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.Statement;

public class BranchExprIT extends BaseCase {
  
  public void testBranchExpr() {
    StringBuffer sql = new StringBuffer();
    sql.append("  select c_custkey, \n")
       .append("         c_name, \n")
       .append("         if(c_custkey < 50, '<50', '>=50') if_expr, \n")
       .append("         case when c_custkey <= 20 then '<=20' when c_custkey <= 50 then '<=50' else '>50' end when_expr, \n")
       .append("         case c_name when 'Customer#000000001' then 1 when 'Customer#000000120' then 120 else -1 end case_expr, \n")
       .append("         nvl(c_name, 'null') nvl_expr, \n")
       .append("         coalesce(c_name, c_custkey) coalesce_expr \n")
       .append("  from   tpch.customer \n")
       .append("  limit  10");
    
    System.out.println("test sql : ");
    System.out.println(sql.toString());
    
    Connection conn = null;
    
    try {
      conn = DriverManager.getConnection("");
      Statement stmt = conn.createStatement();
      ResultSet rs = stmt.executeQuery(sql.toString());
      
      System.out.println();
      String formatter = "| %3d | %4s | %4s | %4s | %3d | %s | %s |\n";
      while (rs.next()) {
        Long custkey = (Long) rs.getObject("c_custkey");
        String name = (String) rs.getObject("c_name");
        String if_expr = (String) rs.getObject("if_expr");
        String when_expr = (String) rs.getObject("when_expr");
        Integer case_expr = (Integer) rs.getObject("case_expr");
        String nvl_expr = (String) rs.getObject("nvl_expr");
        Object coalesce_expr = rs.getObject("coalesce_expr");
        
        // test if 
        if (custkey < 50) {
          assertEquals("<50", if_expr);
        } else {
          assertEquals(">=50", if_expr);
        }
        // test when
        if (custkey <= 20) {
          assertEquals("<=20", when_expr);
        } else if (custkey <= 50) {
          assertEquals("<=50", when_expr);
        } else {
          assertEquals(">50", when_expr);
        }
        // test case
        if (name.equals("Customer#000000001")) {
          assertEquals(1, case_expr.intValue());
        } else if (name.equals("Customer#000000120")) {
          assertEquals(120, case_expr.intValue());
        } else {
          assertEquals(-1, case_expr.intValue());
        }
        // test nvl
        if (nvl_expr != null) {
          assertEquals(name, nvl_expr);
        }
        // test coalesce
        if (name != null && !name.equals("null")) {
          assertEquals(name, coalesce_expr);
        } else if (custkey != null) {
          assertEquals(custkey, coalesce_expr);
        }
        
        System.out.printf(formatter, custkey, name, if_expr, when_expr, case_expr, nvl_expr, coalesce_expr);
      }
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    }
  }
  
}
