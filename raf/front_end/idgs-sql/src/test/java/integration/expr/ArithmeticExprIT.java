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

public class ArithmeticExprIT extends TestCase {

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
        Double add = (Double) data.getFieldValue("add");
        Double sub = (Double) data.getFieldValue("substract");
        Double mul = (Double) data.getFieldValue("multiply");
        Double div = (Double) data.getFieldValue("divide");
        Double mod = (Double) data.getFieldValue("modulus");
        Integer hash = (Integer) data.getFieldValue("hashcode");
        Long bitadd = (Long) data.getFieldValue("bitadd");
        Long bitsub = (Long) data.getFieldValue("bitsub");
        Long bitxor = (Long) data.getFieldValue("bitxor");
        Long bitnot = (Long) data.getFieldValue("bitnot");

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
      }
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    }
  }
  
}
