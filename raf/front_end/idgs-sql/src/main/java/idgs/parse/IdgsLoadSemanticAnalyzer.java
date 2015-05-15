/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.parse;

import java.io.IOException;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.net.URI;

import idgs.IdgsConfVars;
import idgs.execution.IdgsLoad;
import idgs.execution.IdgsLoadTask;
import idgs.metadata.StoreMetadata;
import idgs.store.pb.PbStoreConfig.StoreConfig;

import org.antlr.runtime.tree.Tree;
import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.hive.conf.HiveConf;
import org.apache.hadoop.hive.ql.exec.TaskFactory;
import org.apache.hadoop.hive.ql.metadata.HiveException;
import org.apache.hadoop.hive.ql.parse.ASTNode;
import org.apache.hadoop.hive.ql.parse.LoadSemanticAnalyzer;
import org.apache.hadoop.hive.ql.parse.SemanticException;

public class IdgsLoadSemanticAnalyzer extends LoadSemanticAnalyzer {

  private boolean isLocal;
  
  private boolean isOverWrite;
  
  public IdgsLoadSemanticAnalyzer(HiveConf conf) throws SemanticException {
    super(conf);
  }

  @Override
  public void analyzeInternal(ASTNode ast) throws SemanticException {
    isLocal = false;
    isOverWrite = false;
    Tree fromTree = ast.getChild(0);
    Tree tableTree = ast.getChild(1);

    if (ast.getChildCount() == 4) {
      isLocal = true;
      isOverWrite = true;
    }

    if (ast.getChildCount() == 3) {
      if (ast.getChild(2).getText().toLowerCase().equals("local")) {
        isLocal = true;
      } else {
        isOverWrite = true;
      }
    }

    URI uri = null;
    FileSystem fs = null;
    String fromPath = stripQuotes(fromTree.getText());
    
    try {
      Field isLocalField = LoadSemanticAnalyzer.class.getDeclaredField("isLocal");
      isLocalField.setAccessible(true);
      isLocalField.set(this, isLocal);
      
      Field isOverWriteField = LoadSemanticAnalyzer.class.getDeclaredField("isOverWrite");
      isOverWriteField.setAccessible(true);
      isLocalField.set(this, isOverWrite);
      
      Method initializeFromURIMethod = LoadSemanticAnalyzer.class.getDeclaredMethod("initializeFromURI", String.class);
      initializeFromURIMethod.setAccessible(true);
      uri = (URI) initializeFromURIMethod.invoke(this, fromPath);
      fs = checkURI(uri);
    } catch (SemanticException e) {
      throw e;
    } catch (Exception e) {
      throw new SemanticException(e);
    }
    
    if (fs == null) {
      throw new SemanticException("No files matching path " + fromPath);
    }

    tableSpec ts = new tableSpec(db, conf, (ASTNode) tableTree);

    String dbName = null;
    try {
      dbName = db.getDatabaseCurrent().getName();
    } catch (HiveException e) {
      throw new SemanticException("database " + dbName + " is not found", e);
    }
    
    String tableName = ts.tableName;
    StoreMetadata metadata = StoreMetadata.getInstance();
    String schemaName = metadata.getSchemaName(dbName);
    if (schemaName == null) {
      throw new SemanticException("database " + dbName + " is not found");
    }
    
    String storeName = metadata.getStoreName(dbName, tableName);
    if (storeName == null) {
      throw new SemanticException("table " + dbName + "." + tableName + " is not found");
    }
    
    StoreConfig storeConfig = metadata.getStoreConfig(schemaName, storeName);
    if (storeConfig == null) {
      throw new SemanticException("store " + schemaName + "." + storeName + " is not found");
    }
    
    genLoadTask(storeConfig, fs, uri.toString());
  }

  @SuppressWarnings("unchecked")
  private void genLoadTask(StoreConfig storeConfig, FileSystem fs, String dataFile) {
    int batchSize = IdgsConfVars.getIntVar(conf, IdgsConfVars.LOAD_BATCH_SIZE);
    int threadCount = IdgsConfVars.getIntVar(conf, IdgsConfVars.LOAD_THREAD_COUNT);
    IdgsLoadTask task = (IdgsLoadTask) TaskFactory.get(new IdgsLoad(storeConfig, fs, dataFile, batchSize, threadCount), conf);
    rootTasks.add(task);
  }
  
  private FileSystem checkURI(URI uri) throws SemanticException {
    try {
      if (isLocal && !uri.getScheme().equals("file")) {
        throw new SemanticException("Source file system should be \"file\" if \"local\" is specified");
      }
      
      FileSystem fs = FileSystem.get(uri, conf);
      
      Path path = new Path(uri.getScheme(), uri.getAuthority(), uri.getPath());
      
      FileStatus[] srcs = LoadSemanticAnalyzer.matchFilesOrDir(fs, path);
      
      if (srcs == null || srcs.length == 0) {
        throw new SemanticException("No files matching path " + uri);
      }

      for (FileStatus oneSrc : srcs) {
        if (oneSrc.isDir()) {
          throw new SemanticException("source contains directory: " + oneSrc.getPath().toString());
        }
      }
      
      return fs;
    } catch (IOException e) {
      throw new SemanticException(e);
    }
  }
  
}
