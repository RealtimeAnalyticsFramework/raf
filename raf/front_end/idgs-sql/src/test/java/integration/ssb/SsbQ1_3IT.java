/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package integration.ssb;

import java.io.File;
import java.io.FileOutputStream;
import java.text.DecimalFormat;
import java.util.HashMap;
import java.util.Map;

import printer.DefaultResultPrinter;
import printer.ResultPrinter;
import idgs.IdgsCliDriver;
import idgs.execution.RowData;
import idgs.execution.ResultSet;
import junit.framework.TestCase;

public class SsbQ1_3IT extends TestCase {
  
  public void testSSBQ1_3() {
    StringBuffer sql = new StringBuffer();
    sql.append("  SELECT sum(lo.lo_extendedprice * lo.lo_discount) AS revenue\n")
       .append("  FROM   ssb.ssb_lineorder lo\n")
       .append("         JOIN ssb.ssb_date d\n")
       .append("           ON lo.lo_orderdate = d.d_datekey\n")
       .append("  WHERE  d.d_weeknuminyear = 6\n")
       .append("         AND d.d_year = 1994\n")
       .append("         AND lo.lo_discount BETWEEN 5 AND 7\n")
       .append("         AND lo.lo_quantity BETWEEN 26 AND 35\n");
    
    System.out.println("========================= run SSB Q1.3 =========================");
    System.out.println(sql.toString());
    
    try {
      long start = System.currentTimeMillis();
      ResultSet resultSet = IdgsCliDriver.run(sql.toString());
      double time = (System.currentTimeMillis() - start) / 1000.0;
      assertNotNull(resultSet);
      
      ResultPrinter printer = new DefaultResultPrinter(resultSet);
      System.out.println(printer.printResults(20));
      
      File file = new File("Q1.3.txt");
      FileOutputStream fos = new FileOutputStream(file);
      fos.write(("Q1.3 " + new DecimalFormat("0.00").format(time)).getBytes());
      fos.close();
      
      if (SsbTable.checkRawResult) {
        Map<String, String> dateMap = new HashMap<String, String>();
        double result = 0;
        
        SsbTable date = new SsbTable("date");
        for (int row = 0; row < date.getRowCount(); ++ row) {
          String d_year = date.getValue(row, "d_year");
          String d_weeknuminyear = date.getValue(row, "d_weeknuminyear");
          if (d_year.equals("1994") && d_weeknuminyear.equals("6")) {
            String d_datekey = date.getValue(row, "d_datekey");
            dateMap.put(d_datekey, null);
          }
        }
        
        SsbTable lineorder = new SsbTable("lineorder");
        for (int row = 0; row < lineorder.getRowCount(); ++ row) {
          String lo_orderdate = lineorder.getValue(row, "lo_orderdate");
          Integer lo_quantity = lineorder.getIntValue(row, "lo_quantity");
          Integer lo_discount = lineorder.getIntValue(row, "lo_discount");
          
          if (lo_quantity >= 26 && lo_quantity <= 35 && lo_discount >= 5 && lo_discount <= 7 && dateMap.containsKey(lo_orderdate)) {
            Double lo_extendedprice = lineorder.getDoubleValue(row, "lo_extendedprice");
            result += lo_extendedprice * lo_discount;
          }
        }
        
        if (resultSet.getRowCount() == 0) {
          assertEquals(0, result);
        } else {
          assertEquals(1, resultSet.getRowCount());
          
          RowData data = resultSet.getResultData(0);
          Object value = data.getFieldValue("revenue");
          assertNotNull(value);
          assertTrue(value instanceof Double);
          
          Double revenue = (Double) value;
          System.out.println("result is : " + new DecimalFormat("0,000,000.#").format(revenue));
          System.out.println("raw result is : " + new DecimalFormat("0,000,000.#").format(result));
          assertEquals(result, revenue, 0.00000001);
        }
      }
      
      System.out.println();
    } catch (Exception e) {
      e.printStackTrace();
    }
  }
  
}
