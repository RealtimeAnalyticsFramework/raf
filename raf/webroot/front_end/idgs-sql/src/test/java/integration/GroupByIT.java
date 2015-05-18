/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package integration;

import printer.DefaultResultPrinter;
import printer.ResultPrinter;
import idgs.IdgsCliDriver;
import idgs.execution.RowData;
import idgs.execution.ResultSet;
import junit.framework.TestCase;

public class GroupByIT extends TestCase {

  public void testGroupBy() {
    StringBuffer sql = new StringBuffer();
    sql.append("  select l_suppkey, \n")
       .append("         count(l_suppkey) cnt, \n")
       .append("         sum(l_suppkey) sum, \n")
       .append("         avg(l_suppkey) avg \n")
       .append("  from   tpch.LINEITEM \n")
       .append("  group  by l_suppkey\n");
    
    System.out.println("run test sql : ");
    System.out.println(sql);
    
    try {
      ResultSet resultSet = IdgsCliDriver.run(sql.toString());
      assertNotNull(resultSet);
      
      ResultPrinter printer = new DefaultResultPrinter(resultSet);
      System.out.println(printer.printResults());
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    }
  }
  
  public void testGroupByAggrConst() {
    StringBuffer sql = new StringBuffer();
    sql.append("  select l_suppkey, \n")
       .append("         count(1) cnt, \n")
       .append("         sum(1) sum \n")
       .append("  from   tpch.LINEITEM \n")
       .append("  group  by l_suppkey\n");
    
    System.out.println("run test sql : ");
    System.out.println(sql);
    
    try {
      ResultSet resultSet = IdgsCliDriver.run(sql.toString());
      assertNotNull(resultSet);
      
      ResultPrinter printer = new DefaultResultPrinter(resultSet);
      System.out.println(printer.printResults());
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    }
  }
  
  public void testGroupByWithoutAggr() {
    StringBuffer sql = new StringBuffer();
    sql.append("  select l_suppkey \n")
       .append("  from   tpch.LINEITEM \n")
       .append("  group  by l_suppkey\n");
    
    System.out.println("run test sql : ");
    System.out.println(sql);
    
    try {
      ResultSet resultSet = IdgsCliDriver.run(sql.toString());
      assertNotNull(resultSet);
      
      ResultPrinter printer = new DefaultResultPrinter(resultSet);
      System.out.println(printer.printResults());
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    }
  }
  
  public void testGroupBySubQuery() {
    StringBuffer sql = new StringBuffer();
    sql.append("  select l_suppkey, \n")
       .append("         count(1) cnt \n")
       .append("  from   (select l_suppkey \n")
       .append("          from   tpch.lineitem \n")
       .append("          where  l_discount = 0.05) t \n")
       .append("  group  by l_suppkey\n");
    
    System.out.println("run test sql : ");
    System.out.println(sql);
    
    try {
      ResultSet resultSet = IdgsCliDriver.run(sql.toString());
      assertNotNull(resultSet);
      
      ResultPrinter printer = new DefaultResultPrinter(resultSet);
      System.out.println(printer.printResults());
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    }
  }

  public void testGroupByKey() {
    StringBuffer sql = new StringBuffer();
    sql.append("  select c_custkey, \n")
       .append("         count(distinct c_acctbal) cnt, \n")
       .append("         sum(distinct c_acctbal) sum \n")
       .append("  from   tpch.customer \n")
       .append("  group  by c_custkey\n");
    
    System.out.println("run test sql : ");
    System.out.println(sql);
    
    try {
      ResultSet resultSet = IdgsCliDriver.run(sql.toString());
      assertNotNull(resultSet);
      
      ResultPrinter printer = new DefaultResultPrinter(resultSet);
      System.out.println(printer.printResults());
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    }
  }

  public void testWholeTable() {
    StringBuffer sql = new StringBuffer();
    sql.append("  select sum(c_acctbal) sum, \n")
       .append("         count(c_custkey) cnt, \n")
       .append("         avg(c_acctbal) avg \n")
       .append("  from   tpch.customer\n");
    
    System.out.println("run test sql : ");
    System.out.println(sql);
    
    try {
      ResultSet resultSet = IdgsCliDriver.run(sql.toString());
      assertNotNull(resultSet);
      
      ResultPrinter printer = new DefaultResultPrinter(resultSet);
      System.out.println(printer.printResults());
      
      RowData data = resultSet.getResultData(0);
      double sum = (Double) data.getFieldValue("sum");
      long cnt = (Long) data.getFieldValue("cnt");
      double avg = (Double) data.getFieldValue("avg");

      assertEquals(sum / cnt, avg);
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    }
  }
  
  public void testGroupByWithDistinct() {
    StringBuffer sql = new StringBuffer();
    sql.append("  select l_suppkey, \n")
       .append("         sum(distinct l_discount) s1, \n")
       .append("         sum(l_discount) s2, \n")
       .append("         avg(distinct l_quantity) s3, \n")
       .append("         avg(l_quantity) s4, \n")
       .append("         count(distinct l_tax) s5, \n")
       .append("         count(l_tax) s6, \n")
       .append("         sum(l_extendedprice) s7, \n")
       .append("         count(distinct l_suppkey) s8 \n")
       .append("  from   tpch.lineitem \n")
       .append("  where  l_orderkey <= 30 \n")
       .append("  group  by l_suppkey, l_orderkey\n");
    
    System.out.println("run test sql : ");
    System.out.println(sql);
    
    try {
      ResultSet resultSet = IdgsCliDriver.run(sql.toString());
      assertNotNull(resultSet);

      ResultPrinter printer = new DefaultResultPrinter(resultSet);
      System.out.println(printer.printResults());
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    }
  }

  public void testWholeTableWithDistinct() {
    StringBuffer sql = new StringBuffer();
    sql.append("  select sum(distinct l_discount) c1, \n")
       .append("         avg(distinct l_discount) c2, \n")
       .append("         avg(l_discount) c3, \n")
       .append("         sum(distinct l_suppkey) c4, \n")
       .append("         sum(l_discount) c5, \n")
       .append("         sum(l_quantity) c6 \n")
       .append("  from   tpch.lineitem\n");

    System.out.println("run test sql : ");
    System.out.println(sql);

    try {
      ResultSet resultSet = IdgsCliDriver.run(sql.toString());
      assertNotNull(resultSet);
      
      ResultPrinter printer = new DefaultResultPrinter(resultSet);
      System.out.println(printer.printResults());
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    }
  }
  
}
