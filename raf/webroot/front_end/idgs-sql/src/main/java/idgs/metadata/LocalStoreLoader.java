/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.metadata;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.hive.conf.HiveConf;

import idgs.IdgsConfVars;
import idgs.client.util.JsonUtil;
import idgs.exception.IdgsException;
import idgs.store.pb.PbStoreConfig.DataStoreConfig;

public class LocalStoreLoader implements StoreLoader {

  private static Log log = LogFactory.getLog(LocalStoreLoader.class);
  
  private static Map<String, DataStoreConfig> storeConfigMap = new HashMap<String, DataStoreConfig>();
  
  private String cfgFilePath;
  
  public LocalStoreLoader() {
  }
  
  public void setCfgFilePath(String cfgFilePath) {
    this.cfgFilePath = cfgFilePath;
  }
  
  @Override
  public void init(HiveConf conf) {
    cfgFilePath = IdgsConfVars.getVar(conf, IdgsConfVars.STORE_CONFIG_FILE);    
  }
  
  @Override
  public DataStoreConfig loadDataStoreConf() throws IdgsException {
    log.info("load store config: " + cfgFilePath);
    if (storeConfigMap.containsKey(cfgFilePath)) {
      return storeConfigMap.get(cfgFilePath);
    }
    
    File file = new File(cfgFilePath);
    do {
      if (file.exists()) {
        break;
      }
      
      String temppath;
      // IDGS_HOME env
      if ((temppath = System.getenv("IDGS_HOME")) != null) {
        temppath = temppath + "/" + cfgFilePath;
        file = new File(temppath);
        if (file.exists()) {
          break;
        }
      } 

      URL url = LocalStoreLoader.class.getClassLoader().getResource(cfgFilePath);
      if (url != null) {
        file = new File(url.getFile());
        break;
      }
      
      log.error("Data store config file " + cfgFilePath + " is not found.");
      throw new IdgsException("Data store config file " + cfgFilePath + " is not found.");
    } while (false);    
    
    DataStoreConfig.Builder builder = DataStoreConfig.newBuilder();
    try {
      JsonUtil.jsonFileToPb(builder, file.getAbsolutePath());
    } catch (IOException e) {
      log.error(e.getMessage());
      throw new IdgsException(e);
    }

    DataStoreConfig conf = builder.build();
    storeConfigMap.put(cfgFilePath, conf);
    return conf;
  }

}
