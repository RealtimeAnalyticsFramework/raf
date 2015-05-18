/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package integration.expr;

import java.sql.Timestamp;
import java.text.SimpleDateFormat;
import java.util.Date;

import idgs.IdgsCliDriver;
import idgs.execution.RowData;
import idgs.execution.ResultSet;
import junit.framework.TestCase;

public class CastExprIT extends TestCase {

  public void testCastExpr() {
    SimpleDateFormat fmtTime = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.S");
    Date curDate = new Date();
    String date = fmtTime.format(curDate);
    
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
       .append("  from tpch.customer \n")
       .append("  limit 10");
    try {
      System.out.println("test sql : ");
      System.out.println(sql.toString());
      ResultSet resultSet = IdgsCliDriver.run(sql.toString());
      assertNotNull(resultSet);
      
      assertEquals(10, resultSet.getRowCount());
      for (int i = 0; i < resultSet.getRowCount(); ++ i) {
        RowData data = resultSet.getResultData(i);
        Long custkey = (Long) data.getFieldValue("c_custkey");
        Double acctbal = (Double) data.getFieldValue("c_acctbal");
        String name = (String) data.getFieldValue("c_name");
        String tostring = (String) data.getFieldValue("tostring");
        Long tolong = (Long) data.getFieldValue("tolong");
        Integer toint = (Integer) data.getFieldValue("toint");
        Integer toshort = (Integer) data.getFieldValue("toshort");
        Integer tobyte = (Integer) data.getFieldValue("tobyte");
        Float tofloat = (Float) data.getFieldValue("tofloat");
        Double todouble = (Double) data.getFieldValue("todouble");
        String tobinary = (String) data.getFieldValue("tobinary");
        Double todecimal = (Double) data.getFieldValue("todecimal");
        String totimestamp = (String) data.getFieldValue("totimestamp");
        
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
        assertEquals(name, tobinary);
        // test cast as decimal
        assertEquals(acctbal, todecimal);
        // test cast as timestamp
        Timestamp ts = Timestamp.valueOf(totimestamp);
        assertEquals(curDate.getTime() / 1000, ts.getTime() / 1000);
      }
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    }
  }
  
}
