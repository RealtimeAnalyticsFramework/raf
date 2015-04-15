/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package integration.ssb;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class SsbTable {
  
  final public static String home = System.getenv("SSB_HOME") + "/ssb-dbgen-master/";
  
  private static Map<String, Integer> mapping;
  
  private static Map<String, List<String[]>> data;
  
  public static boolean checkRawResult = false;
  
  private String tblName;
  
  static {
    String envCheckRaw = System.getenv("CHECK_RAW_RESULT");
    checkRawResult = envCheckRaw != null && envCheckRaw.equalsIgnoreCase("true");

    if (checkRawResult) {
      try {
        initMapping();
        initTableData();
      } catch (Exception e) {
        e.printStackTrace();
      }
    }
  }
  
  public SsbTable(String tblName) {
    this.tblName = tblName;
  }
  
  public int getRowCount() {
    return data.get(tblName).size();
  }
  
  public String getValue(int row, String column) {
    Integer fieldIndex = mapping.get(column);
    if (fieldIndex == null) {
      return null;
    }
    
    return getValue(row, fieldIndex);
  }
  
  public String getValue(int row, int fieldIndex) {
    return data.get(tblName).get(row)[fieldIndex];
  }
  
  public Integer getIntValue(int row, String column) {
    String value = getValue(row, column);
    if (value == null) {
      return null;
    }
    
    return Integer.valueOf(value);
  }
  
  public Integer getIntValue(int row, int fieldIndex) {
    String value = getValue(row, fieldIndex);
    if (value == null) {
      return null;
    }
    
    return Integer.valueOf(value);
  }
  
  public Double getDoubleValue(int row, String column) {
    String value = getValue(row, column);
    if (value == null) {
      return null;
    }
    
    return Double.valueOf(value);
  }
  
  public Double getDoubleValue(int row, int fieldIndex) {
    String value = getValue(row, fieldIndex);
    if (value == null) {
      return null;
    }
    
    return Double.valueOf(value);
  }

  private static void initMapping() {
    mapping = new HashMap<String, Integer>();
    
    mapping.put("d_datekey", 0);
    mapping.put("d_date", 1);
    mapping.put("d_dateofweek", 2);
    mapping.put("d_month", 3);
    mapping.put("d_year", 4);
    mapping.put("d_yearmonthnum", 5);
    mapping.put("d_yearmonth", 6);
    mapping.put("d_daynuminweek", 7);
    mapping.put("d_daynuminmonth", 8);
    mapping.put("d_daynuminyear", 9);
    mapping.put("d_monthnuminyear", 10);
    mapping.put("d_weeknuminyear", 11);
    mapping.put("d_sellingseason", 12);
    mapping.put("d_lastdayinweekfl", 13);
    mapping.put("d_lastdayinmonthfl", 14);
    mapping.put("d_holidayfl", 15);
    mapping.put("d_weekdayfl", 16);
    
    mapping.put("p_partkey", 0);
    mapping.put("p_name", 1);
    mapping.put("p_mfgr", 2);
    mapping.put("p_category", 3);
    mapping.put("p_brand", 4);
    mapping.put("p_color", 5);
    mapping.put("p_type", 6);
    mapping.put("p_size", 7);
    mapping.put("p_container", 8);
    
    mapping.put("s_suppkey", 0);
    mapping.put("s_name", 1);
    mapping.put("s_address", 2);
    mapping.put("s_city", 3);
    mapping.put("s_nation", 4);
    mapping.put("s_region", 5);
    mapping.put("s_phone", 6);
    
    mapping.put("c_custkey", 0);
    mapping.put("c_name", 1);
    mapping.put("c_address", 2);
    mapping.put("c_city", 3);
    mapping.put("c_nation", 4);
    mapping.put("c_region", 5);
    mapping.put("c_phone", 6);
    mapping.put("c_mktsegment", 8);
    
    mapping.put("lo_orderkey", 0);
    mapping.put("lo_linenumber", 1);
    mapping.put("lo_custkey", 2);
    mapping.put("lo_partkey", 3);
    mapping.put("lo_suppkey", 4);
    mapping.put("lo_orderdate", 5);
    mapping.put("lo_orderpriority", 6);
    mapping.put("lo_shippriority", 7);
    mapping.put("lo_quantity", 8);
    mapping.put("lo_extendedprice", 9);
    mapping.put("lo_ordertotalprice", 10);
    mapping.put("lo_discount", 11);
    mapping.put("lo_revenue", 12);
    mapping.put("lo_supplycost", 13);
    mapping.put("lo_tax", 14);
    mapping.put("lo_commitdate", 15);
    mapping.put("lo_shipmode", 16);
  }

  private static void initTableData() throws Exception {
    data = new HashMap<String, List<String[]>>();
    loadTableData("date");
    loadTableData("supplier");
    loadTableData("part");
    loadTableData("customer");
    loadTableData("lineorder");
  }
  
  private static void loadTableData(String name) throws Exception {
    BufferedReader br = null;
    String line = null;
    data.put(name, new ArrayList<String[]>());
    try {
      br = new BufferedReader(new InputStreamReader(new FileInputStream(home + name + ".tbl")));
      while ((line = br.readLine()) != null) {
        String[] values = line.split("\\|");
        data.get(name).add(values);
      }
    } catch (Exception e) {
      throw e;
    } finally {
      if (br != null) {
        br.close();
      }
    }
  }
  
}
