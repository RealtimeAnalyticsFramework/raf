/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.jdbc;

import idgs.IdgsConfVars;
import idgs.client.pb.PbClientConfig.ClientConfig;
import idgs.client.pb.PbClientConfig.Endpoint.Builder;

import java.sql.SQLException;
import java.util.Properties;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.hive.conf.HiveConf;
import org.apache.hadoop.hive.conf.HiveConf.ConfVars;
import org.apache.hive.jdbc.HiveConnection;
import org.apache.hive.jdbc.Utils;
import org.apache.hive.jdbc.Utils.JdbcConnectionParams;
import org.apache.hive.service.cli.thrift.TCLIService;
import org.apache.hive.service.cli.thrift.TCLIService.Iface;
import org.apache.hive.service.cli.thrift.TCloseSessionReq;
import org.apache.hive.service.cli.thrift.TSessionHandle;

public class IdgsConnection extends HiveConnection {

  private static Log LOG = LogFactory.getLog(IdgsConnection.class);
  
  public IdgsConnection(String url, Properties info) throws SQLException {
    super(Utils.URL_PREFIX, info);
    
    JdbcConnectionParams connParams = null;
    try {
      connParams = (JdbcConnectionParams) ReflectUtils.getFieldValue(HiveConnection.class, "connParams", this);
    } catch (Exception e) {
      e.printStackTrace();
    }
    
    TCLIService.Iface client = null;
    
    HiveConf conf = new HiveConf();
    HiveConf.setBoolVar(conf, ConfVars.HIVEMAPSIDEAGGREGATE, false);
    
    String storeCfgFilePath = info.getProperty(IdgsJdbcDriver.STORE_CONFIG_FILE_PATH);
    if (storeCfgFilePath == null) {
      storeCfgFilePath = IdgsConfVars.getVar(conf, IdgsConfVars.STORE_CONFIG_FILE);
    } else {
      IdgsConfVars.setVar(conf, IdgsConfVars.STORE_CONFIG_FILE, storeCfgFilePath);
    }
    
    if (connParams.getHost() == null) {
      String clientCfgFilePath = info.getProperty(IdgsJdbcDriver.CLIENT_CONFIG_FILE_PATH);
      if (clientCfgFilePath == null) {
        clientCfgFilePath = IdgsConfVars.getVar(conf, IdgsConfVars.CLIENT_CONFIG_FILE);
      } else {
        IdgsConfVars.setVar(conf, IdgsConfVars.CLIENT_CONFIG_FILE, clientCfgFilePath);
      }
    } else {
      String host = connParams.getHost();
      int port = connParams.getPort();
      LOG.info("Use " + host + ":" + port + " to connect.");
      ClientConfig.Builder cfgBuilder = ClientConfig.newBuilder();
      Builder addrBuilder = cfgBuilder.addServerAddressesBuilder();
      addrBuilder.setHost(host);
      addrBuilder.setPort(String.valueOf(port));
      cfgBuilder.setPoolSize(50);
    }

    IdgsClient embeddedClient = new IdgsClient();
    embeddedClient.init(conf);
    embeddedClient.initConnection(conf);
    embeddedClient.initMetaStore(conf);
    client = embeddedClient;
    
    try {
      TCLIService.Iface hiveClient = (Iface) ReflectUtils.getFieldValue(HiveConnection.class, "client", this);
      TSessionHandle sessHandle = (TSessionHandle) ReflectUtils.getFieldValue(HiveConnection.class, "sessHandle", this);
      if (hiveClient != null) {
        hiveClient.CloseSession(new TCloseSessionReq(sessHandle));
        ReflectUtils.setFieldValue(HiveConnection.class, "client", this, null);
      }
      
      ReflectUtils.setFieldValue(HiveConnection.class, "client", this, client);
      ReflectUtils.methodInvoke(HiveConnection.class, "openSession", null, this, null);
    } catch (Exception e) {
      e.printStackTrace();
    }
  }
  
  @Override
  public void commit() throws SQLException {
    LOG.warn("not supported commit");
  }
  
}
