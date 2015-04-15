/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.jdbc;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.net.URL;
import java.sql.SQLException;

import idgs.IdgsConfVars;
import idgs.IdgsDriver;
import idgs.client.TcpClientInterface;
import idgs.client.TcpClientPool;
import idgs.client.pb.PbClientConfig.ClientConfig;
import idgs.exception.IdgsException;
import idgs.metadata.LocalStoreLoader;
import idgs.metadata.StoreLoader;
import idgs.metadata.StoreMetadata;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.hive.conf.HiveConf;
import org.apache.hadoop.hive.conf.HiveConf.ConfVars;
import org.apache.hadoop.hive.metastore.api.MetaException;
import org.apache.hadoop.hive.ql.Driver;
import org.apache.hadoop.hive.ql.processors.CommandProcessor;
import org.apache.hadoop.hive.ql.processors.CommandProcessorFactory;
import org.apache.hadoop.hive.ql.processors.CommandProcessorResponse;
import org.apache.hadoop.hive.ql.session.SessionState;
import org.apache.hadoop.hive.service.HiveServer.HiveServerHandler;
import org.apache.hadoop.hive.service.HiveServerException;
import org.apache.thrift.TException;

public class IdgsClient extends HiveServerHandler {

  private static Log LOG = LogFactory.getLog(IdgsClient.class);
  
  private TcpClientPool clientPool;
  
  private Field isHiveQueryField;
  
  private Field driverField;
  
  private Field responseField;

  public IdgsClient(String clientCfgFile, String storeCfgFile) throws MetaException, SQLException {
    initIdgsClient();
    initConnection(clientCfgFile);
    initMetadata(storeCfgFile);
  }
  
  public IdgsClient(ClientConfig clientCfg, String storeCfgFile) throws MetaException, SQLException {
    initIdgsClient();
    initConnection(clientCfg);
    initMetadata(storeCfgFile);
  }
  
  @Override
  public void clean() {
    LOG.info("Start close connection.");
    super.clean();
    Driver driver = getDriver();
    if (driver != null) {
      driver.close();
      driver.destroy();
    }
    
    if (clientPool != null) {
      clientPool.close();
      clientPool = null;
    }
  }

  @Override
  public void execute(String cmd) throws HiveServerException, TException {
    String trimmedCommand = cmd.trim();
    String[] tokens = trimmedCommand.split("\\s");
    
    CommandProcessor proc = CommandProcessorFactory.get(tokens[0]);
    if (proc == null) {
      return;
    }

    int ret = 0;
    CommandProcessorResponse response = null;
    
    Driver driver = getDriver();
    if (driver != null) {
      driver.close();
      driver = null;
    }
    
    SessionState session = SessionState.get();
    HiveConf conf = session.getConf();
    
    boolean isHiveQuery = (proc instanceof Driver);
    setIsHiveQuery(isHiveQuery);
    
    try {
      if (isHiveQuery) {
        driver = (IdgsConfVars.getVar(conf, IdgsConfVars.EXEC_MODE) == "idgs") ? new IdgsDriver(conf) : (Driver) proc;
        driver.setTryCount(Integer.MAX_VALUE);
        response = driver.run(cmd);
      } else {
        driver = null;
        
        Method setupSessionIOMethod = HiveServerHandler.class.getDeclaredMethod("setupSessionIO", SessionState.class);
        setupSessionIOMethod.setAccessible(true);
        setupSessionIOMethod.invoke(this, session);
        String command = trimmedCommand.substring(tokens[0].length()).trim();
        response = proc.run(command);
      }

      setDriver(driver);
      setResponse(response);
      
      ret = response.getResponseCode();
    } catch (Exception e) {
      HiveServerException ex = new HiveServerException();
      ex.setMessage("Error running query: " + e.toString());
      ex.setErrorCode(ret == 0? -10000: ret);
      throw ex;
    }

    if (ret != 0) {
      String errorMessage = response.getErrorMessage();
      String SQLState = response.getSQLState();
      String exMessage = "Query returned non-zero code: " + ret + ", cause: " + errorMessage;
      throw new HiveServerException(exMessage, ret, SQLState);
    }
  }
  
