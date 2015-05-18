/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.jdbc.expr;

import java.math.BigDecimal;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.Statement;
import java.sql.Timestamp;
import java.text.SimpleDateFormat;
import java.util.Date;

import idgs.jdbc.BaseCase;

public class CastExprIT extends BaseCase {

  public void testCastExpr() {
    SimpleDateFormat fmtTime = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.S");
    Date curDate = new Date();
    String date = fmtTime.format(curDate);
    long timestamp = curDate.getTime() / 1000;
    
    StringBuffer sql = new StringBuffer();
    sql.append("  select c_custkey, \n")
       .append("         c_acctbal, \n")
       .append("         c_name, \n")
       .append("         cast(c_acctbal as string) tostring, \n")
       .append("         cast(c_acctbal as bigint) tolong, \n")
       .append("         cast(c_acctbal as int) toint, \n")
       .append("         cast(c_custkey as smallint) toshort, \n")
       .append("         cast(c_custkey as tinyint) tobyte, \n")
       .append("         cast(c_acctbal as float) tofloat, \n")
       .append("         cast(c_custkey as double) todouble, \n")
       .append("         cast(c_custkey as boolean) tobool, \n")
       .append("         cast(c_name as binary) tobinary, \n")
       .append("         cast(c_acctbal as decimal) todecimal, \n")
       .append("         cast('" + date + "' as timestamp) totimestamp \n")
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
      String formatter = "| %3d | %7.2f | %s | %7s | %4d | %4d | %3d | %4d | %7.2f | %6.2f | %s | %7.2f | %s |\n";
      while (rs.next()) {
        Long custkey = (Long) rs.getObject("c_custkey");
        Double acctbal = (Double) rs.getObject("c_acctbal");
        String name = (String) rs.getObject("c_name");
        String tostring = (String) rs.getObject("tostring");
        Long tolong = (Long) rs.getObject("tolong");
        Integer toint = (Integer) rs.getObject("toint");
        Short toshort = (Short) rs.getObject("toshort");
        Byte tobyte = (Byte) rs.getObject("tobyte");
        Double tofloat = (Double) rs.getObject("tofloat");
        Double todouble = (Double) rs.getObject("todouble");
        byte[] tobinary = (byte[]) rs.getObject("tobinary");
        BigDecimal todecimal = (BigDecimal) rs.getObject("todecimal");
        Timestamp totimestamp = (Timestamp) rs.getObject("totimestamp");
        
        // test cast as string
        assertEquals(acctbal.toString(), tostring);
        // test cast as long
        assertEquals(acctbal.longValue(), tolong.longValue());
        // test cast as int
        assertEquals(acctbal.intValue(), toint.intValue());
        // test cast as smallint
        assertEquals(custkey.shortValue(), toshort.shortValue());
        // test cast as tinyint
        assertEquals(custkey.byteValue(), tobyte.byteValue());
        // test cast as float
        assertEquals(acctbal.floatValue(), tofloat.floatValue());
        // test cast as double
        assertEquals(custkey.doubleValue(), todouble.doubleValue());
        // test cast as binary
        assertEquals(name, new String(tobinary));
        // test cast as decimal
        assertEquals(acctbal, todecimal.doubleValue());
        // test cast as timestamp
        assertEquals(timestamp, totimestamp.getTime() / 1000);
        
        System.out.printf(formatter, custkey, acctbal, name, tostring, 
            tolong, toint, toshort, tobyte, tofloat, todouble, 
            new String(tobinary), todecimal, totimestamp);
      }
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    }
  }
  
}
