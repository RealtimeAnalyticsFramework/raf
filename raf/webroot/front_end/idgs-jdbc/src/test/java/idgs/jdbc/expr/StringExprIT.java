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

import idgs.jdbc.BaseCase;

public class StringExprIT extends BaseCase {

  public void testStringExpr() {
    StringBuffer sql = new StringBuffer();
    sql.append("  select c_name, \n")
       .append("         substr(c_name, 2) substr1, substring(c_name, 2, 2) substr2, \n")
       .append("         length(c_name) len, \n")
       .append("         reverse(c_name) reverse, \n")
       .append("         concat(c_name, 'INTEL') concat, \n")
       .append("         concat_ws('___', c_name, 'INTEL') concat_ws, \n")
       .append("         upper(c_name) upper, \n")
       .append("         ucase(c_name) ucase, \n")
       .append("         lower(c_name) lower, \n")
       .append("         lcase(c_name) lcase, \n")
       .append("         trim(concat('  ', c_name, '  ')) trim, \n")
       .append("         ltrim(concat('  ', c_name)) ltrim, \n")
       .append("         rtrim(concat(c_name, '  ')) rtrim, \n")
       .append("         space(8) space, \n")
       .append("         repeat(c_name, 2) repeat, \n")
       .append("         ascii(c_name) ascii, \n")
       .append("         lpad(c_name, 20, ' ') lpad, \n")
       .append("         rpad(c_name, 20, '0') rpad, \n")
       .append("         locate('Customer', c_name) locate1, \n")
       .append("         locate('Customer', c_name, 4) locate2, \n")
       .append("         instr(c_name, 'Customer') instr, \n")
       .append("         find_in_set('bc', 'a,bc,def,g') findinset, \n")
       .append("         parse_url('http://www.intel.com', 'HOST') host, \n")
       .append("         parse_url('http://www.intel.com', 'PROTOCOL') protocol, \n")
       .append("         regexp('hello foothebar', 'foo(.*?)(bar)') regexp, \n")
       .append("         regexp_replace('hello foothebar', 'foo(.*?)(bar)', 'world') replace, \n")
       .append("         regexp_extract('hello foothebar', 'foo(.*?)(bar)') extract \n")
       .append("  from   tpch.customer \n")
       .append("  where  c_name like '%1' \n")
       .append("  limit  10");
    
    System.out.println("test sql : ");
    System.out.println(sql.toString());
    
    Connection conn = null;
    
    try {
      conn = DriverManager.getConnection("");
      Statement stmt = conn.createStatement();
      ResultSet rs = stmt.executeQuery(sql.toString());
      
      System.out.println();
      String formatter = "| %s | %s | %s | %d | %s | %s | %s | %s | %s | %s "
                       + "| %s | %s | %s | %s | %s | %s | %s | %s | %d | %d "
                       + "| %d | %d | %s | %s | %s | %s | %s |\n";
      
      while (rs.next()) {
        String name = (String) rs.getObject("c_name");
        String substr1 = (String) rs.getObject("substr1");
        String substr2 = (String) rs.getObject("substr2");
        Integer len = (Integer) rs.getObject("len");
        String reverse = (String) rs.getObject("reverse");
        String concat = (String) rs.getObject("concat");
        String concat_ws = (String) rs.getObject("concat_ws");
        String upper = (String) rs.getObject("upper");
        String ucase = (String) rs.getObject("ucase");
        String lower = (String) rs.getObject("lower");
        String lcase = (String) rs.getObject("lcase");
        String trim = (String) rs.getObject("trim");
        String ltrim = (String) rs.getObject("ltrim");
        String rtrim = (String) rs.getObject("rtrim");
        String space = (String) rs.getObject("space");
        String repeat = (String) rs.getObject("repeat");
        String lpad = (String) rs.getObject("lpad");
        String rpad = (String) rs.getObject("rpad");
        Integer locate1 = (Integer) rs.getObject("locate1");
        Integer locate2 = (Integer) rs.getObject("locate2");
        Integer instr = (Integer) rs.getObject("instr");
        Integer findinset = (Integer) rs.getObject("findinset");
        String host = (String) rs.getObject("host");
        String protocol = (String) rs.getObject("protocol");
        Boolean regexp = (Boolean) rs.getObject("regexp");
        String replace = (String) rs.getObject("replace");
        String extract = (String) rs.getObject("extract");
        
        // test substr
        assertEquals(name.substring(2), substr1);
        assertEquals(name.substring(2, 4), substr2);
        // test like
        assertTrue(name.endsWith("1"));
        // test length
        assertEquals(name.length(), len.intValue());
        // test reverse
        assertEquals(new StringBuffer(name).reverse().toString(), reverse);
        // test concat
        assertEquals(name + "INTEL", concat);
        // test concat_ws
        assertEquals(name + "___INTEL", concat_ws);
        // test upper
        assertEquals(name.toUpperCase(), upper);
        assertEquals(name.toUpperCase(), ucase);
        // test lower
        assertEquals(name.toLowerCase(), lower);
        assertEquals(name.toLowerCase(), lcase);
        // test trim
        assertEquals(name, trim);
        assertEquals(name, ltrim);
        assertEquals(name, rtrim);
        // test space
        assertEquals("        ", space);
        // test repeat
        assertEquals(name + name, repeat);
        // test lpad
        assertEquals("  " + name, lpad);
        // test rpad
        assertEquals(name + "00", rpad);
        // test locate
        assertEquals(1, locate1.intValue());
        assertEquals(0, locate2.intValue());
        // test instr
        assertEquals(1, instr.intValue());
        // test find_in_set
        assertEquals(2, findinset.intValue());
        // test parse_url
        assertEquals("www.intel.com", host);
        assertEquals("http", protocol);
        // test regexp
        assertTrue(regexp);
        // test regexp_replace
        assertEquals("hello world", replace);
        // test regexp_extract
        assertEquals("the", extract);
        
        System.out.printf(formatter, name, substr1, substr2, len, reverse, concat, concat_ws, 
            upper, ucase, lower, lcase, trim, ltrim, rtrim,space, repeat, lpad, rpad,
            locate1, locate2, instr, findinset, host, protocol, regexp, replace, extract);
      }
    } catch (Exception e) {
      e.printStackTrace();
      fail();
    }
  }
  
}