  private void initIdgsClient() throws SQLException {
    SessionState session = SessionState.get();
    HiveConf conf = session.getConf();
    HiveConf.setBoolVar(conf, ConfVars.HIVEMAPSIDEAGGREGATE, false);
    
    try {
      isHiveQueryField = HiveServerHandler.class.getDeclaredField("isHiveQuery");
      isHiveQueryField.setAccessible(true);
      
      driverField = HiveServerHandler.class.getDeclaredField("driver");
      driverField.setAccessible(true);
      
      responseField = HiveServerHandler.class.getDeclaredField("response");
      responseField.setAccessible(true);
    } catch (Exception e) {
      throw new SQLException(e.getMessage());
    }
  }
  
  private void initConnection(ClientConfig clientCfg) throws SQLException {
    LOG.info("Start to initialize connection.");
    clientPool = TcpClientPool.getInstance();
    try {
      clientPool.loadClientConfig(clientCfg);
      if (clientPool.size() == 0) {
        LOG.error("No available server found.");
        throw new SQLException("No available server found.");
      }
    } catch (IOException e) {
      throw new SQLException(e);
    }
    
    TcpClientInterface client = clientPool.getClient();
    if (client == null) {
      LOG.error("Connection refused.");
      throw new SQLException("Connection refused.");
    }
    client.close();
  }
  
  private void initConnection(String clientCfgFile) throws SQLException {
    LOG.info("Start to initialize connection.");
    if (clientCfgFile == null) {
      SessionState session = SessionState.get();
      HiveConf conf = session.getConf();
      clientCfgFile = IdgsConfVars.getVar(conf, IdgsConfVars.CLIENT_CONFIG_FILE);
    }
    
    LOG.info("Use client config file " + clientCfgFile);
    clientPool = TcpClientPool.getInstance();
    try {
      clientPool.loadClientConfig(clientCfgFile);
      if (clientPool.size() == 0) {
        LOG.error("No available server found.");
        throw new SQLException("No available server found");
      }
    } catch (IOException e) {
      throw new SQLException(e);
    }
    
    TcpClientInterface client = clientPool.getClient();
    if (client == null) {
      LOG.error("Connection refused.");
      throw new SQLException("Connection refused.");
    }
    client.close();
  }
  
  private void initMetadata(String storeCfgFile) throws SQLException {
    SessionState session = SessionState.get();
    HiveConf conf = session.getConf();
    if (storeCfgFile == null) {
      storeCfgFile = IdgsConfVars.getVar(conf, IdgsConfVars.STORE_CONFIG_FILE);
    }

    StoreLoader loader = new LocalStoreLoader(storeCfgFile);
    try {
      IdgsConfVars.setVar(conf, IdgsConfVars.STORE_CONFIG_FILE, storeCfgFile);
    
      StoreMetadata.getInstance().initMetadata(conf, loader.loadDataStoreConf());
    } catch (IdgsException e) {
      throw new SQLException(e);
    }
  }
  
  private void setIsHiveQuery(boolean isHiveQuery) {
    try {
      isHiveQueryField.set(this, isHiveQuery);
    } catch (Exception e) {
      e.printStackTrace();
    }
  }
  
  private void setDriver(Driver driver) {
    try {
      driverField.set(this, driver);
    } catch (Exception e) {
      e.printStackTrace();
    }
  }
  
  private Driver getDriver() {
    try {
      return (Driver) driverField.get(this);
    } catch (Exception e) {
      e.printStackTrace();
    }
    
    return null;
  }
  
  private void setResponse(CommandProcessorResponse response) {
    try {
      responseField.set(this, response);
    } catch (Exception e) {
    }
  }

}
