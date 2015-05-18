/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package integration.expr;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

import idgs.IdgsCliDriver;
import idgs.execution.RowData;
import idgs.execution.ResultSet;
import junit.framework.TestCase;

public class DateExprIT extends TestCase {

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
       .append("  from tpch.customer \n")
       .append("  limit 1");
    try {
      System.out.println("test sql : ");
      System.out.println(sql.toString());
      ResultSet resultSet = IdgsCliDriver.run(sql.toString());
      assertNotNull(resultSet);
      
      assertEquals(1, resultSet.getRowCount());
      for (int i = 0; i < resultSet.getRowCount(); ++ i) {
        RowData data = resultSet.getResultData(i);
        String unixtime1 = (String) data.getFieldValue("unixtime1");
        String unixtime2 = (String) data.getFieldValue("unixtime2");
        Long timestamp1 = (Long) data.getFieldValue("timestamp1");
        Long timestamp2 = (Long) data.getFieldValue("timestamp2");
        String todate = (String) data.getFieldValue("to_date");
        Integer year = (Integer) data.getFieldValue("year");
        Integer month = (Integer) data.getFieldValue("month");
        Integer day = (Integer) data.getFieldValue("day");
        Integer dayofmonth = (Integer) data.getFieldValue("dayofmonth");
        Integer hour = (Integer) data.getFieldValue("hour");
        Integer minute = (Integer) data.getFieldValue("minute");
        Integer second = (Integer) data.getFieldValue("second");
        Integer weekofyear = (Integer) data.getFieldValue("weekofyear");
        Integer datediff = (Integer) data.getFieldValue("datediff");
        String dateadd = (String) data.getFieldValue("dateadd");
        String datesub = (String) data.getFieldValue("datesub");
        
        // test from_unixtime
        System.out.println("test from_unixtime " + datetime + ", " + unixtime1);
        assertEquals(datetime, unixtime1);
        System.out.println("test from_unixtime " + fmtDate.format(curDate) + ", " + unixtime2);
        assertEquals(fmtDate.format(curDate), unixtime2);
        // test unix_timestamp
        assertTrue(timestamp1.longValue() - timestamp < 60);
        System.out.println("test unix_timestamp " + timestamp + ", " + timestamp2.longValue());
        assertEquals(timestamp, timestamp2.longValue());
        // test to_date
        System.out.println("test to_date " + date + ", " + todate);
        assertEquals(date, todate);
        // test year
        System.out.println("test year " + c.get(Calendar.YEAR) + ", " + year.intValue());
        assertEquals(c.get(Calendar.YEAR), year.intValue());
        // test month
        System.out.println("test month " + c.get(Calendar.MONTH) + 1 + ", " + month.intValue());
        assertEquals(c.get(Calendar.MONTH) + 1, month.intValue());
        // test day
        System.out.println("test day " + c.get(Calendar.DATE) + ", " + day.intValue() + ", " + dayofmonth.intValue());
        assertEquals(c.get(Calendar.DATE), day.intValue());
        assertEquals(c.get(Calendar.DATE), dayofmonth.intValue());
        // test hour
        System.out.println("test hour " + c.get(Calendar.HOUR_OF_DAY) + "," +  hour.intValue());
        assertEquals(c.get(Calendar.HOUR_OF_DAY), hour.intValue());
        // test minute
        System.out.println(c.get(Calendar.MINUTE) + ", " + minute.intValue());
        assertEquals(c.get(Calendar.MINUTE), minute.intValue());
        // test second
        System.out.println("test second " + c.get(Calendar.SECOND) + ", " + second.intValue());
        assertEquals(c.get(Calendar.SECOND), second.intValue());
        // test weekofyear
        System.out.println("test weekofyear " + c.get(Calendar.WEEK_OF_YEAR) + ", " + weekofyear.intValue());
//        assertEquals(0, Math.abs(c.get(Calendar.WEEK_OF_YEAR) - weekofyear.intValue()) % 52);
        // test datediff
        System.out.println("test datediff " + 30 + ", " + datediff.longValue());
        assertEquals(30, datediff.longValue());
        // test dateadd
        System.out.println("test dateadd " + dateaddWeek + ", " + dateadd);
        assertEquals(dateaddWeek, dateadd);
        // test datesub
        System.out.println("test datesub " + datesubWeek + ", " + datesub);
        assertEquals(datesubWeek, datesub);
      }
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    }
  }
  
}
