/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.parse;

import idgs.IdgsConfVars;

import org.apache.hadoop.hive.conf.HiveConf;
import org.apache.hadoop.hive.ql.parse.ASTNode;
import org.apache.hadoop.hive.ql.parse.BaseSemanticAnalyzer;
import org.apache.hadoop.hive.ql.parse.ExplainSemanticAnalyzer;
import org.apache.hadoop.hive.ql.parse.LoadSemanticAnalyzer;
import org.apache.hadoop.hive.ql.parse.SemanticAnalyzer;
import org.apache.hadoop.hive.ql.parse.SemanticAnalyzerFactory;
import org.apache.hadoop.hive.ql.parse.SemanticException;

public class IdgsSemanticAnalyzerFactory {

  public static BaseSemanticAnalyzer get(HiveConf conf, ASTNode tree) {
    try {
      BaseSemanticAnalyzer baseSem = SemanticAnalyzerFactory.get(conf, tree);

      if (baseSem instanceof SemanticAnalyzer) {
        return new IdgsSemanticAnalyzer(conf);
      } else if (baseSem instanceof LoadSemanticAnalyzer) {
        return new IdgsLoadSemanticAnalyzer(conf);
      } else if (baseSem instanceof ExplainSemanticAnalyzer) {
        String mode = IdgsConfVars.getVar(conf, IdgsConfVars.EXPLAIN_MODE);
        return mode.equals("idgs") ? null : baseSem;
      } else {
        return baseSem;
      }
    } catch (SemanticException e) {
      e.printStackTrace();
    }

    return null;
  }
}
