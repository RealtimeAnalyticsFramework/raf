/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.jdbc.expr;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.Statement;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

import idgs.jdbc.BaseCase;

public class DateExprIT extends BaseCase {

  public void testDateExpr() {
    SimpleDateFormat fmtTime = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
    SimpleDateFormat fmtDate = new SimpleDateFormat("yyyy-MM-dd");
    
    Calendar c = Calendar.getInstance();
    Date curDate = c.getTime();
    
    String datetime = fmtTime.format(curDate);
    String date = fmtDate.format(curDate);
    long timestamp = curDate.getTime() / 1000;
    
    c.add(Calendar.DATE, 30);
    String diffdate = fmtTime.format(c.getTime());

    c.setTime(curDate);
    c.add(Calendar.DATE, 7);
    String dateaddWeek = fmtDate.format(c.getTime());
    
    c.setTime(curDate);
    c.add(Calendar.DATE, -7);
    String datesubWeek = fmtDate.format(c.getTime());
    
    c.setTime(curDate);
    
    StringBuffer sql = new StringBuffer();
    sql.append("  select from_unixtime(" + timestamp + ") unixtime1, \n")
       .append("         from_unixtime(" + timestamp + ", 'yyyy-MM-dd') unixtime2, \n")
       .append("         unix_timestamp() timestamp1, \n")
       .append("         unix_timestamp('" + datetime + "') timestamp2, \n")
       .append("         to_date('" + datetime + "') to_date, \n")
       .append("         year('" + datetime + "') year, \n")
       .append("         month('" + datetime + "') month, \n")
       .append("         day('" + datetime + "') day, \n")
       .append("         dayofmonth('" + datetime + "') dayofmonth, \n")
       .append("         hour('" + datetime + "') hour, \n")
       .append("         minute('" + datetime + "') minute, \n")
       .append("         second('" + datetime + "') second, \n")
       .append("         weekofyear('" + datetime + "') weekofyear, \n")
       .append("         datediff('" + diffdate + "', '" + datetime + "') datediff, \n")
       .append("         date_add('" + datetime + "', 7) dateadd, \n")
       .append("         date_sub('" + datetime + "', 7) datesub \n")
       .append("  from   tpch.customer \n")
       .append("  limit  1");
    
    System.out.println("test sql : ");
    System.out.println(sql.toString());
    
    Connection conn = null;
    
    try {
      conn = DriverManager.getConnection("");
      Statement stmt = conn.createStatement();
      ResultSet rs = stmt.executeQuery(sql.toString());
      
      System.out.println();
      String formatter = "| %s | %s | %d | %d | %s | %d | %d | %d | %d | %d | %d | %d | %d | %s | %s |\n";
      while (rs.next()) {
        String unixtime1 = (String) rs.getObject("unixtime1");
        String unixtime2 = (String) rs.getObject("unixtime2");
        Long timestamp1 = (Long) rs.getObject("timestamp1");
        Long timestamp2 = (Long) rs.getObject("timestamp2");
        String todate = (String) rs.getObject("to_date");
        Integer year = (Integer) rs.getObject("year");
        Integer month = (Integer) rs.getObject("month");
        Integer day = (Integer) rs.getObject("day");
        Integer dayofmonth = (Integer) rs.getObject("dayofmonth");
        Integer hour = (Integer) rs.getObject("hour");
        Integer minute = (Integer) rs.getObject("minute");
        Integer second = (Integer) rs.getObject("second");
        Integer weekofyear = (Integer) rs.getObject("weekofyear");
        Integer datediff = (Integer) rs.getObject("datediff");
        String dateadd = (String) rs.getObject("dateadd");
        String datesub = (String) rs.getObject("datesub");
        
        // test from_unixtime
        assertEquals(datetime, unixtime1);
        assertEquals(fmtDate.format(curDate), unixtime2);
        // test unix_timestamp
        assertTrue(timestamp1.longValue() - timestamp < 60);
        assertEquals(timestamp, timestamp2.longValue());
        // test to_date
        assertEquals(date, todate);
        // test year
        assertEquals(c.get(Calendar.YEAR), year.intValue());
        // test month
        assertEquals(c.get(Calendar.MONTH) + 1, month.intValue());
        // test day
        assertEquals(c.get(Calendar.DATE), day.intValue());
        assertEquals(c.get(Calendar.DATE), dayofmonth.intValue());
        // test hour
        assertEquals(c.get(Calendar.HOUR_OF_DAY), hour.intValue());
        // test minute
        assertEquals(c.get(Calendar.MINUTE), minute.intValue());
        // test second
        assertEquals(c.get(Calendar.SECOND), second.intValue());
        // test weekofyear
//        assertEquals(0, Math.abs(c.get(Calendar.WEEK_OF_YEAR) - weekofyear.intValue()) % 52);
        // test datediff
        assertEquals(30, datediff.longValue());
        // test dateadd
        assertEquals(dateaddWeek, dateadd);
        // test datesub
        assertEquals(datesubWeek, datesub);
        
        System.out.printf(formatter, unixtime1, unixtime2, timestamp1, timestamp2, 
            todate, year, month, day, dayofmonth, hour, minute, second, 
            weekofyear, datediff, dateadd, datesub);
      }
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    }
  }
  
}
