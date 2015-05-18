/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.jdbc;

import idgs.exception.IdgsException;
import idgs.util.LOGUtils;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.Properties;
import java.util.regex.Pattern;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hive.jdbc.HiveDriver;
import org.apache.hive.jdbc.Utils;

public class IdgsJdbcDriver extends HiveDriver {

  private static Log LOG = LogFactory.getLog(IdgsJdbcDriver.class);
  
  public static final String STORE_CONFIG_FILE_PATH = "idgs.store.config"; 
  
  public static final String CLIENT_CONFIG_FILE_PATH = "idgs.client.config";
  
  private static final String URL_PREFIX = "jdbc:idgs://";
  
  static {
    // set hive log dir
    String dir = System.getenv("IDGS_LOG_DIR");
    if (dir != null) {
      java.util.Properties sysp = new java.util.Properties(System.getProperties());
      sysp.setProperty("idgs.log.dir", dir);
      sysp.setProperty("hive.log.dir", dir);
      sysp.setProperty("hive.querylog.location", dir);
      sysp.setProperty("hive.exec.scratchdir", dir);
      System.setProperties(sysp);
    }
  }

  static {
    try {
      DriverManager.registerDriver(new IdgsJdbcDriver());
      LOGUtils.initLog4j();
    } catch (SQLException e) {
      e.printStackTrace();
    } catch (IdgsException e) {
      e.printStackTrace();
    }
  }
  
  public IdgsJdbcDriver() {
  }
  
  @Override
  public Connection connect(String url, Properties info) throws SQLException {
    if (url == null) {
      LOG.error("The url cannot be null");
      return null;
    }
    
    url = url.trim();
    
    String hiveUrl = null;
    if (url.isEmpty()) {
      hiveUrl = Utils.URL_PREFIX;
    } else {
      if (!acceptsURL(url)) {
        LOG.error("The url " + url + " is must be start with " + URL_PREFIX);
        return null;
      }
      
      hiveUrl = url.replaceFirst(URL_PREFIX, Utils.URL_PREFIX);
    }
    
    LOG.info("JDBC URL: " + url + ", Properties: " + info.toString());
    
    return new IdgsConnection(hiveUrl, info);
  }

  @Override
  public boolean acceptsURL(String url) throws SQLException {
    return Pattern.matches(URL_PREFIX + ".*", url);
  }
  
}
