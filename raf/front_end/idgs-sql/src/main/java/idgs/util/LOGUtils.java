/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
package idgs.util;

import idgs.exception.IdgsException;

import java.io.File;
import java.net.URL;

import org.apache.log4j.LogManager;
import org.apache.log4j.PropertyConfigurator;
import org.apache.hadoop.hive.conf.HiveConf;

public class LOGUtils {

  private static final String IDGS_L4J = "idgs-log4j.properties";

  public static String initLog4j() throws IdgsException {
    HiveConf conf = new HiveConf();
    String log4jFileName = HiveConf.getVar(conf, HiveConf.ConfVars.HIVE_LOG4J_FILE);
    
    if (log4jFileName.equals("")) {
      return initLog4jDefault("");
    } else {
      File log4jFile = new File(log4jFileName);
      if (!log4jFile.exists()) {
        return initLog4jDefault("Cannot find conf file: " + log4jFile + "\n");
      } else {
        LogManager.resetConfiguration();
        PropertyConfigurator.configure(log4jFileName);
        
        return "Logging initialized using configuration in " + log4jFile;
      }
    }
  }
  
  private static String initLog4jDefault(String logMessage) throws IdgsException {
    URL log4jURL = LOGUtils.class.getClassLoader().getResource(IDGS_L4J);
    if (log4jURL == null) {
      throw new IdgsException(logMessage + "Unable to initialize logging using " + IDGS_L4J + ", not found on CLASSPATH!");
    } else {
      LogManager.resetConfiguration();
      PropertyConfigurator.configure(log4jURL);
      
      return logMessage + "Logging initialized using default configuration in " + log4jURL;
    }
  }

}
