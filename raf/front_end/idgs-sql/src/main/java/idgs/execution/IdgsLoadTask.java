/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.execution;

import idgs.exception.IdgsException;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.fs.FSDataInputStream;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.hive.ql.Context;
import org.apache.hadoop.hive.ql.DriverContext;
import org.apache.hadoop.hive.ql.exec.Task;
import org.apache.hadoop.hive.ql.plan.api.StageType;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.util.LineReader;

public class IdgsLoadTask extends Task<IdgsLoad> {

  private static final long serialVersionUID = 1L;
  
  private static Log LOG = LogFactory.getLog(IdgsLoadTask.class);
  
  @Override
  protected int execute(DriverContext ctx) {
    FileSystem fs = work.getFileSystem();
    String dataFile = work.getDataFile();
    Path dataPath = new Path(dataFile);

    InsertOperator op = new InsertOperator(work.getStoreConfig());
    
    int batchSize = work.getBatchSize();
    List<String> buffer = new ArrayList<String>();
    
    FSDataInputStream in = null;
    LineReader reader = null;
    try {
      in = fs.open(dataPath);
      reader = new LineReader(in);
      Text line = new Text();
      int res = 1;
      while (res > 0) {
        res = reader.readLine(line);
        if (line.toString().trim().isEmpty()) {
          continue;
        }
        
        buffer.add(line.toString());
        if (buffer.size() == batchSize) {
          for (String row : buffer) {
            try {
              op.parseRow(row);
              op.process();
            } catch (IdgsException e) {
              LOG.error("Error when insert data " + row + ", caused by " + e.getMessage() + ".", e);
              return 2;
            }
          }
          buffer.clear();
        }
      }
    } catch (IOException e) {
      LOG.error("Cannot open file " + dataFile + ".", e);
      return 1;
    } finally {
      if (reader != null) {
        try {
          reader.close();
        } catch (IOException e) {
          e.printStackTrace();
        }
      }
      
      if (in != null) {
        try {
          in.close();
        } catch (IOException e) {
          e.printStackTrace();
        }
      }
      
      try {
        fs.close();
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
    
    return 0;
  }
  
  @Override
  public String getName() {
    return "MAPRED-IDGS";
  }
  
  @Override
  public StageType getType() {
    return StageType.MAPRED;
  }

  @Override
  protected void localizeMRTmpFilesImpl(Context ctx) {
    super.localizeMRTmpFiles(ctx);
  }
  
}
