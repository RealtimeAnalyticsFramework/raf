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

public class ArithmeticExprIT extends BaseCase {

  public void testArithmeticExpr() {
    StringBuffer sql = new StringBuffer();
    sql.append("  select c_custkey, \n")
       .append("         c_acctbal, \n")
       .append("         c_acctbal + c_custkey add, \n")
       .append("         c_acctbal - c_custkey substract, \n")
       .append("         c_acctbal * c_custkey multiply, \n")
       .append("         c_acctbal / c_custkey divide, \n")
       .append("         c_acctbal % c_custkey modulus, \n")
       .append("         hash(c_acctbal) hashcode, \n")
       .append("         c_custkey & 5 bitadd, \n")
       .append("         c_custkey | 5 bitsub, \n")
       .append("         c_custkey ^ 5 bitxor, \n")
       .append("         ~c_custkey bitnot \n")
       .append("  from   tpch.customer \n")
       .append("  limit  10");
    
    System.out.println("run test sql : ");
    System.out.println(sql.toString());
    
    Connection conn = null;
    
    try {
      conn = DriverManager.getConnection("");
      Statement stmt = conn.createStatement();
      ResultSet rs = stmt.executeQuery(sql.toString());
      
      System.out.println();
      String formatter = "| %3d | %7.2f | %7.2f | %7.2f | %10.2f | %6.2f | %5.2f | %11d | %d | %3d | %3d | %4d |\n";
      while (rs.next()) {
        Long custkey = (Long) rs.getObject("c_custkey");
        Double acctbal = (Double) rs.getObject("c_acctbal");
        Double add = (Double) rs.getObject("add");
        Double sub = (Double) rs.getObject("substract");
        Double mul = (Double) rs.getObject("multiply");
        Double div = (Double) rs.getObject("divide");
        Double mod = (Double) rs.getObject("modulus");
        Integer hash = (Integer) rs.getObject("hashcode");
        Long bitadd = (Long) rs.getObject("bitadd");
        Long bitsub = (Long) rs.getObject("bitsub");
        Long bitxor = (Long) rs.getObject("bitxor");
        Long bitnot = (Long) rs.getObject("bitnot");

        // test +
        assertEquals(acctbal + custkey, add);
        // test -
        assertEquals(acctbal - custkey, sub);
        // test *
        assertEquals(acctbal * custkey, mul);
        // test /
        assertEquals(acctbal / custkey, div);
        // test %
        assertEquals(acctbal % custkey, mod);
        // test hash
        assertTrue(hash.intValue() != 0);
        // test &
        assertEquals(custkey & 5, bitadd.longValue());
        // test |
        assertEquals(custkey | 5, bitsub.longValue());
        // test ^
        assertEquals(custkey ^ 5, bitxor.longValue());
        // test ~
        assertEquals(~custkey, bitnot.longValue());
        
        System.out.printf(formatter, custkey, acctbal, add, sub, mul, div, mod, hash, bitadd, bitsub, bitxor, bitnot);
      }
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    }
  }
  
}
