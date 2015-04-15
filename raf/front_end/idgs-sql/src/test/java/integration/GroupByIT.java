/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package integration;

import printer.DefaultResultPrinter;
import printer.ResultPrinter;
import idgs.IdgsCliDriver;
import idgs.execution.ResultData;
import idgs.execution.ResultSet;
import junit.framework.TestCase;

public class GroupByIT extends TestCase {

  public void testGroupBy() {
    String sql = "select l_suppkey, count(l_suppkey) cnt, sum(l_suppkey) sum, avg(l_suppkey) avg from LINEITEM group by l_suppkey";
    System.out.println(sql);
    try {
      ResultSet resultSet = IdgsCliDriver.run(sql);
      assertNotNull(resultSet);
      
      ResultPrinter printer = new DefaultResultPrinter(resultSet);
      System.out.println(printer.printResults());
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    }
  }
  
  public void testGroupByConst() {
    String sql = "select l_suppkey, count(1) cnt from LINEITEM group by l_suppkey";
    System.out.println(sql);
    try {
      ResultSet resultSet = IdgsCliDriver.run(sql);
      assertNotNull(resultSet);
      
      ResultPrinter printer = new DefaultResultPrinter(resultSet);
      System.out.println(printer.printResults());
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    }
  }
  
  public void testGroupByWithNoAggr() {
    String sql = "select l_suppkey from LINEITEM group by l_suppkey";
    System.out.println(sql);
    try {
      ResultSet resultSet = IdgsCliDriver.run(sql);
      assertNotNull(resultSet);
      
      ResultPrinter printer = new DefaultResultPrinter(resultSet);
      System.out.println(printer.printResults());
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    }
  }
  
  public void testGroupBySubQuery() {
    String sql = "select l_suppkey, count(1) cnt from (select l_suppkey from lineitem where l_discount = 0.05) t group by l_suppkey";
    System.out.println(sql);
    try {
      ResultSet resultSet = IdgsCliDriver.run(sql);
      assertNotNull(resultSet);
      
      ResultPrinter printer = new DefaultResultPrinter(resultSet);
      System.out.println(printer.printResults());
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    }
  }

  public void testGroupByKey() {
    String sql = "select c_custkey, count(distinct c_acctbal), sum(distinct c_acctbal) from customer group by c_custkey";
    System.out.println(sql);
    try {
      ResultSet resultSet = IdgsCliDriver.run(sql);
      assertNotNull(resultSet);
      
      ResultPrinter printer = new DefaultResultPrinter(resultSet);
      System.out.println(printer.printResults());
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    }
  }

  public void testWholeTable() {
    try {
      String sql = "select sum(c_acctbal) sum, count(c_custkey) cnt, avg(c_acctbal) avg from customer";
      
      System.out.println(sql);
      ResultSet resultSet = IdgsCliDriver.run(sql);
      assertNotNull(resultSet);
      
      ResultPrinter printer = new DefaultResultPrinter(resultSet);
      System.out.println(printer.printResults());
      
      ResultData data = resultSet.getResultData(0);
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
    try {
      String sql = "select l_suppkey, "
          + "sum(distinct l_discount) s1, "
          + "sum(l_discount) s2, "
          + "avg(distinct l_quantity) s3, "
          + "avg(l_quantity) s5, "
          + "count(distinct l_tax) s6, "
          + "count(l_tax) s7, "
          + "sum(l_extendedprice) s8, "
          + "count(distinct l_suppkey) s9 "
          + "from lineitem where l_orderkey <= 30 group by l_suppkey, l_orderkey";
      
      System.out.println(sql);
      ResultSet resultSet = IdgsCliDriver.run(sql);
      assertNotNull(resultSet);

      ResultPrinter printer = new DefaultResultPrinter(resultSet);
      System.out.println(printer.printResults());
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    }
  }

  public void testWholeTableWithDistinct() {
    try {
      String sql = "select sum(distinct l_discount) c1, "
                        + "avg(distinct l_discount) c2, "
                        + "avg(l_discount) c3, "
                        + "sum(distinct l_suppkey) c4, "
                        + "sum(l_discount) c5, "
                        + "sum(l_quantity) c6 "
                        + "from lineitem";
      
      System.out.println(sql);
      ResultSet resultSet = IdgsCliDriver.run(sql);
      assertNotNull(resultSet);
      
      ResultPrinter printer = new DefaultResultPrinter(resultSet);
      System.out.println(printer.printResults());
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    }
  }
  
}
