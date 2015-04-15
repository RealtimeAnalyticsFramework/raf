/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.jdbc;

import idgs.exception.IdgsException;
import idgs.util.LOGUtils;

import java.io.IOException;
import java.net.URL;
import java.sql.Connection;
import java.sql.Driver;
import java.sql.DriverManager;
import java.sql.DriverPropertyInfo;
import java.sql.SQLException;
import java.sql.SQLFeatureNotSupportedException;
import java.util.Properties;
import java.util.jar.Attributes;
import java.util.jar.Manifest;
import java.util.logging.Logger;
import java.util.regex.Pattern;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class IdgsJdbcDriver implements Driver {

  private static Log LOG = LogFactory.getLog(IdgsJdbcDriver.class);
  
  public static final String STORE_CONFIG_FILE_PATH = "idgs.store.config"; 
  
  public static final String CLIENT_CONFIG_FILE_PATH = "idgs.client.config";
  
  private static final String URL_PREFIX = "jdbc:idgs://";
  
  private static final String DBNAME_PROPERTY_KEY = "DBNAME";

  private static final String HOST_PROPERTY_KEY = "HOST";

  private static final String PORT_PROPERTY_KEY = "PORT";
  
  private Integer majorVersion = null;
  
  private Integer minorVersion = null;

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
    if (!url.isEmpty() && !acceptsURL(url)) {
      LOG.error("The url " + url + " is must be start with " + URL_PREFIX);
      return null;
    }
    
    return new IdgsConnection(url, info);
  }

  @Override
  public boolean acceptsURL(String url) throws SQLException {
    return Pattern.matches(URL_PREFIX + ".*", url);
  }

  @Override
  public DriverPropertyInfo[] getPropertyInfo(String url, Properties info) throws SQLException {
    if (info == null) {
      info = new Properties();
    }

    if ((url != null) && url.startsWith(URL_PREFIX)) {
      String[] parts = url.split("/");
      String[] hostport = parts[0].split(":");
      String host = hostport[0];
      Integer port = 10000;
      try {
        port = Integer.parseInt(hostport[1]);
      } catch (Exception e) {
      }
      info.setProperty(HOST_PROPERTY_KEY, host);
      info.setProperty(PORT_PROPERTY_KEY, port.toString());
    }
    
    DriverPropertyInfo[] dpi = new DriverPropertyInfo[3];

    dpi[0] = new DriverPropertyInfo(HOST_PROPERTY_KEY, info.getProperty(HOST_PROPERTY_KEY, ""));
    dpi[0].description = "Hostname of Server";
    dpi[0].required = false;

    dpi[1] = new DriverPropertyInfo(PORT_PROPERTY_KEY, info.getProperty(PORT_PROPERTY_KEY, ""));
    dpi[1].description = "Port of Server";
    dpi[1].required = false;

    dpi[2] = new DriverPropertyInfo(DBNAME_PROPERTY_KEY, info.getProperty(DBNAME_PROPERTY_KEY, "default"));
    dpi[2].description = "Database name";
    dpi[2].required = false;

    return dpi;
  }

  @Override
  public int getMajorVersion() {
    if (majorVersion == null) {
      try {
        loadManifestVersion();
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
    return majorVersion;
  }

  @Override
  public int getMinorVersion() {
    if (minorVersion == null) {
      try {
        loadManifestVersion();
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
    return minorVersion;
  }
  
  @Override
  public boolean jdbcCompliant() {
    return false;
  }

  //@Override
  public Logger getParentLogger() throws SQLFeatureNotSupportedException {
    throw new SQLFeatureNotSupportedException("Not support getParentLogger");
  }
  
  private synchronized void loadManifestVersion() throws IOException {
    if (majorVersion != null && minorVersion != null) {
      return;
    }
    
    majorVersion = new Integer(-1);
    minorVersion = new Integer(-1);
    
    String classContainer = IdgsJdbcDriver.class.getProtectionDomain().getCodeSource().getLocation().toString();
    URL manifestUrl = new URL("jar:" + classContainer + "!/META-INF/MANIFEST.MF");
    Manifest manifest = new Manifest(manifestUrl.openStream());
    Attributes attrs = manifest.getMainAttributes();
    
    String fullVersion = attrs.getValue(Attributes.Name.IMPLEMENTATION_VERSION);
    String[] tokens = fullVersion.split("\\.");
    
    if (tokens != null && tokens.length > 0 && tokens[0] != null) {
      majorVersion = Integer.parseInt(tokens[0]);
    }
    
    if (tokens != null && tokens.length > 1 && tokens[1] != null) {
      minorVersion = Integer.parseInt(tokens[1]);
    }
  }
  
}
