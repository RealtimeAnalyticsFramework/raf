/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.metadata;

import idgs.IdgsCliDriver;
import idgs.IdgsConfVars;
import idgs.exception.IdgsException;
import idgs.store.pb.PbStoreConfig.DataStoreConfig;
import idgs.store.pb.PbStoreConfig.StoreConfig;
import idgs.store.pb.PbStoreConfig.StoreSchema;
import idgs.util.TypeUtils;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.hive.conf.HiveConf;
import org.apache.hadoop.hive.metastore.TableType;
import org.apache.hadoop.hive.metastore.api.AlreadyExistsException;
import org.apache.hadoop.hive.metastore.api.Database;
import org.apache.hadoop.hive.metastore.api.FieldSchema;
import org.apache.hadoop.hive.ql.io.RCFileInputFormat;
import org.apache.hadoop.hive.ql.io.RCFileOutputFormat;
import org.apache.hadoop.hive.ql.metadata.Hive;
import org.apache.hadoop.hive.ql.metadata.HiveException;
import org.apache.hadoop.hive.ql.metadata.Table;
import org.apache.hadoop.hive.serde.serdeConstants;

import com.google.protobuf.Descriptors.Descriptor;
import com.google.protobuf.Descriptors.FieldDescriptor;

import protubuf.MessageHelper;

public class StoreMetadata {

  private static Log LOG = LogFactory.getLog(StoreMetadata.class);
  
  private Map<String, StoreConfig> storeCache;
  
  private Map<String, String> schemaNames;
  
  private Map<String, String> storeNames;
  
  private static StoreMetadata instance;
  
  private StoreMetadata() {
    if (storeCache == null) {
      storeCache = new HashMap<String, StoreConfig>();
    }
    
    if (storeNames == null) {
      storeNames = new HashMap<String, String>();
    }
    
    if (schemaNames == null) {
      schemaNames = new HashMap<String, String>();
    }
  }
  
  public static StoreMetadata getInstance() {
    if (instance == null) {
      instance = new StoreMetadata();
    }
    
    return instance;
  }

  public String getSchemaName(String dbName) {
    return schemaNames.get(dbName.toLowerCase());
  }
  
  public String getStoreName(String dbName, String tableName) {
    return storeNames.get(dbName.toLowerCase() + "." + tableName.toLowerCase());
  }
  
  public StoreConfig getStoreConfig(String schemaName, String storeName) {
    return storeCache.get(schemaName + "." + storeName);
  }
  
  public void initMetadata(HiveConf hiveConf, DataStoreConfig storeConfig) throws IdgsException {
    Hive db = null;

    Map<String, String> dbMap = new HashMap<String, String>();
    Map<String, String> tableMap = new HashMap<String, String>();
    try {
      db = Hive.get(hiveConf);
      for (String database : db.getAllDatabases()) {
        dbMap.put(database.toLowerCase(), database);
        List<String> tblList = db.getAllTables(database);
        for (String tbl : tblList) {
          tableMap.put(database.toLowerCase() + "." + tbl.toLowerCase(), tbl);
        }
      }
    } catch (HiveException e) {
      LOG.error("initialize store metadata error, caused by " + e.getMessage());
      throw new IdgsException(e);
    }
    
    String storeCfgFile = IdgsConfVars.getVar(hiveConf, IdgsConfVars.STORE_CONFIG_FILE);
    URL url = IdgsCliDriver.class.getClassLoader().getResource(storeCfgFile);
    
    List<StoreSchema> schemas = storeConfig.getSchemasList();
    LOG.info("schemas " + schemas.size());
    for (int i = 0; i < schemas.size(); ++ i) {
      StoreSchema schema = schemas.get(i);
      
      String baseDir = null;
      if (url != null) {
        baseDir = new File(url.getFile()).getParent();
      }
      registerStoreMessage(schema, baseDir, storeCfgFile);
      
      String dbName = schema.getSchemaName().toLowerCase();
      if (!dbMap.containsKey(dbName)) {
        try {
          Database database = new Database();
          database.setName(dbName);
          db.createDatabase(database);
        } catch (AlreadyExistsException e) {
          LOG.warn("database " + schema.getSchemaName() + " is already exists.");
        } catch (HiveException e) {
          LOG.error("error create database " + schema.getSchemaName(), e);
        }
      }

      schemaNames.put(dbName, schema.getSchemaName());
      for (StoreConfig store : schema.getStoreConfigList()) {
        if (!store.hasFieldSeperator()) {
          store = store.toBuilder().setFieldSeperator("\\|").build();
        }
        
        storeNames.put(dbName + "." + store.getName().toLowerCase(), store.getName());
        storeCache.put(schema.getSchemaName() + "." + store.getName(), store);
        
        String tableName = store.getName().toLowerCase();
        if (tableMap.containsKey(dbName + "." + tableName)) {
          LOG.info("table " + dbName + "." + tableName + " exists, next table.");
          continue;
        }
        
        String keyType = store.getKeyType();
        String valueType = store.getValueType();
        
        if (!MessageHelper.isMessageRegistered(keyType)) { 
          throw new IdgsException("cannot register message with key type " + store.getKeyType());
        }
        
        if (!MessageHelper.isMessageRegistered(valueType)) {
          throw new IdgsException("cannot register message with key type " + store.getValueType());
        }
        
        try {
          createLocalHiveTable(db, dbName, tableName, store.getFieldSeperator(), keyType, valueType);
        } catch (HiveException e) {
          LOG.error("creating table " + store.getName() + " error, caused by " + e.getMessage(), e);
        }
      }
    }
  }
  
