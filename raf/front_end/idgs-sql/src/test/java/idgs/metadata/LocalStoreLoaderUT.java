/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.metadata;

import idgs.store.pb.PbStoreConfig.DataStoreConfig;
import idgs.store.pb.PbStoreConfig.StoreSchema;
import junit.framework.TestCase;

public class LocalStoreLoaderUT extends TestCase {

  public void testLoadDataStoreConf() {
    String cfgFilePath = this.getClass().getResource("/data_store.conf").getPath();
    LocalStoreLoader loader = new LocalStoreLoader(cfgFilePath);
    try {
      DataStoreConfig storeconf = loader.loadDataStoreConf();
      assertEquals(1, storeconf.getSchemasCount());

      StoreSchema schema = storeconf.getSchemasList().get(0);
      assertEquals("tpch_test", schema.getSchemaName());

      assertEquals(3, schema.getStoreConfigCount());
    } catch (Exception e) {
      e.printStackTrace();
      fail(e.getMessage());
    }
  }

}
