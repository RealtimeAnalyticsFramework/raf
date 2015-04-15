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
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFCase;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFCoalesce;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFConcatWS;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFHash;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFIf;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFIn;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFInstr;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFLocate;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFNvl;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPAnd;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPEqual;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPEqualOrGreaterThan;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPEqualOrLessThan;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPGreaterThan;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPLessThan;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPNot;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPNotEqual;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPNotNull;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPNull;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFOPOr;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFTimestamp;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFToBinary;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFToDecimal;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFToUnixTimeStamp;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFToUtcTimestamp;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFUnixTimeStamp;
import org.apache.hadoop.hive.ql.udf.generic.GenericUDFWhen;

import idgs.exception.IdgsParseException;
import idgs.pb.PbExpr.DataType;
import idgs.pb.PbExpr.Expr;
import idgs.util.TypeUtils;

public class ExprFactory {
  
  private static Log log = LogFactory.getLog(ExprFactory.class);
  
  public static final String FIELD = "FIELD";
  public static final String CONST = "CONST";
  
  private static Map<Class<?>, String> internalUdfType;
  
  static {
    internalUdfType = new HashMap<Class<?>, String>();
    internalUdfType.put(GenericUDFOPAnd.class, "AND");
    internalUdfType.put(GenericUDFOPOr.class, "OR");
    internalUdfType.put(GenericUDFOPEqual.class, "EQ");
    internalUdfType.put(GenericUDFOPNotEqual.class, "NE");
    internalUdfType.put(GenericUDFOPGreaterThan.class, "GT");
    internalUdfType.put(GenericUDFOPEqualOrGreaterThan.class, "GE");
    internalUdfType.put(GenericUDFOPLessThan.class, "LT");
    internalUdfType.put(GenericUDFOPEqualOrLessThan.class, "LE");
    internalUdfType.put(GenericUDFOPNot.class, "NOT");
    internalUdfType.put(GenericUDFOPNull.class, "ISNULL");
    internalUdfType.put(GenericUDFOPNotNull.class, "ISNOTNULL");
    internalUdfType.put(GenericUDFBetween.class, "BETWEEN");
    internalUdfType.put(GenericUDFIn.class, "IN");
    internalUdfType.put(GenericUDFHash.class, "HASH");
    internalUdfType.put(GenericUDFIf.class, "IF");
    internalUdfType.put(GenericUDFConcatWS.class, "CONCAT_WS");
    internalUdfType.put(GenericUDFCase.class, "CASE");
    internalUdfType.put(GenericUDFWhen.class, "WHEN");
    internalUdfType.put(GenericUDFNvl.class, "NVL");
    internalUdfType.put(GenericUDFCoalesce.class, "COALESCE");
    internalUdfType.put(GenericUDFLocate.class, "LOCATE");
    internalUdfType.put(GenericUDFInstr.class, "INSTR");
    internalUdfType.put(GenericUDFToBinary.class, "UDFTOBINARY");
    internalUdfType.put(GenericUDFToDecimal.class, "UDFTODECIMAL");
    internalUdfType.put(GenericUDFToUnixTimeStamp.class, "UNIX_TIMESTAMP");
    internalUdfType.put(GenericUDFToUtcTimestamp.class, "UNIX_TIMESTAMP");
    internalUdfType.put(GenericUDFTimestamp.class, "TIMESTAMP");
    internalUdfType.put(GenericUDFUnixTimeStamp.class, "UNIX_TIMESTAMP");
  }
  
  public static Expr buildExpression(ExprNodeDesc node, final Map<String, String> fieldSchemas) throws IdgsParseException {
    Expr.Builder exprBuilder = Expr.newBuilder();
    if (node instanceof ExprNodeColumnDesc) {
      ExprNodeColumnDesc colNode = (ExprNodeColumnDesc) node;
      exprBuilder.setName(FIELD);
      if (fieldSchemas != null && fieldSchemas.containsKey(colNode.getColumn())) {
        exprBuilder.setValue(fieldSchemas.get(colNode.getColumn()));
      } else {
        exprBuilder.setValue(colNode.getColumn());
      }
    } else if (node instanceof ExprNodeConstantDesc) {
      ExprNodeConstantDesc constNode = (ExprNodeConstantDesc) node;
      exprBuilder.setName(CONST);
      exprBuilder.setConstType(TypeUtils.hiveToDataType(constNode.getTypeInfo().getTypeName()));
      exprBuilder.setValue(constNode.getValue().toString());
    } else if (node instanceof ExprNodeGenericFuncDesc) {
      ExprNodeGenericFuncDesc exprNode = (ExprNodeGenericFuncDesc) node;
      GenericUDF udf = exprNode.getGenericUDF();
      if (udf instanceof GenericUDFBridge) {
        GenericUDFBridge bridge = (GenericUDFBridge) udf;
        String udfName = bridge.getUdfName().toUpperCase();
        if (udfName.equals("-") && exprNode.getChildExprs().size() == 1) {
          udfName = "NEGATIVE";
        }
        exprBuilder.setName(udfName);
      } else {
        if (internalUdfType.containsKey(udf.getClass())) {
          String udfName = internalUdfType.get(udf.getClass());
          exprBuilder.setName(udfName);
        } else {
          log.error("type " + udf.getClass().getSimpleName() + " is not supported yet.");
          throw new IdgsParseException("type " + udf.getClass().getSimpleName() + " is not supported yet.");
        }
      }
      
      for (ExprNodeDesc childExpr : exprNode.getChildExprs()) {
        Expr exp = buildExpression(childExpr, fieldSchemas);
        if (exp != null) {
          exprBuilder.addExpression(exp);
        }
      }
    } else {
      log.error("node type " + node.getClass().getSimpleName() + " is not supported yet.");
      throw new IdgsParseException("node type " + node.getClass().getSimpleName() + " is not supported yet.");
    }
    
    return exprBuilder.build();
  }

  public static Expr buildExpression(ExprNodeDesc node) throws IdgsParseException {
    return buildExpression(node, null);
  }

  public static Expr CONST(String value) {
    return CONST(value, DataType.STRING);
  }
  
  public static Expr CONST(String value, DataType type) {
    Expr.Builder expr = Expr.newBuilder();
    expr.setName("CONST");
    expr.setValue(value);
    expr.setConstType(type);
    return expr.build();
  }
  
  public static Expr FIELD(String value) {
    Expr.Builder expr = Expr.newBuilder();
    expr.setName("FIELD");
    expr.setValue(value);
    return expr.build();
  }
  
  public static Expr EXPR(String name, Expr ... children) {
    Expr.Builder expr = Expr.newBuilder();
    expr.setName(name);
    for (Expr child : children) {
      expr.addExpression(child);
    }
    
    return expr.build();
  }
  
}
