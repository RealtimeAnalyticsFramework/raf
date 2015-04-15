
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.client;

import idgs.store.pb.PbStoreService.InsertResponse;
import idgs.store.pb.PbStoreService.StoreResultCode;

import java.io.IOException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class TcpClientPoolIT extends BaseClientTest {
	
	private static Log log = LogFactory.getLog(TcpClientPoolIT.class);
	
	public synchronized void testTcpClientPool() {
	  log.info("test create singleton pool instance");
	  final TcpClientPool pool = TcpClientPool.getInstance();
	  final TcpClientPool pool1 = TcpClientPool.getInstance();
	  assertEquals(pool, pool1);
	  
	  log.info("test load client config to create all clients");
	  // String cfgFile = getClass().getResource("/test-client.conf").getPath();
	  String cfgFile = "conf/client.conf";
	  try {
	    pool.loadClientConfig(cfgFile);
	  } catch (IOException e) {
	    log.error(e.getMessage(), e);
	  }
	  int actualPoolSize = pool.size();
	  int expectPoolSize = pool.getClientConfig().getPoolSize();
	  log.info("config pool size: " + expectPoolSize);
	  log.info("actual pool size: " + actualPoolSize);
	  assertTrue(expectPoolSize >= pool.size());
	  
	  log.info("test poll a client from the pool, and insert a customer into store, then push it back the pool");
    ClientActorMessage requestActorMsg = createActorMessage();
    final int poolSize = pool.size(); 
    if(poolSize <= 0) {
      log.warn("no avaiable client to be used");
      return;
    }
    final int loopCount = poolSize * 2; // let all client to be used at least once 
    for(int i = 0; i < loopCount; i++) {
      int prevPoolSize = pool.size();
      TcpClientInterface client = pool.getClient();
      int currPoolSize = pool.size();
      log.info("after poll out, current pool size: " + currPoolSize + ", previous pool size: " + prevPoolSize);
      assertEquals(currPoolSize, prevPoolSize - 1);
      if(client != null) {
        try {
          log.debug("-------------------request msg----------------------");
          log.debug(requestActorMsg.toString());
          ClientActorMessage responseActorMsg = client.sendRecv(requestActorMsg);
          InsertResponse.Builder builder = InsertResponse.newBuilder();
          responseActorMsg.parsePayload(builder);
          log.debug("-------------------response msg----------------------");
          log.debug(responseActorMsg.toString());
          InsertResponse response = builder.build();
          log.debug(response.toString());
          assertEquals(response.getResultCode(), StoreResultCode.SRC_SUCCESS);
        } catch(Exception e) {
          log.error(e.getMessage(), e);
        } finally {
          prevPoolSize = pool.size();
          client.close();
          currPoolSize = pool.size();
          log.info("after push back, current pool size: " + currPoolSize + ", previous pool size: " + prevPoolSize);
          assertEquals(currPoolSize, prevPoolSize + 1);
        }
      }
    }
    
    log.info("test client pool timer task whether auto create client connection when empty");
    
    Thread t1 = new Thread() {
    @Override
      public void run() {
        while(pool.size() != pool.getClientConfig().getPoolSize() * pool.getLoadFactor()) { // poll out client, never push back, test
          pool.getClient();
//          client.close(); 
          try {
            Thread.sleep(1000);
          } catch (InterruptedException e) {
            // do nothing
          }
        }
      }
    };
    
    t1.start();
    
    try {
      t1.join();
    } catch (InterruptedException e) {
      Thread.currentThread().interrupt();
    }
    
    assertEquals(pool.getClientConfig().getPoolSize() * pool.getLoadFactor(), pool.size());
    
    log.info("test close pool to release all clients ");
    assertTrue(pool.size() >= 0);
    pool.close();
    
    assertEquals(0, pool.size());
  }
}
