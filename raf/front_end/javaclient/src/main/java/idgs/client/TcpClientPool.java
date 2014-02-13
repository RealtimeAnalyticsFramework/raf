
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.client;

import idgs.client.pb.PbClientConfig.ClientConfig;
import idgs.client.pb.PbClientConfig.Endpoint;
import idgs.client.util.JsonUtil;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicBoolean;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.googlecode.protobuf.format.JsonFormat.ParseException;

public class TcpClientPool {

  private static Log log = LogFactory.getLog(TcpClientPool.class);

  private ConcurrentLinkedQueue<TcpClient> pool = new ConcurrentLinkedQueue<TcpClient>();

  private List<Endpoint> availableServers = new ArrayList<Endpoint>();

  private static TcpClientPool instance = new TcpClientPool();

  private AtomicBoolean initialized = new AtomicBoolean(false);
  
  private AtomicBoolean running = new AtomicBoolean(true);
  
  private ClientConfig cfg = null;
  
  private int loadFactor = 2;

  private TcpClientPool() {

  }

  public static TcpClientPool getInstance() {
    return instance;
  }
  

  public void loadClientConfig(final ClientConfig cfg) throws IOException {
    if(cfg == null) {
      throw new NullPointerException("client config can not be null");
    }
    this.cfg = cfg;
    loadClientConfig();
  }
  
  public void loadClientConfig(String file) throws IOException {
    ClientConfig.Builder builder = ClientConfig.newBuilder();
    try {
      JsonUtil.jsonFileToPb(builder, file);
    } catch (ParseException e) {
      log.error(e.getMessage(), e);
      throw e;
    }
    cfg = builder.build();
    loadClientConfig();
  }
  
  private void loadClientConfig() {
    log.debug(cfg.toString());
    init();
    Timer timer = new Timer();
    timer.schedule(new TimerTask() {
      @Override
      public void run() {
        int poolSize = -1;
        if(running.get() && (poolSize = size()) <= 0) {
          log.warn("no available client to use, will auto create 2 multiples clients of config pool size");
          newConnection(cfg.getPoolSize() * loadFactor);
          poolSize = size();
          log.info("new pool size: " + poolSize);
        } 
      }
    }, 5000, 3000);
  }
  
  public void setLoadFactor(int loadFactor) {
    this.loadFactor = loadFactor;
  }
  
  public int getLoadFactor() {
    return loadFactor;
  }
  
  public ClientConfig getClientConfig() {
    return cfg;
  }
  
  public void init() {
    if(initialized.compareAndSet(false, true)) {
      newConnection(cfg.getPoolSize());
    }
  }
  
  public void setClientConfig(ClientConfig cfg) {
    this.cfg = cfg;
  }
  
  private void newConnection(int poolSize) {
    extractAvailableServers();
    final int availableServerSize = availableServers.size();
    if(availableServerSize <= 0) {
      return;
    }
    // try to connect with available servers, if pool size 50, available server size 2, then 25 connections will be created with each server
    for (int i = 0; i < poolSize; ++i) {
      int index = i % availableServerSize; // connect to which server when create the i connection
      Endpoint server = availableServers.get(index);
      TcpClient client = null;
      try {
        int port = Integer.parseInt(server.getPort());
        client = new TcpClient(server.getAddress(), port);
        client.connect(cfg.getEnableConnectTimeout(), cfg.getConnectTimeout(), cfg.getRetryTimes()); // @todo will replaced by client config file
      } catch (IOException e) {
        log.warn(e.getMessage(), e);
        continue; // ignore failed connection
      }
      if(client != null && client.isConnected()) {
        client.setId(i);
        pool.add(client);
      }
    }
  }

  private void extractAvailableServers() {
    for (int i = 0; i < cfg.getServerAddressesCount(); ++i) { // calculate available servers
      Endpoint server = cfg.getServerAddresses(i);
      TcpClient client = null;
      try {
        int port = Integer.parseInt(server.getPort());
        client = new TcpClient(server.getAddress(), port);
        boolean connected = client.connect(cfg.getEnableConnectTimeout(), cfg.getConnectTimeout(), cfg.getRetryTimes()); // @todo will replaced by client config file
        if(connected) {
          availableServers.add(server);
        }
      } catch (IOException e) {
        log.error(e.getMessage(), e);
      } finally {
        if(client != null) {
          client.close();
        }
      }
    } // end for 
  }
  
  /**
   * poll a client from pool
   * 
   * @return
   */
  public TcpClientInterface getClient() {
    TcpClient client = pool.poll();
    if(client == null) {
      return null;
    }
    return new TcpClientInterface(client);
  }
  
  /**
   * poll a client from pool
   * 
   * @return
   */
  public TcpClientInterface getClient(long timeout, int retryTimes) {
    if(timeout <= 0) {
      throw new IllegalArgumentException("argument 'timeout' should be a positive long value");
    }
    if(retryTimes <= 0) {
      throw new IllegalArgumentException("argument 'retryTimes' should be a positive integer value");
    }
    TcpClient client = null;
    int count = 0;
    do {
      client = pool.poll();
      if(client != null) {
        break;
      } else {
        try {
          Thread.sleep(timeout);
        } catch (InterruptedException e) {
          // do nothing
        }
      }
    } while(++count < retryTimes);
    if(client == null) {
      return null;
    }
    return new TcpClientInterface(client);
  }

  /**
   * return pool size
   * 
   * @return
   */
  public int size() {
    return pool.size();
  }

  /**
   * release pool
   */
  public void close() {
    if (!initialized.get()) {
      return;
    }
    running.compareAndSet(true, false);
    for(Iterator<TcpClient> it = pool.iterator(); it.hasNext() ; ) {
      TcpClient client = it.next();
      client.close();
      it.remove();
    }
  }

  /**
   * put back client into pool
   * 
   * @param client
   * @return
   */
  boolean putBack(TcpClient client) {
    if (!initialized.get()) {
      return false;
    }

    if (!pool.add(client)) {
      return false;
    }
    return true;
  }

  public String toString() {
    StringBuilder buf = new StringBuilder();
    final boolean emptyFlag = pool.isEmpty();
    if(cfg != null) {
      buf.append(cfg.toString());
    }
    if (emptyFlag) {
      buf.append("client pool is empty!");
    } else {
      buf.append("\n===========client pool queue=============\n");
      Iterator<TcpClient> it = pool.iterator();
      while (it.hasNext()) {
        buf.append("client id: " + it.next().getId()).append("\n");
      }
      buf.append("=========//client pool queue=============\n");
    }
    return buf.toString();
  }
}
