/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.metadata;

import idgs.IdgsConfVars;
import idgs.exception.IdgsException;
import idgs.store.pb.PbStoreConfig.DataStoreConfig;
import idgs.store.pb.PbStoreConfig.StoreConfig;
import idgs.store.pb.PbStoreConfig.StoreSchema;
import idgs.util.TypeUtils;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.hive.conf.HiveConf;
import org.apache.hadoop.hive.metastore.TableType;
import org.apache.hadoop.hive.metastore.api.FieldSchema;
import org.apache.hadoop.hive.ql.io.RCFileInputFormat;
import org.apache.hadoop.hive.ql.io.RCFileOutputFormat;
import org.apache.hadoop.hive.ql.metadata.Hive;
import org.apache.hadoop.hive.ql.metadata.HiveException;
import org.apache.hadoop.hive.ql.metadata.Table;

import com.google.protobuf.Descriptors.Descriptor;
import com.google.protobuf.Descriptors.FieldDescriptor;

import protubuf.MessageHelper;

public class StoreMetadata {

  private static Log log = LogFactory.getLog(StoreMetadata.class);
  
  private Map<String, StoreConfig> storeCache;
  
  private Map<String, String> storeNames;
  
  private static StoreMetadata instance;
  
  private StoreMetadata() {
    storeCache = new HashMap<String, StoreConfig>();
    storeNames = new HashMap<String, String>();
  }
  
  public static StoreMetadata getInstance() {
    if (instance == null) {
      instance = new StoreMetadata();
    }
    
    return instance;
  }
  
  public String getStoreName(String tableName) {
    return storeNames.get(tableName.toLowerCase());
  }
  
  public StoreConfig getStoreConfig(String storeName) {
    return storeCache.get(storeName);
  }
  
  public void initMetadata(HiveConf hiveConf, DataStoreConfig conf) throws IdgsException {
    initStore(hiveConf, conf);
    createLocalTable(hiveConf);
  }
  
  private void initStore(HiveConf hiveConf, DataStoreConfig conf) throws IdgsException {
    List<StoreSchema> schemas = conf.getSchemasList();
    for (int i = 0; i < schemas.size(); ++ i) {
      StoreSchema schema = schemas.get(i);
      
      String filePath = schema.getProtoFilename();
      File file = new File(filePath);
      if (!file.exists()) {
        if (filePath.charAt(0) == '/' || filePath.charAt(0) == '\\') {
          filePath = filePath.substring(1, filePath.length() - 1);
        }
        filePath = IdgsConfVars.getVar(hiveConf, IdgsConfVars.HOME_PATH) + filePath;
        file = new File(filePath);
        if (!file.exists()) {
          throw new IdgsException("file " + file.getAbsolutePath() + " is not found.");
        }
      }
      
      if (schema.hasProtoFilename()) {
        log.info("register file " + schema.getProtoFilename());
        MessageHelper.registerMessage(filePath);
      } else if (schema.hasProtoContent()) {
        String filename = schema.getSchemaName();
        file = new File(filename);
        FileOutputStream fos = null;
        try {
          fos = new FileOutputStream(file);
          fos.write(schema.getProtoContent().getBytes());
        } catch (IOException e) {
          throw new IdgsException(e);
        } finally {
          try {
            fos.close();
          } catch (IOException e) {
            throw new IdgsException(e);
          }
        }
        
        MessageHelper.registerMessage(filename);
        
        file.delete();
      }
      
      for (StoreConfig store : schema.getStoreConfigList()) {
        if (!MessageHelper.isMessageRegistered(store.getKeyType())) {
          log.error("store " + store.getName() + " key type " + store.getKeyType() + " is not registered on system.");
          System.exit(1);
        }
        
        if (!MessageHelper.isMessageRegistered(store.getValueType())) {
          log.error("store " + store.getName() + " value type " + store.getValueType() + " is not registered on system.");
          System.exit(1);
        }
        
        storeNames.put(store.getName().toLowerCase(), store.getName());
        storeCache.put(store.getName(), store);
      }
    }
  }
  
  private void createLocalTable(HiveConf conf) throws IdgsException {
      Hive db = null;

      Map<String, String> tableMap = new HashMap<String, String>();
      try {
        db = Hive.get(conf);
        for (String tbl : db.getAllTables()) {
          tableMap.put(tbl.toLowerCase(), tbl.toLowerCase());
        }
      } catch (HiveException e) {
        log.error("initialize store metadata error, caused by " + e.getMessage());
        throw new IdgsException(e);
      }
      
      for (StoreConfig cfg : storeCache.values()) {
        String tableName = cfg.getName().toLowerCase();
        
        if (tableMap.containsKey(tableName)) {
          log.info("table " + tableName + " exists drop it.");
          try {
            db.dropTable(tableName);
          } catch (Exception e) {
            log.error("error when drop table " + tableName + " caused by " + e.getMessage());
          }
        }
        
        try {
          Table tbl = db.newTable(tableName);
        
          List<FieldSchema> fields = new ArrayList<FieldSchema>();
          
          Descriptor keyDescriptor = MessageHelper.getMessageDescriptor(cfg.getKeyType());
          for (FieldDescriptor field : keyDescriptor.getFields()) {
            fields.add(new FieldSchema(field.getName(), TypeUtils.javaTypeToHive(field.getJavaType()), ""));
          }
          
          Descriptor valueDescriptor = MessageHelper.getMessageDescriptor(cfg.getValueType());
          for (FieldDescriptor field : valueDescriptor.getFields()) {
            fields.add(new FieldSchema(field.getName(), TypeUtils.javaTypeToHive(field.getJavaType()), ""));
          }
  
          tbl.setProperty("EXTERNAL", "TRUE");
          tbl.setTableType(TableType.EXTERNAL_TABLE);
          
          tbl.setInputFormatClass(RCFileInputFormat.class);
          tbl.setOutputFormatClass(RCFileOutputFormat.class);
          tbl.setFields(fields);
  
          log.info("create local table " + cfg.getName() + " with key " + keyDescriptor.getFullName() + ", value " + valueDescriptor.getFullName());
          
          db.createTable(tbl, true);
        } catch (HiveException e) {
          log.error("creating table " + cfg.getName() + " error, caused by " + e.getMessage(), e);
        }
      }
  }
  
}
