/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.parse;

import java.util.HashMap;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.hive.ql.plan.ExprNodeColumnDesc;
import org.apache.hadoop.hive.ql.plan.ExprNodeConstantDesc;
import org.apache.hadoop.hive.ql.plan.ExprNodeDesc;
import org.apache.hadoop.hive.ql.plan.ExprNodeGenericFuncDesc;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDF;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFBetween;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFBridge;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPAnd;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPEqual;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPEqualOrGreaterThan;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPEqualOrLessThan;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPGreaterThan;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPLessThan;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPNot;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPNotEqual;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPOr;

import idgs.pb.PbExpr.Expr;
import idgs.pb.PbExpr.ExpressionType;
import idgs.util.TypeUtils;

public class ExprFactory {
  
  private static Log log = LogFactory.getLog(ExprFactory.class);
  
  private static Map<String, ExpressionType> udfExpressionType;
  
  static {
    udfExpressionType = new HashMap<String, ExpressionType>();
    udfExpressionType.put("substr", ExpressionType.SUBSTR);
    udfExpressionType.put("+", ExpressionType.ADD);
    udfExpressionType.put("-", ExpressionType.SUBTRACT);
    udfExpressionType.put("*", ExpressionType.MULTIPLY);
    udfExpressionType.put("/", ExpressionType.DIVIDE);
    udfExpressionType.put("%", ExpressionType.MODULUS);
  }
  
  public static Expr buildExpression(ExprNodeDesc node, final Map<String, String> fieldSchemas) {
    Expr.Builder exprBuilder = Expr.newBuilder();
    if (node instanceof ExprNodeColumnDesc) {
      ExprNodeColumnDesc colNode = (ExprNodeColumnDesc) node;
      exprBuilder.setType(ExpressionType.FIELD);
      if (fieldSchemas != null && fieldSchemas.containsKey(colNode.getColumn())) {
        exprBuilder.setValue(fieldSchemas.get(colNode.getColumn()));
      } else {
        exprBuilder.setValue(colNode.getColumn());
      }
    } else if (node instanceof ExprNodeConstantDesc) {
      ExprNodeConstantDesc constNode = (ExprNodeConstantDesc) node;
      exprBuilder.setType(ExpressionType.CONST);
      exprBuilder.setConstType(TypeUtils.hiveToDataType(constNode.getTypeInfo().getTypeName()));
      exprBuilder.setValue(constNode.getValue().toString());
    } else if (node instanceof ExprNodeGenericFuncDesc) {
      ExprNodeGenericFuncDesc exprNode = (ExprNodeGenericFuncDesc) node;
      GenericUDF udf = exprNode.getGenericUDF();
      if (udf instanceof GenericUDFBridge) {
        GenericUDFBridge bridge = (GenericUDFBridge) udf;
        exprBuilder.setType(udfExpressionType.get(bridge.getUdfName()));
      } else if (udf instanceof GenericUDFOPAnd) {
        exprBuilder.setType(ExpressionType.AND);
      } else if (udf instanceof GenericUDFOPOr) {
        exprBuilder.setType(ExpressionType.OR);
      } else if (udf instanceof GenericUDFOPEqual) {
        exprBuilder.setType(ExpressionType.EQ);
      } else if (udf instanceof GenericUDFOPNotEqual) {
        exprBuilder.setType(ExpressionType.NE);
      } else if (udf instanceof GenericUDFOPGreaterThan) {
        exprBuilder.setType(ExpressionType.GT);
      } else if (udf instanceof GenericUDFOPEqualOrGreaterThan) {
        exprBuilder.setType(ExpressionType.GE);
      } else if (udf instanceof GenericUDFOPLessThan) {
        exprBuilder.setType(ExpressionType.LT);
      } else if (udf instanceof GenericUDFOPEqualOrLessThan) {
        exprBuilder.setType(ExpressionType.LE);
      } else if (udf instanceof GenericUDFOPNot) {
        exprBuilder.setType(ExpressionType.NOT);
      } else if (udf instanceof GenericUDFBetween) {
        exprBuilder.setType(ExpressionType.BETWEEN);
      } else {
        log.error("type " + udf.getClass().getSimpleName() + " is not supported yet.");
        System.out.println("type " + udf.getClass().getSimpleName() + " is not supported yet.");
        return null;
      }
      
      for (ExprNodeDesc childExpr : exprNode.getChildExprs()) {
        exprBuilder.addExpression(buildExpression(childExpr, fieldSchemas));
      }
    } else {
      log.error("type " + node.getClass().getSimpleName() + " is not supported yet.");
      System.out.println("type " + node.getClass().getSimpleName() + " is not supported yet.");
      return null;
    }
    
    return exprBuilder.build();
  }

  public static Expr buildExpression(ExprNodeDesc node) {
    return buildExpression(node, null);
  }
  
}
