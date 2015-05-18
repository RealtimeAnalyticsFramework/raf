/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.execution;

import idgs.store.pb.PbStoreConfig.StoreConfig;

import java.io.Serializable;

import org.apache.hadoop.fs.FileSystem;

public class IdgsLoad implements Serializable {

  private static final long serialVersionUID = 1L;
  
  private StoreConfig storeConfig;
  
  private FileSystem fs;
  
  private String dataFile;
  
  private int batchSize;
  
  private int threadCount;
  
  public IdgsLoad(StoreConfig storeConfig, FileSystem fs, String dataFile, int batchSize, int threadCount) {
    this.storeConfig = storeConfig;
    this.fs = fs;
    this.batchSize = batchSize;
    this.threadCount = threadCount;
    this.dataFile = dataFile;
  }

  public StoreConfig getStoreConfig() {
    return storeConfig;
  }

  public FileSystem getFileSystem() {
    return fs;
  }
  
  public String getDataFile() {
    return dataFile;
  }

  public int getBatchSize() {
    return batchSize;
  }
  
  public int getThreadCount() {
    return threadCount;
  }
  
}
