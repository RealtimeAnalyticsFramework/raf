/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.jdbc;

import idgs.client.pb.PbClientConfig.ClientConfig;
import idgs.client.pb.PbClientConfig.Endpoint.Builder;

import java.lang.reflect.Field;
import java.sql.SQLException;
import java.util.Properties;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.hive.conf.HiveConf;
import org.apache.hadoop.hive.jdbc.HiveConnection;
import org.apache.hadoop.hive.service.HiveInterface;

public class IdgsConnection extends HiveConnection {

  private static Log LOG = LogFactory.getLog(IdgsConnection.class);
  
  private static final String URI_PREFIX = "jdbc:idgs://";
  
  private Field clientField;
  
  public IdgsConnection(HiveConf hiveconf) throws SQLException {
    super(hiveconf);
    try {
      clientField = HiveConnection.class.getDeclaredField("client");
      clientField.setAccessible(true);
    } catch (Exception e) {
      e.printStackTrace();
    }
  }
  
  public IdgsConnection(String url, Properties info) throws SQLException {
    this(new HiveConf());
    LOG.info("JDBC URL: " + url + ", Properties: " + info.toString());

    HiveInterface client = null;
    setClient(client);
    
    if (url != null && !url.isEmpty()) {
      // invalid url
      if (!url.startsWith(URI_PREFIX)) {
        throw new SQLException("Invalid URL: " + url, "08S01");
      }

      // remove prefix
      url = url.substring(URI_PREFIX.length());
    }

    // If url is not specified, use default.
    String storeCfgPath = info.getProperty(IdgsJdbcDriver.STORE_CONFIG_FILE_PATH);
    if (url == null || url.isEmpty()) {
      String cfgFile = info.getProperty(IdgsJdbcDriver.CLIENT_CONFIG_FILE_PATH);
      try {
        LOG.info("client config file: " + cfgFile + ", store config: " + storeCfgPath);
        client = new IdgsClient(cfgFile, storeCfgPath);
      } catch (Exception ex) {
        throw new SQLException(ex);
      }
    } else {
      String[] parts = url.split("/");
      String[] hostport = parts[0].split(":");
      Integer port = 10000;
      String host = hostport[0];
      try {
        port = Integer.parseInt(hostport[1]);
      } catch (Exception e) {
      }
      
      LOG.info("Use " + host + ":" + port + " to connect.");
      ClientConfig.Builder cfgBuilder = ClientConfig.newBuilder();
      Builder addrBuilder = cfgBuilder.addServerAddressesBuilder();
      addrBuilder.setAddress(host);
      addrBuilder.setPort(port.toString());
      cfgBuilder.setPoolSize(50);

      try {
        client = new IdgsClient(cfgBuilder.build(), storeCfgPath);
      } catch (Exception ex) {
        throw new SQLException(ex);
      }
    }
    
    setClient(client);
  }
  
  private void setClient(HiveInterface client) {
    try {
      clientField.set(this, client);
    } catch (Exception e) {
      e.printStackTrace();
    }
  }
  
}
