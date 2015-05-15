/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.metadata;

import org.apache.hadoop.hive.conf.HiveConf;
import org.apache.hadoop.hive.ql.session.SessionState;

import idgs.IdgsConfVars;
import idgs.store.pb.PbStoreConfig.DataStoreConfig;
import idgs.store.pb.PbStoreConfig.PartitionType;
import idgs.store.pb.PbStoreConfig.StoreConfig;
import idgs.store.pb.PbStoreConfig.StoreType;
import idgs.util.LOGUtils;
import junit.framework.TestCase;

public class StoreMetadataUT extends TestCase {

  public void testInitMetadata() {
    try {
      LOGUtils.initLog4j();
      
      String cfgFilePath = null;
      String home = System.getenv("IDGS_HOME");
      if (home == null) {
        cfgFilePath = this.getClass().getResource("/data_store.conf").getPath();
      } else {
        cfgFilePath = home + "/conf/data_store.conf";
      }
      
      LocalStoreLoader loader = new LocalStoreLoader();
      loader.setCfgFilePath(cfgFilePath);
      
      DataStoreConfig conf = loader.loadDataStoreConf();
      HiveConf hiveConf = new HiveConf(SessionState.class);
      
      IdgsConfVars.setVar(hiveConf, IdgsConfVars.STORE_CONFIG_FILE, cfgFilePath);
      
      StoreMetadata.getInstance().initMetadata(hiveConf, conf);
      
      StoreConfig storeCfg = StoreMetadata.getInstance().getStoreConfig("tpch", "Customer");
      
      assertEquals(storeCfg.getName(), "Customer");
      assertEquals(storeCfg.getStoreType(), StoreType.ORDERED);
      assertEquals(storeCfg.getPartitionType(), PartitionType.PARTITION_TABLE);
      assertEquals(storeCfg.getKeyType(), "idgs.sample.tpch.pb.CustomerKey");
      assertEquals(storeCfg.getValueType(), "idgs.sample.tpch.pb.Customer");
      
      storeCfg = StoreMetadata.getInstance().getStoreConfig("tpch", "Order");
      assertTrue(storeCfg == null);
    } catch (Exception e) {
      e.printStackTrace();
      fail(e.getMessage());
    }
  }

}
