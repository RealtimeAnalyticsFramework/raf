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
import java.util.TreeMap;

import printer.DefaultResultPrinter;
import printer.ResultPrinter;
import idgs.IdgsCliDriver;
import idgs.execution.RowData;
import idgs.execution.ResultSet;
import junit.framework.TestCase;

public class SsbQ4_1IT extends TestCase {
  
  public void testSSBQ4_1() {
    StringBuffer sql = new StringBuffer();
    sql.append("  SELECT d.d_year,\n")
       .append("         c.c_nation,\n")
       .append("         sum(lo.lo_revenue - lo.lo_supplycost) AS profit\n")
       .append("  FROM   ssb.ssb_lineorder lo\n")
       .append("         JOIN ssb.ssb_date d\n")
       .append("           ON lo.lo_orderdate = d.d_datekey\n")
       .append("         JOIN ssb.ssb_customer c\n")
       .append("           ON lo.lo_custkey = c.c_custkey\n")
       .append("         JOIN ssb.ssb_supplier s\n")
       .append("           ON lo.lo_suppkey = s.s_suppkey\n")
       .append("         JOIN ssb.ssb_part p\n")
       .append("           ON lo.lo_partkey = p.p_partkey\n")
       .append("  WHERE  c.c_region = 'AMERICA'\n")
       .append("         AND s.s_region = 'AMERICA'\n")
       .append("         AND ( p.p_mfgr = 'MFGR#1'\n")
       .append("                OR p.p_mfgr = 'MFGR#2' )\n")
       .append("  GROUP  BY d.d_year,\n")
       .append("            c.c_nation\n")
       .append("  ORDER  BY d_year,\n")
       .append("            c_nation\n");
    
    System.out.println("========================= run SSB Q4.1 =========================");
    System.out.println(sql.toString());
    
    try {
      long start = System.currentTimeMillis();
      ResultSet resultSet = IdgsCliDriver.run(sql.toString());
      double time = (System.currentTimeMillis() - start) / 1000.0;
      assertNotNull(resultSet);
      
      System.out.println("The top 20 results is : ");
      ResultPrinter printer = new DefaultResultPrinter(resultSet);
      System.out.println(printer.printResults(20));
      
      File file = new File("Q4.1.txt");
      FileOutputStream fos = new FileOutputStream(file);
      fos.write(("Q4.1 " + new DecimalFormat("0.00").format(time)).getBytes());
      fos.close();
      
      if (SsbTable.checkRawResult) {
        Map<String, String> dateMap = new HashMap<String, String>();
        Map<String, String> custMap = new HashMap<String, String>();
        Map<String, String> suppMap = new HashMap<String, String>();
        Map<String, String> partMap = new HashMap<String, String>();
        Map<String, Double> resultMap = new TreeMap<String, Double>();
        
        SsbTable date = new SsbTable("date");
        for (int row = 0; row < date.getRowCount(); ++ row) {
          String d_datekey = date.getValue(row, "d_datekey");
          String d_year = date.getValue(row, "d_year");
          dateMap.put(d_datekey, d_year);
        }
        
        SsbTable customer = new SsbTable("customer");
        for (int row = 0; row < customer.getRowCount(); ++ row) {
          String c_region = customer.getValue(row, "c_region");
          if (c_region.equals("AMERICA")) {
            String c_custkey = customer.getValue(row, "c_custkey");
            String c_nation = customer.getValue(row, "c_nation");
            custMap.put(c_custkey, c_nation);
          }
        }
        
        SsbTable supplier = new SsbTable("supplier");
        for (int row = 0; row < supplier.getRowCount(); ++ row) {
          String s_region = supplier.getValue(row, "s_region");
          if (s_region.equals("AMERICA")) {
            String s_suppkey = supplier.getValue(row, "s_suppkey");
            suppMap.put(s_suppkey, null);
          }
        }
        
        SsbTable part = new SsbTable("part");
        for (int row = 0; row < part.getRowCount(); ++ row) {
          String p_mfgr = part.getValue(row, "p_mfgr");
          if (p_mfgr.equals("MFGR#1") || p_mfgr.equals("MFGR#2")) {
            String p_partkey = part.getValue(row, "p_partkey");
            partMap.put(p_partkey, null);
          }
        }
        
        SsbTable lineorder = new SsbTable("lineorder");
        for (int row = 0; row < lineorder.getRowCount(); ++ row) {
          String lo_orderdate = lineorder.getValue(row, "lo_orderdate");
          String lo_custkey = lineorder.getValue(row, "lo_custkey");
          String lo_suppkey = lineorder.getValue(row, "lo_suppkey");
          String lo_partkey = lineorder.getValue(row, "lo_partkey");
          
          if (dateMap.containsKey(lo_orderdate) && custMap.containsKey(lo_custkey) && suppMap.containsKey(lo_suppkey) && partMap.containsKey(lo_partkey)) {
            Double lo_revenue = lineorder.getDoubleValue(row, "lo_revenue");
            Double lo_supplycost = lineorder.getDoubleValue(row, "lo_supplycost");
            String d_year = dateMap.get(lo_orderdate);
            String c_nation = custMap.get(lo_custkey);
            String key = d_year + "_" + c_nation;
            if (resultMap.containsKey(key)) {
              resultMap.put(key, resultMap.get(key) + (lo_revenue - lo_supplycost));
            } else {
              resultMap.put(key, (lo_revenue - lo_supplycost));
            }
          }
        }
        
        assertEquals(resultSet.getRowCount(), resultMap.size());
        for (int i = 0; i < resultSet.getRowCount(); ++ i) {
          RowData data = resultSet.getResultData(i);
          String d_year = data.getFieldValue("d_year").toString();
          String c_nation = data.getFieldValue("c_nation").toString();
          
          Double profit = (Double) data.getFieldValue("profit");
          String key = d_year + "_" + c_nation;
          
          assertTrue(resultMap.containsKey(key));
          assertEquals(profit, resultMap.get(key), 0.00000001);
        }
      }
      
      System.out.println("================================================================");
      System.out.println();
    } catch (Exception e) {
      e.printStackTrace();
    }
  }
  
}
