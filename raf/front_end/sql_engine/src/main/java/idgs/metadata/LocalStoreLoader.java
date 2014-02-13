/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.metadata;

import java.io.IOException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import idgs.client.util.JsonUtil;
import idgs.exception.IdgsException;
import idgs.store.pb.PbStoreConfig.DataStoreConfig;

public class LocalStoreLoader implements StoreLoader {

  private static Log log = LogFactory.getLog(LocalStoreLoader.class);
  
  private String cfgFilePath;
  
  public LocalStoreLoader(String cfgFilePath) {
    this.cfgFilePath = cfgFilePath;
  }
  
  @Override
  public DataStoreConfig loadDataStoreConf() throws IdgsException {
    log.info("load data_store.conf");
    DataStoreConfig.Builder builder = DataStoreConfig.newBuilder();
    try {
      JsonUtil.jsonFileToPb(builder, cfgFilePath);
    } catch (IOException e) {
      log.error(e.getMessage());
      throw new IdgsException(e);
    }

    DataStoreConfig conf = builder.build();
    return conf;
  }

}
