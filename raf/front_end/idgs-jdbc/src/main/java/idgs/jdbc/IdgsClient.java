/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.jdbc;

import java.io.IOException;
import java.sql.SQLException;

import idgs.IdgsConfVars;
import idgs.client.TcpClientInterface;
import idgs.client.TcpClientPool;
import idgs.client.pb.PbClientConfig.ClientConfig;
import idgs.jdbc.service.IdgsCLIService;
import idgs.metadata.StoreLoader;
import idgs.metadata.StoreMetadata;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.hive.conf.HiveConf;
import org.apache.hive.service.cli.thrift.TCloseSessionReq;
import org.apache.hive.service.cli.thrift.TCloseSessionResp;
import org.apache.hive.service.cli.thrift.ThriftBinaryCLIService;
import org.apache.thrift.TException;

public class IdgsClient extends ThriftBinaryCLIService {

  private static Log LOG = LogFactory.getLog(IdgsClient.class);
  
  private TcpClientPool clientPool;
  
  public IdgsClient() {
    super(new IdgsCLIService(null));
    isEmbedded = true;
    HiveConf.setLoadHiveServer2Config(true);
  }

  @Override
  public synchronized void init(HiveConf hiveConf) {
    cliService.init(hiveConf);
    cliService.start();
    
    super.init(hiveConf);
  }
  
  @Override
  public TCloseSessionResp CloseSession(TCloseSessionReq req) throws TException {
    LOG.info("Start close connection.");
    if (clientPool != null) {
      clientPool.close();
      clientPool = null;
    }
    
    return super.CloseSession(req);
  }
  
  public void initConnection(HiveConf conf) throws SQLException {
    LOG.info("Start to initialize connection.");
    clientPool = TcpClientPool.getInstance();
    try {
      do {
        Object objVar = IdgsConfVars.getObjectVar(conf, IdgsConfVars.CLIENT_PB_CONFIG);
        if (objVar != null && objVar instanceof ClientConfig) {
          ClientConfig clientCfg = (ClientConfig) objVar;
          clientPool.loadClientConfig(clientCfg);
          break;
        }
        
        String clientCfgFilePath = IdgsConfVars.getVar(conf, IdgsConfVars.CLIENT_CONFIG_FILE);
        if (clientCfgFilePath != null) {
          clientPool.loadClientConfig(clientCfgFilePath);
          break;
        }
        
        throw new SQLException("cannot get connection from server");
      } while(false);
    } catch (IOException e) {
      throw new SQLException(e);
    }

    if (clientPool.size() == 0) {
      LOG.error("No available server found.");
      throw new SQLException("No available server found.");
    }
    
    TcpClientInterface client = clientPool.getClient();
    if (client == null) {
      LOG.error("Connection refused.");
      throw new SQLException("Connection refused.");
    }
    client.close();
  }
  
  public void initMetaStore(HiveConf conf) throws SQLException {
    String storeLoaderClass = IdgsConfVars.getVar(conf, IdgsConfVars.STORE_LOADER_CLASS);
    
    try {
      Class<?> _class = Class.forName(storeLoaderClass);
      Object inst = _class.newInstance();
      if (inst instanceof StoreLoader) {
        StoreLoader loader = (StoreLoader) inst;
        loader.init(conf);
        StoreMetadata.getInstance().initMetadata(conf, loader.loadDataStoreConf());
      } else {
        throw new SQLException("class " + storeLoaderClass + " is not StoreLoader");
      }
    } catch (Exception e) {
      throw new SQLException(e);
    }
  }
  
}