  private void registerStoreMessage(StoreSchema schema, String baseDir, String storeCfgFile) throws IdgsException {
    String filePath = schema.getProtoFilename();
    filePath = filePath.replaceAll("\\\\", "/");

    File file = null;
    if (schema.hasProtoFilename()) {
      LOG.info("register file " + schema.getProtoFilename());
      
      do {
        // directly
        file = new File(filePath);
        if (file.exists()) {
          break;
        }
        String temppath;

        // same dir as store config
        file = new File(storeCfgFile);
        if (file.exists()) {
          temppath = file.getParent() + "/" + filePath;
          file = new File(temppath);
          if (file.exists()) {
            filePath = temppath;
            break;
          }
        } 
          
        // base dir
        if (baseDir != null) {
          temppath = baseDir + "/" + filePath;
          file = new File(temppath);
          if (file.exists()) {
            filePath = temppath;
            break;
          }
        }
        
        // IDGS_HOME env
        if ((temppath = System.getenv("IDGS_HOME")) != null) {
          temppath = temppath + "/" + filePath;
          file = new File(temppath);
          if (file.exists()) {
            filePath = temppath;
            break;
          }
        } 
          
        throw new IdgsException("cannot get proto file " + filePath + " for store " + storeCfgFile);
      } while(false);
      
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
  }
  
  private void createLocalHiveTable(Hive db, String dbName, String tableName, String seperator, String keyType, String valueType) throws HiveException {
    Table tbl = db.newTable(tableName);
    
    List<FieldSchema> fields = new ArrayList<FieldSchema>();
    
    Descriptor keyDescriptor = MessageHelper.getMessageDescriptor(keyType);
    for (FieldDescriptor field : keyDescriptor.getFields()) {
      fields.add(new FieldSchema(field.getName(), TypeUtils.javaTypeToHive(field.getJavaType()), ""));
    }
    
    Descriptor valueDescriptor = MessageHelper.getMessageDescriptor(valueType);
    for (FieldDescriptor field : valueDescriptor.getFields()) {
      fields.add(new FieldSchema(field.getName(), TypeUtils.javaTypeToHive(field.getJavaType()), ""));
    }

    tbl.setDbName(dbName);
    tbl.setProperty("EXTERNAL", "TRUE");
    tbl.setTableType(TableType.EXTERNAL_TABLE);
    
    tbl.setInputFormatClass(RCFileInputFormat.class);
    tbl.setOutputFormatClass(RCFileOutputFormat.class);
    tbl.setFields(fields);
    
    tbl.setSerdeParam(serdeConstants.FIELD_DELIM, seperator);
    tbl.setSerdeParam(serdeConstants.SERIALIZATION_FORMAT, seperator);

    LOG.info("create local table " + tableName + " with key " + keyDescriptor.getFullName() + ", value " + valueDescriptor.getFullName());
    
    db.createTable(tbl, true);
  }
  
}
