/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.jdbc.expr;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.Statement;

import idgs.jdbc.BaseCase;

public class MathExprIT extends BaseCase {

  public void testMathExpr() {
    StringBuffer sql = new StringBuffer();
    sql.append("  select c_custkey, \n")
       .append("         c_acctbal, \n")
       .append("         c_name, \n")
       .append("         round(c_acctbal) round1, \n")
       .append("         round(c_acctbal, 1) round2, \n")
       .append("         floor(c_acctbal) floor, \n")
       .append("         ceil(c_custkey) ceil1, \n")
       .append("         ceiling(c_acctbal) ceil2, \n")
       .append("         rand() rand, \n")
       .append("         exp(c_custkey % 10) exp, \n")
       .append("         ln(c_acctbal) ln, \n")
       .append("         log10(c_acctbal) log10, \n")
       .append("         log2(c_acctbal) log2, \n")
       .append("         log(2, c_acctbal) log, \n")
       .append("         pow(c_acctbal, 2) pow, \n")
       .append("         sqrt(c_custkey) sqrt, \n")
       .append("         bin(c_custkey) bin, \n")
       .append("         hex(c_custkey) hex1, \n")
       .append("         hex(c_name) hex2, \n")
       .append("         unhex(hex(c_name)) unhex, \n")
       .append("         conv(c_custkey, 10, 16) conv, \n")
       .append("         abs(c_acctbal) abs, \n")
       .append("         pmod(c_acctbal, c_custkey) pmod, \n")
       .append("         sin(c_acctbal) sin, \n")
       .append("         asin(sin(c_acctbal)) asin, \n")
       .append("         cos(c_acctbal) cos, \n")
       .append("         acos(cos(c_acctbal)) acos, \n")
       .append("         positive(c_acctbal) positive, \n")
       .append("         negative(c_acctbal) negative, \n")
       .append("         degrees(c_acctbal) degrees, \n")
       .append("         radians(c_acctbal) radians, \n")
       .append("         sign(c_acctbal) sign, \n")
       .append("         e() e, \n")
       .append("         pi() pi \n")
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
      String formatter = "| %3d | %7.2f | %s | %7.2f | %7.2f | %4d | %3d | %4d "
                       + "| %4.2f | %7.2f | %4.2f | %4.2f | %5.2f | %5.2f | %13.4f "
                       + "| %5.2f | %8s | %2s | %s | %s | %2s | %7.2f | %5.2f | %9.6f "
                       + "| %9.6f | %9.6f | %9.6f | %7.2f | %8.2f | %13.6f | %10.6f "
                       + "| %1.0f | %f | %f |\n";
      
      while (rs.next()) {
        Long custkey = (Long) rs.getObject("c_custkey");
        Double acctbal = (Double) rs.getObject("c_acctbal");
        String name = (String) rs.getObject("c_name");
        Double round1 = (Double) rs.getObject("round1");
        Double round2 = (Double) rs.getObject("round2");
        Long floor = (Long) rs.getObject("floor");
        Long ceil1 = (Long) rs.getObject("ceil1");
        Long ceil2 = (Long) rs.getObject("ceil2");
        Double rand = (Double) rs.getObject("rand");
        Double exp = (Double) rs.getObject("exp");
        Double ln = (Double) rs.getObject("ln");
        Double log10 = (Double) rs.getObject("log10");
        Double log2 = (Double) rs.getObject("log2");
        Double log = (Double) rs.getObject("log");
        Double pow = (Double) rs.getObject("pow");
        Double sqrt = (Double) rs.getObject("sqrt");
        String bin = (String) rs.getObject("bin");
        String hex1 = (String) rs.getObject("hex1");
        String hex2 = (String) rs.getObject("hex2");
        String unhex = new String((byte[]) rs.getObject("unhex"));
        String conv = (String) rs.getObject("conv");
        Double abs = (Double) rs.getObject("abs");
        Double pmod = (Double) rs.getObject("pmod");
        Double sin = (Double) rs.getObject("sin");
        Double asin = (Double) rs.getObject("asin");
        Double cos = (Double) rs.getObject("cos");
        Double acos = (Double) rs.getObject("acos");
        Double positive = (Double) rs.getObject("positive");
        Double negative = (Double) rs.getObject("negative");
        Double degrees = (Double) rs.getObject("degrees");
        Double radians = (Double) rs.getObject("radians");
        Double sign = (Double) rs.getObject("sign");
        Double e = (Double) rs.getObject("e");
        Double pi = (Double) rs.getObject("pi");
        
        // test round
        assertEquals(Math.round(acctbal), round1, 0.0000001);
        assertEquals(Math.round(acctbal * 10) / 10.0, round2, 0.0000001);
        // test floor
        assertEquals(Math.floor(acctbal), floor, 0.0000001);
        // test ceil
        assertEquals(Math.ceil(custkey), ceil1, 0.0000001);
        assertEquals(Math.ceil(acctbal), ceil2, 0.0000001);
        // test rand
        assertTrue(rand >= 0 && rand <= 1);
        // test exp
        assertEquals(Math.exp(custkey % 10), exp, 0.0000001);
        // test ln
        assertEquals(Math.log(acctbal), ln, 0.0000001);
        // test log10
        assertEquals(Math.log(acctbal) / Math.log(10), log10, 0.0000001);
        // test log2
        assertEquals(Math.log(acctbal) / Math.log(2), log2, 0.0000001);
        // test log
        assertEquals(Math.log(acctbal) / Math.log(2), log, 0.0000001);
        // test pow
        assertEquals(Math.pow(acctbal, 2), pow, 0.0000001);
        // test sqrt
        assertEquals(Math.sqrt(custkey), sqrt, 0.0000001);
        // test bin
        assertEquals(Integer.toBinaryString(custkey.intValue()), bin);
        // test hex
        assertEquals(Integer.toHexString(custkey.intValue()).toLowerCase(), hex1.toLowerCase());
        String result = new String();
        for (byte c : name.getBytes()) {
          result += Integer.toHexString(c).toUpperCase();
        }
        assertEquals(result, hex2);
        // test unhex
        assertEquals(name, unhex);
        // test conv
        assertEquals(Integer.toHexString(custkey.intValue()), conv);
        // test abs
        assertEquals(Math.abs(acctbal), abs, 0.0000001);
        // test pmod
        assertEquals(Math.abs(acctbal % custkey), pmod, 0.0000001);
        // test sin
        assertEquals(Math.sin(acctbal), sin, 0.0000001);
        // test asin
        assertEquals(Math.asin(Math.sin(acctbal)), asin, 0.0000001);
        // test cos
        assertEquals(Math.cos(acctbal), cos, 0.0000001);
        // test acos
        assertEquals(Math.acos(Math.cos(acctbal)), acos, 0.0000001);
        // test positive
        assertEquals(acctbal, positive);
        // test negative
        assertEquals(0 - acctbal, negative);
        // test degrees
        assertEquals(acctbal * 180.0 / Math.PI, degrees, 0.0000001);
        // test radians
        assertEquals(acctbal / 180.0 * Math.PI, radians, 0.0000001);
        // test sign
        assertEquals(Math.signum(acctbal), sign);
        // test e
        assertEquals(Math.E, e, 0.0000001);
        // test pi
        assertEquals(Math.PI, pi, 0.0000001);
        
        System.out.printf(formatter, custkey, acctbal, name, round1, round2, 
            floor, ceil1, ceil2, rand, exp, ln, log10, log2, log, pow, sqrt, 
            bin, hex1, hex2, unhex, conv, abs, pmod, sin, asin, cos, acos, 
            positive, negative, degrees, radians, sign, e, pi);
      }
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    }
  }
  
}
