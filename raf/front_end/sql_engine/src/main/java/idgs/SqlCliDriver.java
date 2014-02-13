/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import idgs.execution.ResultSet;

public class SqlCliDriver {

  private static Log LOG = LogFactory.getLog(SqlCliDriver.class);
  
  public static void main(String[] args) {
    String sql = null;
    if (args.length > 0) {
      for (String s : args) {
        sql = (sql == null) ? s : sql + " " + s;
      }
    } else {
      System.out.println("no sql found");
      System.exit(1);
    }
    
    try {
      System.out.println(sql);
      ResultSet resultSet = IdgsCliDriver.run(sql);
      if (resultSet == null) {
        System.exit(2);
      } else {
        System.out.println(resultSet.toString());
      }
       
      System.exit(0);
    } catch (Exception e) {
      LOG.error(e.getMessage(), e);
      e.printStackTrace();
      System.exit(1);
    }
  }
  
}
