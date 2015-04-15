/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package integration.expr;

import idgs.IdgsCliDriver;
import idgs.execution.ResultData;
import idgs.execution.ResultSet;
import junit.framework.TestCase;

public class MathExprIT extends TestCase {

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
       .append("  from customer \n")
       .append("  limit 10");
    try {
      System.out.println("test sql : ");
      System.out.println(sql.toString());
      ResultSet resultSet = IdgsCliDriver.run(sql.toString());
      assertNotNull(resultSet);
      
      assertEquals(10, resultSet.getRowCount());
      for (int i = 0; i < resultSet.getRowCount(); ++ i) {
        ResultData data = resultSet.getResultData(i);
        Long custkey = (Long) data.getFieldValue("c_custkey");
        Double acctbal = (Double) data.getFieldValue("c_acctbal");
        String name = (String) data.getFieldValue("c_name");
        Double round1 = (Double) data.getFieldValue("round1");
        Double round2 = (Double) data.getFieldValue("round2");
        Long floor = (Long) data.getFieldValue("floor");
        Long ceil1 = (Long) data.getFieldValue("ceil1");
        Long ceil2 = (Long) data.getFieldValue("ceil2");
        Double rand = (Double) data.getFieldValue("rand");
        Double exp = (Double) data.getFieldValue("exp");
        Double ln = (Double) data.getFieldValue("ln");
        Double log10 = (Double) data.getFieldValue("log10");
        Double log2 = (Double) data.getFieldValue("log2");
        Double log = (Double) data.getFieldValue("log");
        Double pow = (Double) data.getFieldValue("pow");
        Double sqrt = (Double) data.getFieldValue("sqrt");
        String bin = (String) data.getFieldValue("bin");
        String hex1 = (String) data.getFieldValue("hex1");
        String hex2 = (String) data.getFieldValue("hex2");
        String unhex = (String) data.getFieldValue("unhex");
        String conv = (String) data.getFieldValue("conv");
        Double abs = (Double) data.getFieldValue("abs");
        Double pmod = (Double) data.getFieldValue("pmod");
        Double sin = (Double) data.getFieldValue("sin");
        Double asin = (Double) data.getFieldValue("asin");
        Double cos = (Double) data.getFieldValue("cos");
        Double acos = (Double) data.getFieldValue("acos");
        Double positive = (Double) data.getFieldValue("positive");
        Double negative = (Double) data.getFieldValue("negative");
        Double degrees = (Double) data.getFieldValue("degrees");
        Double radians = (Double) data.getFieldValue("radians");
        Double sign = (Double) data.getFieldValue("sign");
        Double e = (Double) data.getFieldValue("e");
        Double pi = (Double) data.getFieldValue("pi");
        
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
      }
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    }
  }
  
}
