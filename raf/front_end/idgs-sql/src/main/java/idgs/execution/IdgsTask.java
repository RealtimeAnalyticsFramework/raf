/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.execution;

import idgs.exception.IdgsException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.hive.ql.Context;
import org.apache.hadoop.hive.ql.DriverContext;
import org.apache.hadoop.hive.ql.exec.Task;
import org.apache.hadoop.hive.ql.plan.api.StageType;

import printer.DefaultResultPrinter;
import printer.ResultPrinter;

public class IdgsTask extends Task<IdgsWork> {

  private static Log LOG = LogFactory.getLog(IdgsTask.class);
  
  private static final long serialVersionUID = 1L;
  
  private static boolean stepShowResult = false;
  
  @Override
  protected int execute(DriverContext driverContext) {
    return process(work.getTopOp());
  }

  private int process(IdgsOperator operator) {
    if (operator.getChildrenOperators() != null) {
      for (IdgsOperator op : operator.getChildrenOperators()) {
        int ret = process(op);
        
        if (ret > 0) {
          return ret;
        }
      }
    }
    
    try {
      operator.process();
      
      if (stepShowResult) {
        stepShowResults(operator);
      }
      return 0;
    } catch (IdgsException e) {
      e.printStackTrace();
      LOG.error(e.getMessage());
      return 2;
    }
  }
  
  private void stepShowResults(IdgsOperator op) {
    try {
      if (op instanceof ActionOperator) {
        return;
      }
      
      ActionOperator action = new CollectActionOperator();
      action.addChildOperator(op);
      action.process();
      
      ResultPrinter printer = new DefaultResultPrinter(action.getResultSet());
      LOG.debug("=========================================================================================");
      LOG.debug(op.getClass().getSimpleName());
      LOG.debug(action.getResultSet().getRowCount());
      LOG.debug(printer.printResults());
      LOG.debug("=========================================================================================");
    } catch (IdgsException e) {
      e.printStackTrace();
    }
  }
  
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
  
  public ResultSet getResultSet() {
    return work.getActionOperator().getResultSet();
  }
  
}
