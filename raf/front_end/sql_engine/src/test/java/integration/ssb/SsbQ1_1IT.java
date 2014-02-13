/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package integration.ssb;

import java.text.DecimalFormat;
import java.util.HashMap;
import java.util.Map;

import idgs.IdgsCliDriver;
import idgs.execution.ResultData;
import idgs.execution.ResultSet;
import junit.framework.TestCase;

public class SsbQ1_1IT extends TestCase {
  
  public void testSSBQ1_1() {
    StringBuffer sql = new StringBuffer();
    sql.append("  SELECT sum(lo.lo_extendedprice * lo.lo_discount) AS revenue\n")
       .append("  FROM   ssb_lineorder lo\n")
       .append("         JOIN ssb_date d\n")
       .append("           ON lo.lo_orderdate = d.d_datekey\n")
       .append("  WHERE  d.d_year = 1993\n")
       .append("         AND lo.lo_discount BETWEEN 1 AND 3\n")
       .append("         AND lo.lo_quantity < 25\n");
    
    System.out.println("========================= run SSB Q1.1 =========================");
    System.out.println(sql.toString());
    
    try {
      ResultSet resultSet = IdgsCliDriver.run(sql.toString());
      assertNotNull(resultSet);
      assertEquals(1, resultSet.getRowCount());
      
      ResultData data = resultSet.getResultData(0);
      
      Object value = data.getFieldValue("revenue");
      
      assertNotNull(value);
      assertTrue(value instanceof Double);
      
      Double revenue = (Double) value;
      System.out.println("result is : " + new DecimalFormat("0,000,000.#").format(revenue));

      if (SsbTable.checkRawResult) {
        Map<String, String> dateMap = new HashMap<String, String>();
        double result = 0;
        
        SsbTable date = new SsbTable("date");
        for (int row = 0; row < date.getRowCount(); ++ row) {
          String d_year = date.getValue(row, "d_year");
          if (d_year.equals("1993")) {
            String d_datekey = date.getValue(row, "d_datekey");
            dateMap.put(d_datekey, null);
          }
        }
  
        SsbTable lineorder = new SsbTable("lineorder");
        for (int row = 0; row < lineorder.getRowCount(); ++ row) {
          String lo_orderdate = lineorder.getValue(row, "lo_orderdate");
          Integer lo_quantity = lineorder.getIntValue(row, "lo_quantity");
          Integer lo_discount = lineorder.getIntValue(row, "lo_discount");
          
          if (lo_quantity < 25 && lo_discount >= 1 && lo_discount <= 3 && dateMap.containsKey(lo_orderdate)) {
            Double lo_extendedprice = lineorder.getDoubleValue(row, "lo_extendedprice");
            result += lo_extendedprice * lo_discount;
          }
        }
        
        System.out.println("raw result is : " + new DecimalFormat("0,000,000.#").format(result));
        assertEquals(result, revenue, 0.00000001);
      }
      
      System.out.println("================================================================");
      System.out.println();
    } catch (Exception e) {
      e.printStackTrace();
    }
  }
  
}
