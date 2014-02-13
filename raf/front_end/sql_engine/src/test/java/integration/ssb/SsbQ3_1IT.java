/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package integration.ssb;

import java.util.HashMap;
import java.util.Map;
import java.util.TreeMap;

import idgs.IdgsCliDriver;
import idgs.execution.ResultData;
import idgs.execution.ResultSet;
import junit.framework.TestCase;

public class SsbQ3_1IT extends TestCase {
  
  public void testSSBQ3_1() {
    StringBuffer sql = new StringBuffer();
    sql.append("  SELECT c.c_nation,\n")
       .append("         s.s_nation,\n")
       .append("         d.d_year,\n")
       .append("         sum(lo.lo_revenue) AS revenue\n")
       .append("  FROM   ssb_lineorder lo\n")
       .append("         JOIN ssb_customer c\n")
       .append("           ON lo.lo_custkey = c.c_custkey\n")
       .append("         JOIN ssb_supplier s\n")
       .append("           ON lo.lo_suppkey = s.s_suppkey\n")
       .append("         JOIN ssb_date d\n")
       .append("           ON lo.lo_orderdate = d.d_datekey\n")
       .append("  WHERE  c.c_region = 'ASIA'\n")
       .append("         AND s.s_region = 'ASIA'\n")
       .append("         AND d.d_year >= 1992\n")
       .append("         AND d.d_year <= 1997\n")
       .append("  GROUP  BY c.c_nation,\n")
       .append("            s.s_nation,\n")
       .append("            d.d_year\n")
       .append("  ORDER  BY d_year ASC,\n")
       .append("            revenue DESC\n");
    
    System.out.println("========================= run SSB Q3.1 =========================");
    System.out.println(sql.toString());
    
    try {
      ResultSet resultSet = IdgsCliDriver.run(sql.toString());
      assertNotNull(resultSet);
      
      System.out.println("The top 20 results is : ");
      System.out.println(resultSet.printResults(20));
      
      if (SsbTable.checkRawResult) {
        Map<String, String> custMap = new HashMap<String, String>();
        Map<String, String> suppMap = new HashMap<String, String>();
        Map<String, String> dateMap = new HashMap<String, String>();
        Map<String, Double> resultMap = new TreeMap<String, Double>();
        
        SsbTable customer = new SsbTable("customer");
        for (int row = 0; row < customer.getRowCount(); ++ row) {
          String c_region = customer.getValue(row, "c_region");
          if (c_region.equals("ASIA")) {
            String c_custkey = customer.getValue(row, "c_custkey");
            String c_nation = customer.getValue(row, "c_nation");
            custMap.put(c_custkey, c_nation);
          }
        }
        
        SsbTable supplier = new SsbTable("supplier");
        for (int row = 0; row < supplier.getRowCount(); ++ row) {
          String s_region = supplier.getValue(row, "s_region");
          if (s_region.equals("ASIA")) {
            String s_suppkey = supplier.getValue(row, "s_suppkey");
            String s_nation = supplier.getValue(row, "s_nation");
            suppMap.put(s_suppkey, s_nation);
          }
        }
        
        SsbTable date = new SsbTable("date");
        for (int row = 0; row < date.getRowCount(); ++ row) {
          int d_year = date.getIntValue(row, "d_year");
          if (d_year >= 1992 && d_year <= 1997) {
            String d_datekey = date.getValue(row, "d_datekey");
            dateMap.put(d_datekey, String.valueOf(d_year));
          }
        }

        SsbTable lineorder = new SsbTable("lineorder");
        for (int row = 0; row < lineorder.getRowCount(); ++ row) {
          String lo_custkey = lineorder.getValue(row, "lo_custkey");
          String lo_suppkey = lineorder.getValue(row, "lo_suppkey");
          String lo_orderdate = lineorder.getValue(row, "lo_orderdate");
          
          if (custMap.containsKey(lo_custkey) && suppMap.containsKey(lo_suppkey) && dateMap.containsKey(lo_orderdate)) {
            Double lo_revenue = lineorder.getDoubleValue(row, "lo_revenue");
            String c_nation = custMap.get(lo_custkey);
            String s_nation = suppMap.get(lo_suppkey);
            String d_year = dateMap.get(lo_orderdate);
            String key = c_nation + "_" + s_nation + "_" + d_year;
            if (resultMap.containsKey(key)) {
              resultMap.put(key, resultMap.get(key) + lo_revenue);
            } else {
              resultMap.put(key, lo_revenue);
            }
          }
        }
        
        assertEquals(resultSet.getRowCount(), resultMap.size());
        for (int i = 0; i < resultSet.getRowCount(); ++ i) {
          ResultData data = resultSet.getResultData(i);
          String c_nation = data.getFieldValue("c_nation").toString();
          String s_nation = data.getFieldValue("s_nation").toString();
          String d_year = data.getFieldValue("d_year").toString();
          
          Double revenue = (Double) data.getFieldValue("revenue");
          String key = c_nation + "_" + s_nation + "_" + d_year;
          
          assertTrue(resultMap.containsKey(key));
          assertEquals(revenue, resultMap.get(key), 0.00000001);
        }
      }
      
      System.out.println("================================================================");
      System.out.println();
    } catch (Exception e) {
      e.printStackTrace();
    }
  }
  
}
