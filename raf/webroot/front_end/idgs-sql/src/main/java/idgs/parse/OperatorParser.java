/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.parse;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.hive.ql.exec.ColumnInfo;
import org.apache.hadoop.hive.ql.exec.ExtractOperator;
import org.apache.hadoop.hive.ql.exec.FileSinkOperator;
import org.apache.hadoop.hive.ql.exec.FilterOperator;
import org.apache.hadoop.hive.ql.exec.GroupByOperator;
import org.apache.hadoop.hive.ql.exec.JoinOperator;
import org.apache.hadoop.hive.ql.exec.LimitOperator;
import org.apache.hadoop.hive.ql.exec.Operator;
import org.apache.hadoop.hive.ql.exec.ReduceSinkOperator;
import org.apache.hadoop.hive.ql.exec.SelectOperator;
import org.apache.hadoop.hive.ql.exec.TableScanOperator;
import org.apache.hadoop.hive.ql.exec.UnionOperator;
import org.apache.hadoop.hive.ql.metadata.Table;
import org.apache.hadoop.hive.ql.parse.OpParseContext;
import org.apache.hadoop.hive.ql.parse.ParseContext;
import org.apache.hadoop.hive.ql.plan.AggregationDesc;
import org.apache.hadoop.hive.ql.plan.ExprNodeColumnDesc;
import org.apache.hadoop.hive.ql.plan.ExprNodeConstantDesc;
import org.apache.hadoop.hive.ql.plan.ExprNodeDesc;
import org.apache.hadoop.hive.ql.plan.JoinDesc;
import org.apache.hadoop.hive.ql.plan.OperatorDesc;

import protubuf.MessageHelper;

import com.google.protobuf.Descriptors.Descriptor;
import com.google.protobuf.Descriptors.FieldDescriptor;

import idgs.exception.IdgsParseException;
import idgs.execution.ActionOperator;
import idgs.execution.CollectActionOperator;
import idgs.execution.DelegateOperator;
import idgs.execution.FilterTransformerOperator;
import idgs.execution.GroupTransformerOperator;
import idgs.execution.HashJoinTransformerOperator;
import idgs.execution.IdgsOperator;
import idgs.execution.ReduceByKeyTransformerOperator;
import idgs.execution.ReduceTransformerOperator;
import idgs.execution.TopNActionOperator;
import idgs.execution.TransformerOperator;
import idgs.execution.UnionTransformerOperator;
import idgs.metadata.StoreMetadata;
import idgs.pb.PbExpr.DataType;
import idgs.pb.PbExpr.Expr;
import idgs.rdd.pb.PbRddService.FieldNamePair;
import idgs.rdd.pb.PbRddService.FieldNamePair.Builder;
import idgs.rdd.pb.PbRddTransform.JoinType;
import idgs.store.pb.PbStoreConfig.StoreConfig;
import idgs.util.TypeUtils;

public class OperatorParser {
  
  private static Log LOG = LogFactory.getLog(OperatorParser.class);
  
  private ParseContext pctx;
  
  private FileSinkOperator sinkOp;
  
  LinkedHashMap<Operator<? extends OperatorDesc>, OpParseContext> opParseCtx;
  
  /**
   * construct to make parse hive operators to idgs operators
   * 
   * @param pctx    context of hive parser
   * @param sinkOp  finally hive operators
   */
  public OperatorParser(ParseContext pctx, FileSinkOperator sinkOp) {
    this.pctx = pctx;
    this.sinkOp = sinkOp;
    if (LOG.isDebugEnabled()) {
      LOG.debug(toHiveOpString(sinkOp, ""));
    }
  }
  
  public String hiveOpToString(Operator<?> operator, String tab) {
    StringBuffer sb = new StringBuffer();
    sb.append(tab).append(operator.getClass().getSimpleName()).append("\n");
    if (operator.getParentOperators() != null) {
      for (Operator<?> op : operator.getParentOperators()) {
        sb.append(hiveOpToString(op, tab + "  "));
      }
    }
    return sb.toString();
  }
  
  /**
   * parse hive operators to idgs operators
   * 
   * @return finally idgs operators
   * @throws IdgsParseException 
   */
  public IdgsOperator parseOperator() throws IdgsParseException {
    if (sinkOp.getParentOperators() == null || sinkOp.getParentOperators().isEmpty()) {
      return null;
    }
    
    // parse all hive operators
    List<IdgsOperator> opList = parseOperator(sinkOp);
    if (opList == null || opList.size() != 1) {
      throw new IdgsParseException("parse operator error");
    }

    // add a filter operator before action operator to change column alias.
    IdgsOperator operator = opList.get(0);
    // generate finally action operator, it is CollectActionOperator or TopNActionOperator
    ActionOperator actionOp = genActionOperator(operator);
    
    if (LOG.isDebugEnabled()) {
      LOG.debug(actionOp.toTreeString());
    }
    
    return actionOp;
  }
  
  private ActionOperator genActionOperator(IdgsOperator operator) throws IdgsParseException {
    Operator<? extends OperatorDesc> op = sinkOp.getParentOperators().get(0);
    // hive operator is ExtractOperator, use TopNActionOperator to handle sql with order by.
    int limit = -1;
    if (op instanceof LimitOperator) {
      limit = ((LimitOperator) op).getConf().getLimit();
      op = op.getParentOperators().get(0);
    }
    
    Operator<? extends OperatorDesc> orderbyOp = null;
    
    if (op.getParentOperators().size() == 1) {
      Operator<? extends OperatorDesc> rsOp = op.getParentOperators().get(0);
      if (rsOp instanceof ReduceSinkOperator) {
        if (!((ReduceSinkOperator) rsOp).getConf().getOrder().equals("")) {
          orderbyOp = rsOp;
        }
      }
    }

    IdgsOperator chOp = null;
    if (operator.getOutputKeyFields() != null || operator.getOutputValueFields() != null) {
      IdgsOperator filterTrans = new FilterTransformerOperator();
      filterTrans.addChildOperator(operator);
      chOp = filterTrans;
    } else {
      chOp = operator;
    }
    
    ActionOperator topOp = null;
    if (orderbyOp != null) {
      ArrayList<ColumnInfo> colInfoList = pctx.getOpParseCtx().get(op).getRowResolver().getColumnInfos();
      // column mapping from hive internal name to real alias.
      Map<String, String> fieldSchemas = new HashMap<String, String>();
      for (ColumnInfo colInfo : colInfoList) {
        fieldSchemas.put(colInfo.getInternalName(), colInfo.getAlias());
      }
      
      List<FieldNamePair> fields = operator.getOutputValueFields();
      for (int i = 0; i < fields.size(); ++ i) {
        FieldNamePair field = fields.get(i);
        String alias = field.getFieldAlias();
        if (fieldSchemas.containsKey(alias)) {
          field = field.toBuilder().setFieldAlias(fieldSchemas.get(alias)).build();
          fields.set(i, field);
        }
      }
      
      ReduceSinkOperator reduceOp = (ReduceSinkOperator) orderbyOp;
      topOp = new TopNActionOperator();
      
      SelectOperator selOp = (SelectOperator) op;
      Map<String, String> mapping = new HashMap<String, String>();
      List<ExprNodeDesc> colList = selOp.getConf().getColList();
      List<String> colNames = selOp.getConf().getOutputColumnNames();
      for (int i = 0; i < colList.size(); ++ i) {
        ExprNodeDesc node = selOp.getConf().getColList().get(i);
        if (node instanceof ExprNodeColumnDesc) {
          String column = ((ExprNodeColumnDesc) node).getColumn();
          mapping.put(column, colNames.get(i));
        }
      }
      
      // order type of column, "+" is asc and "-" is desc
      String order = reduceOp.getConf().getOrder();
      // order by column
      List<ExprNodeDesc> keyCols = reduceOp.getConf().getKeyCols();
      // order by column alias
      List<String> keyOutputNames = reduceOp.getConf().getOutputKeyColumnNames();

      /// handle each order by field, add to TopNActionOperator
      for (int i = 0; i < keyCols.size(); ++ i) {
        ExprNodeDesc node = keyCols.get(i);
        DataType dataType = getDataType(node);
        String keyName = keyOutputNames.get(i);
        
        FieldNamePair.Builder builder = FieldNamePair.newBuilder();
        builder.setFieldAlias(keyName);
        builder.setFieldType(dataType);
        builder.setExpr(ExprFactory.buildExpression(node, fieldSchemas));
        FieldNamePair field = builder.build();
        
        if (!(node instanceof ExprNodeColumnDesc)) {
          if (mapping.containsKey("KEY." + keyName)) {
            String internalAilas = mapping.get("KEY." + keyName);
            if (fieldSchemas.containsKey(internalAilas)) {
              String alias = fieldSchemas.get(internalAilas);
              fields.add(field.toBuilder().setFieldAlias(alias).build());
            }
          }
        }
        
        boolean desc = false;
        if (i < order.length()) {
          desc = (order.charAt(i) == '-');
        }
        ((TopNActionOperator) topOp).addOrderField(field, desc);
      }
    } else {
      topOp = new CollectActionOperator();
    }

    if (limit > 0) {
      topOp.setLimit(limit);
    }
    topOp.addChildOperator(chOp);
    
    return topOp;
  }
  
  /**
   * parse hive operators to idgs operators
   * @throws IdgsParseException 
   */
  private List<IdgsOperator> parseOperator(Operator<? extends OperatorDesc> operator) throws IdgsParseException {
    List<IdgsOperator> children = null;
    // recursive parse operators
    if (operator.getParentOperators() != null) {
      children = new ArrayList<IdgsOperator>();
      for (Operator<? extends OperatorDesc> parentOp : operator.getParentOperators()) {
        children.addAll(parseOperator(parentOp));
      }
    }
    
    List<IdgsOperator> ret = null;
    IdgsOperator oper = null;
    // parse table scan operator to delegate operator
    if (operator instanceof TableScanOperator) {
      oper = genTablePlan(operator);
    // parse group by operator
    } else if (operator instanceof GroupByOperator) {
      oper = genGroupByPlan(operator, children);
    // parse union operator
    } else if (operator instanceof UnionOperator) {
      oper = genUnionOperator(operator, children);
    // parse join operator
    } else if (operator instanceof JoinOperator) {
      oper = genJoinPlan(operator, children);
    // parse extract operator to handle order by in sub query
    } else if (operator instanceof ExtractOperator) {
      if (operator.getChildOperators().size() == 1 && operator.getChildOperators().get(0) instanceof FileSinkOperator) {
        oper = genExtractPlan(operator, children);
      } else {
        return children;
      }
    } else if (operator instanceof LimitOperator) {
      return children;
    } else {
      return children;
    }
    
    ret = new ArrayList<IdgsOperator>();
    ret.add(oper);
    
    return ret;
  }
  
  /**
   * parse table scan operator to delegate operator
   * @return DelegateOperator
   * @throws IdgsParseException 
   */
  private IdgsOperator genTablePlan(Operator<? extends OperatorDesc> operator) throws IdgsParseException {
    // get store name from table name
    TableScanOperator tabOp = (TableScanOperator) operator;
    Table tbl = pctx.getTopToTable().get(tabOp);
    String dbName = tbl.getDbName();
    String tableName = tbl.getTableName();
    StoreMetadata metadata = StoreMetadata.getInstance();
    String schemaName = metadata.getSchemaName(dbName);
    if (schemaName == null) {
      throw new IdgsParseException("schema " + dbName + " is not found.");
    }
    
    String storeName = metadata.getStoreName(dbName, tableName);
    if (storeName == null) {
      throw new IdgsParseException("store " + dbName + "." + tableName + " is not found.");
    }
    
    StoreConfig cfg = metadata.getStoreConfig(schemaName, storeName);
    if (cfg == null) {
      throw new IdgsParseException("store " + schemaName + "." + storeName + " is not found.");
    }
    
    // make delegate operator and set its filter and select
    IdgsOperator delegate = new DelegateOperator(schemaName, cfg);
    genFilterAndSelectExpr(operator, delegate);
    
    return delegate;
  }
  
  /**
   * parse group operator
   * 1. Aggregate with not group, like "select sum(c) from t". 
   * 2. Group by with column witch is not key.  
   * 3. Group by with key.
   * @return 1. ReduceByKeyTransformerOperator[GroupTransformerOperator]
   *         2. ReduceByKeyTransformerOperator
   *         3. ReduceTransformerOperator
   *         4. ReduceTransformerOperator[GroupTransformerOperator]
   *         5. HashjoinTransformerOperator[ReduceTransformerOperator[GroupTransformerOperator], ..., ReduceTransformerOperator, ...]
   * @throws IdgsParseException 
   */
  private IdgsOperator genGroupByPlan(Operator<? extends OperatorDesc> operator, List<IdgsOperator> children) throws IdgsParseException {
    IdgsOperator op = children.get(0);
    
    GroupByOperator groupOp = (GroupByOperator) operator;
    
    Operator<? extends OperatorDesc> lastOperator = operator.getParentOperators().get(0);
    if (lastOperator instanceof ReduceSinkOperator) {
      ReduceSinkOperator reduceOp = (ReduceSinkOperator) operator.getParentOperators().get(0);
      ArrayList<ExprNodeDesc> grpKeyCols = groupOp.getConf().getKeys();
      ArrayList<ExprNodeDesc> keyCols = reduceOp.getConf().getKeyCols();
      
      IdgsOperator grpOperator = null;
      if (grpKeyCols.isEmpty() && keyCols.isEmpty()) {
        // sql has no group by and distinct
        grpOperator = genGroupByPlanWithNoKey(operator, op);
      } else if (grpKeyCols.isEmpty() && !keyCols.isEmpty()) {
        // sql has no group by but has distinct
        grpOperator = genGroupByPlanWithDistinct(operator, op);
      } else if (isReuseKey(op, keyCols.subList(0, grpKeyCols.size()))) {
        // sql has group by and group by field is key
        grpOperator = genGroupByPlanReuseKey(operator, op);
      } else {
        // sql has group by and group by field is not key
        grpOperator = genGroupByPlan(operator, op);
      }

      genFilterAndSelectExpr(operator, grpOperator);
      return grpOperator;
    } else {
      LOG.error("Cannot parse GroupByOperator without ReduceSinkOperator, not supported yet.");
      throw new IdgsParseException("Not supported yet.");
    }
  }
  
  /**
   * parse group operator with not group by and distinct
   * @return ReduceTransformerOperator not contain AVG
   *         ReduceTransformerOperator[FilterTransformerOperator] contain AVG
   * @throws IdgsParseException 
   */
  private IdgsOperator genGroupByPlanWithNoKey(Operator<? extends OperatorDesc> operator, IdgsOperator op) throws IdgsParseException {
    // build value template with column.
    Map<String, FieldNamePair> fieldMapping = new HashMap<String, FieldNamePair>();

    ReduceSinkOperator reduceOp = (ReduceSinkOperator) operator.getParentOperators().get(0);
    ArrayList<ExprNodeDesc> valueCols = reduceOp.getConf().getValueCols();
    ArrayList<ColumnInfo> reduceSchema = reduceOp.getSchema().getSignature();
    
    int valueIndex = 0;
    for (ColumnInfo colInfo : reduceSchema) {
      String alias = colInfo.getInternalName();
      if (alias.startsWith("VALUE.")) {
        ExprNodeDesc colNode = valueCols.get(valueIndex ++);
        FieldNamePair expr = genFieldExpr(colNode, alias);
        fieldMapping.put(alias, expr);
      }
    }

    boolean hasAvg = false;
    ReduceTransformerOperator reduceTrans = new ReduceTransformerOperator();
    
    GroupByOperator groupOp = (GroupByOperator) operator;
    ArrayList<AggregationDesc> aggr = groupOp.getConf().getAggregators();
    // reset values of last operator
    List<FieldNamePair> valueFields = new ArrayList<FieldNamePair>();
    // value fields of reduce transformer
    List<FieldNamePair> reduceFields = new ArrayList<FieldNamePair>();
    // value fields of reduce by key transformer
    List<FieldNamePair> filterFields = new ArrayList<FieldNamePair>();
    // handle aggregate function
    for (int i = 0; i < aggr.size(); ++ i) {
      AggregationDesc aggrDesc = aggr.get(i);
      String alias = "_col" + i;
      String reduceType = aggrDesc.getGenericUDAFName().toUpperCase();
      
      // column node for each aggregation
      ExprNodeDesc node = aggrDesc.getParameters().get(0);
      // according node build value expression
      FieldNamePair expr = null;
      if (node instanceof ExprNodeColumnDesc) {
        FieldNamePair field = fieldMapping.get(((ExprNodeColumnDesc) node).getColumn());
        if (field != null) {
          FieldNamePair.Builder fieldBuilder = field.toBuilder().clone();
          fieldBuilder.setFieldAlias(alias);
          expr = fieldBuilder.build();
        } else {
          continue;
        }
      } else if (node instanceof ExprNodeConstantDesc) {
        expr = genFieldExpr(node, alias);
      } else {
        DataType dataType = getDataType(node);
        
        FieldNamePair.Builder fieldBuilder = FieldNamePair.newBuilder();
        fieldBuilder.setFieldAlias(alias);
        fieldBuilder.setFieldType(dataType);
        fieldBuilder.setExpr(ExprFactory.FIELD(alias));
        expr = fieldBuilder.build();
      }
      
      // if current aggregation is average, make the column to sum and count column
      // else expr is value field expression
      if (reduceType.equals("AVG")) {
        hasAvg = true;
        FieldNamePair.Builder countBuilder = expr.toBuilder().clone();
        countBuilder.setFieldAlias(alias + "_count");
        countBuilder.setFieldType(expr.getFieldType());
        valueFields.add(countBuilder.build());

        FieldNamePair.Builder sumBuilder = expr.toBuilder().clone();
        sumBuilder.setFieldAlias(alias + "_sum");
        sumBuilder.setFieldType(expr.getFieldType());
        valueFields.add(sumBuilder.build());

        FieldNamePair.Builder avgFieldBuilder = FieldNamePair.newBuilder();
        avgFieldBuilder.setFieldAlias(alias);
        avgFieldBuilder.setFieldType(DataType.DOUBLE);
        avgFieldBuilder.setExpr(ExprFactory.EXPR("IF", ExprFactory.EXPR("EQ", ExprFactory.FIELD(alias + "_count"), ExprFactory.CONST("0", DataType.INT32)), 
            ExprFactory.CONST("0", DataType.INT32), ExprFactory.EXPR("DIVIDE", ExprFactory.FIELD(alias + "_sum"), ExprFactory.FIELD(alias + "_count"))));

        reduceFields.add(avgFieldBuilder.build());

        reduceTrans.addReduceField(alias + "_sum", "SUM", aggrDesc.getDistinct());
        reduceTrans.addReduceField(alias + "_count", "COUNT", aggrDesc.getDistinct());
      } else {
        DataType dataType = getDataType(node);
        
        FieldNamePair.Builder exprBuilder = FieldNamePair.newBuilder();
        exprBuilder.setFieldAlias(alias);
        exprBuilder.setFieldType(dataType);
        exprBuilder.setExpr(ExprFactory.FIELD(alias));
        FieldNamePair reduceField = exprBuilder.build();
        
        valueFields.add(expr);
        reduceFields.add(reduceField);
        filterFields.add(reduceField);
        reduceTrans.addReduceField(alias, reduceType, aggrDesc.getDistinct());
      }
    }
    
    op.setOutputValueFields(valueFields);

    // key type of reduce transformer must be "idgs.pb.Integer"
    reduceTrans.setKeyType("idgs.pb.Integer");
    reduceTrans.setOutputValueFields(reduceFields);
    
    reduceTrans.addChildOperator(op);
    if (hasAvg) {
      // build filter transformer to calculate average by sum / count
      TransformerOperator filter = new FilterTransformerOperator();
      filter.setOutputValueFields(filterFields);
      filter.addChildOperator(reduceTrans);
      return filter;
    } else {
      return reduceTrans;
    }
  }
  
  /**
   * Through column node and aggregation, generate reduce transformer about this column
   * 
   * @return ReduceTransformerOperator
   *         ReduceTransformerOperator[GroupTransformerOperator]
   *         FilterTransfomerOperator[ReduceTransformerOperator]
   *         FilterTransfomerOperator[ReduceTransformerOperator[GroupTransformerOperator]]
   * @throws IdgsParseException
   */
  private IdgsOperator genDistinctFieldOperator(ExprNodeDesc col, int index, IdgsOperator op, List<List<Integer>> distinctIndices, ArrayList<AggregationDesc> aggrs) throws IdgsParseException {
    GroupTransformerOperator grpTrans = new GroupTransformerOperator();
    ReduceTransformerOperator reduceTrans = new ReduceTransformerOperator();

    FieldNamePair keyField = genFieldExpr(col, "reducesink_key");
    
    // build value template with column.
    Map<String, FieldNamePair.Builder> fieldMap = new HashMap<String, FieldNamePair.Builder>();
    
    int diIndex = 0;
    for (AggregationDesc aggrDesc : aggrs) {
      if (aggrDesc.getDistinct()) {
        if (distinctIndices.get(diIndex).get(0) == index) {
          ExprNodeDesc colNode = aggrDesc.getParameters().get(0);
          if (colNode instanceof ExprNodeColumnDesc) {
            fieldMap.put(((ExprNodeColumnDesc) colNode).getColumn(), keyField.toBuilder());
          }
        }
        
        ++ diIndex;
      }
    }
    
    // fields of group operator
    List<FieldNamePair> grpFields = new ArrayList<FieldNamePair>();
    // fields of reduce operator
    List<FieldNamePair> reduceFields = new ArrayList<FieldNamePair>();
    // fields of filter operator
    List<FieldNamePair> filterFields = new ArrayList<FieldNamePair>();
    
    boolean hasAVG = false;
    for (int i = 0; i < aggrs.size(); ++ i) {
      AggregationDesc aggrDesc = aggrs.get(i);
      ExprNodeDesc colNode = aggrDesc.getParameters().get(0);
      if (colNode instanceof ExprNodeColumnDesc) {
        String colName = ((ExprNodeColumnDesc) colNode).getColumn();
        if (fieldMap.containsKey(colName)) {
          // get field template of current column
          FieldNamePair.Builder fldBuilder = fieldMap.get(colName);
          String fieldAlias = "_col" + i;
          String reduceType = aggrDesc.getGenericUDAFName().toUpperCase();
          
          // if current aggregation is average, make the column to sum and count column
          // else fldBuilder is value field expression
          if (reduceType.equals("AVG")) {
            hasAVG = true;
            FieldNamePair.Builder countBuilder = fldBuilder.clone();
            countBuilder.setFieldAlias(fieldAlias + "_count");
            countBuilder.setFieldType(fldBuilder.getFieldType());
            grpFields.add(countBuilder.build());

            FieldNamePair.Builder sumBuilder = fldBuilder.clone();
            sumBuilder.setFieldAlias(fieldAlias + "_sum");
            sumBuilder.setFieldType(fldBuilder.getFieldType());
            grpFields.add(sumBuilder.build());

            FieldNamePair.Builder avgFieldBuilder = FieldNamePair.newBuilder();
            avgFieldBuilder.setFieldAlias(fieldAlias);
            avgFieldBuilder.setFieldType(DataType.DOUBLE);
            avgFieldBuilder.setExpr(ExprFactory.EXPR("IF", 
                ExprFactory.EXPR("EQ", ExprFactory.FIELD(fieldAlias + "_count"), ExprFactory.CONST("0", DataType.INT32)), 
                ExprFactory.CONST("0", DataType.INT32), 
                ExprFactory.EXPR("DIVIDE", ExprFactory.FIELD(fieldAlias + "_sum"), ExprFactory.FIELD(fieldAlias + "_count"))));

            reduceFields.add(avgFieldBuilder.build());
            
            FieldNamePair.Builder filterFieldBuilder = FieldNamePair.newBuilder();
            filterFieldBuilder.setFieldAlias(fieldAlias);
            filterFieldBuilder.setFieldType(DataType.DOUBLE);
            filterFieldBuilder.setExpr(ExprFactory.FIELD(fieldAlias));
            filterFields.add(filterFieldBuilder.build());
            
            reduceTrans.addReduceField(fieldAlias + "_sum", "SUM", aggrDesc.getDistinct());
            reduceTrans.addReduceField(fieldAlias + "_count", "COUNT", aggrDesc.getDistinct());
          } else {
            FieldNamePair.Builder grpBuilder = fldBuilder.clone();
            grpBuilder.setFieldAlias(fieldAlias);
            grpFields.add(grpBuilder.build());
            
            FieldNamePair.Builder reduceBuilder = fldBuilder.clone();
            reduceBuilder.setFieldAlias(fieldAlias);
            reduceBuilder.getExprBuilder().setValue(fieldAlias);
            reduceFields.add(reduceBuilder.build());
            
            FieldNamePair.Builder reduceByKeyBuilder = fldBuilder.clone();
            reduceByKeyBuilder.setFieldAlias(fieldAlias);
            reduceByKeyBuilder.getExprBuilder().setValue(fieldAlias);
            FieldNamePair reduceByKeyField = reduceByKeyBuilder.build();
            filterFields.add(reduceByKeyField);

            reduceTrans.addReduceField(fieldAlias, reduceType, aggrDesc.getDistinct());
          }
        }
      }
    }

    reduceTrans.setKeyType("idgs.pb.Integer");
    reduceTrans.setOutputValueFields(reduceFields);
    
    if (isReuseKey(op, col)) {
      // column is primary key, don't use group transformer
      op.setOutputValueFields(grpFields);
      reduceTrans.addChildOperator(op);
    } else {
      // column is not primary key, use group transformer
      List<FieldNamePair> keyFields = new ArrayList<FieldNamePair>();
      keyFields.add(keyField);
      op.setOutputKeyFields(keyFields);
      
      grpTrans.setOutputValueFields(grpFields);
      grpTrans.addChildOperator(op);
      reduceTrans.addChildOperator(grpTrans);
    }
    
    if (hasAVG) {
      // aggregation has average, use filter transformer to calculate average = sum / count
      TransformerOperator filter = new FilterTransformerOperator();
      filter.setOutputValueFields(filterFields);
      filter.addChildOperator(reduceTrans);
      return filter;
    } else {
      return reduceTrans;
    }
  }
  
  /**
   * parse group operator with not group
   * @return if distinct column size = 1 and no value aggregation
   *           ReduceTransformerOperator
   *           ReduceTransformerOperator[GroupTransformerOperator]
   *           FilterTransfomerOperator[ReduceTransformerOperator]
   *           FilterTransfomerOperator[ReduceTransformerOperator[GroupTransformerOperator]]
   *         else
   *           HashjoinTransformerOperator[ReduceTransformerOperator[GroupTransformerOperator], ..., ReduceTransformerOperator, ...]
   * @throws IdgsParseException 
   */
  private IdgsOperator genGroupByPlanWithDistinct(Operator<? extends OperatorDesc> operator, IdgsOperator op) throws IdgsParseException {
    GroupByOperator groupOp = (GroupByOperator) operator;
    ReduceSinkOperator reduceOp = (ReduceSinkOperator) operator.getParentOperators().get(0);
    ArrayList<ExprNodeDesc> keyCols = reduceOp.getConf().getKeyCols();
    ArrayList<ExprNodeDesc> valueCols = reduceOp.getConf().getValueCols();
    ArrayList<AggregationDesc> aggrs = groupOp.getConf().getAggregators();
    List<List<Integer>> distinctIndices = reduceOp.getConf().getDistinctColumnIndices();
    
    IdgsOperator reduceTransOp = genDistinctFieldOperator(keyCols.get(0), 0, op.clone(), distinctIndices, aggrs);
    
    // build more reduce transformer, and join them
    List<HashJoinTransformerOperator> joinTransList = new ArrayList<HashJoinTransformerOperator>();
    for (int i = 1; i < keyCols.size(); ++ i) {
      ExprNodeDesc col = keyCols.get(i);
      HashJoinTransformerOperator joinTrans = new HashJoinTransformerOperator();
      joinTrans.setJoinType(JoinType.INNER_JOIN);
      
      if (joinTransList.isEmpty()) {
        joinTrans.addChildOperator(reduceTransOp);
      } else {
        joinTrans.addChildOperator(joinTransList.get(joinTransList.size() - 1));
      }
      joinTrans.addChildOperator(genDistinctFieldOperator(col, i, op.clone(), distinctIndices, aggrs));
      
      List<FieldNamePair> joinValueField = new ArrayList<FieldNamePair>();
      for (IdgsOperator joinChildOp : joinTrans.getChildrenOperators()) {
        for (FieldNamePair field : joinChildOp.getOutputValueFields()) {
          joinValueField.add(field);
        }
      }
      
      joinTrans.setOutputValueFields(joinValueField);
      joinTransList.add(joinTrans);
    }
    
    if (!joinTransList.isEmpty()) {
      reduceTransOp = joinTransList.get(joinTransList.size() - 1);
    }

    // build reduce transformer with none distinct field
    if (valueCols.isEmpty()) {
      genFilterAndSelectExpr(operator, reduceTransOp);
      return reduceTransOp;
    } else {
      IdgsOperator valueReduceTransOp = genGroupByPlanWithNoKey(operator, op.clone());
      
      HashJoinTransformerOperator valueJoinTrans = new HashJoinTransformerOperator();
      valueJoinTrans.setJoinType(JoinType.INNER_JOIN);

      valueJoinTrans.addChildOperator(reduceTransOp);
      valueJoinTrans.addChildOperator(valueReduceTransOp);
      
      List<FieldNamePair> joinValueField = new ArrayList<FieldNamePair>();
      for (IdgsOperator joinChildOp : valueJoinTrans.getChildrenOperators()) {
        for (FieldNamePair field : joinChildOp.getOutputValueFields()) {
          joinValueField.add(field);
        }
      }
      
      valueJoinTrans.setOutputValueFields(joinValueField);
      
      return valueJoinTrans;
    }
  }
  
  /**
   * parse group operator with column which is key
   * @return ReduceByKeyTransformerOperator
   * @throws IdgsParseException 
   */
  private IdgsOperator genGroupByPlanReuseKey(Operator<? extends OperatorDesc> operator, IdgsOperator op) throws IdgsParseException {
    GroupByOperator groupOp = (GroupByOperator) operator;
    ArrayList<ExprNodeDesc> grpKeyCols = groupOp.getConf().getKeys();
    
    ReduceSinkOperator reduceOp = (ReduceSinkOperator) operator.getParentOperators().get(0);
    ArrayList<ExprNodeDesc> keyCols = reduceOp.getConf().getKeyCols();
    
    // build alias mapping
    Map<String, String> mapping = new HashMap<String, String>();
    List<FieldNamePair> outputValueFields = op.getOutputValueFields();
    if (outputValueFields != null) {
      for (FieldNamePair field : outputValueFields) {
        if (field.getExpr().getName().equals("FIELD")) {
          mapping.put(field.getFieldAlias(), field.getExpr().getValue());
        }
      }
    }

    // add key fields
    List<FieldNamePair> keyFields = new ArrayList<FieldNamePair>();
    int index = 0;
    for (int i = 0; i < grpKeyCols.size(); ++ i, ++ index) {
      ExprNodeDesc colNode = keyCols.get(i);
      String alias = "_col" + index;
      keyFields.add(genFieldExpr(colNode, alias, mapping));
    }

    // add value fields from key column
    List<FieldNamePair> valueFields = new ArrayList<FieldNamePair>();
    for (int i = index; i < keyCols.size(); ++ i, ++ index) {
      ExprNodeDesc colNode = keyCols.get(i);
      String alias = "_col" + index;
      valueFields.add(genFieldExpr(colNode, alias, mapping));
    }
    
    // add value fields from value column
    ArrayList<ExprNodeDesc> valueCols = reduceOp.getConf().getValueCols();
    for (int i = 0; i < valueCols.size(); ++ i, ++ index) {
      ExprNodeDesc colNode = valueCols.get(i);
      String alias = "_col" + index;
      valueFields.add(genFieldExpr(colNode, alias, mapping));
    }

    // update child operator
    op.setOutputKeyFields(keyFields);
    
    // build ReduceByKeyTransformer
    ReduceByKeyTransformerOperator reduceByKeyTrans = new ReduceByKeyTransformerOperator();
    ArrayList<AggregationDesc> aggr = groupOp.getConf().getAggregators();
    if (aggr.isEmpty()) {
      op.setOutputValueFields(valueFields);
    } else {
      Map<String, Integer> schemaMap = new HashMap<String, Integer>();
      ArrayList<ColumnInfo> schemaList = reduceOp.getSchema().getSignature();
      for (int i = 0; i < schemaList.size(); ++ i) {
        ColumnInfo colInfo = schemaList.get(i);
        schemaMap.put(colInfo.getInternalName(), i);
      }
      

      List<FieldNamePair> allFields = new ArrayList<FieldNamePair>();
      allFields.addAll(keyFields);
      allFields.addAll(valueFields);
      
      List<FieldNamePair> reduceFields = new ArrayList<FieldNamePair>();
      List<List<Integer>> distinctIndices = reduceOp.getConf().getDistinctColumnIndices();
      int aggrIndex = keyFields.size(), distinctIndex = 0;
      for (int i = 0; i < aggr.size(); ++ i) {
        String alias = "_col" + (aggrIndex ++);
        AggregationDesc aggrDesc = aggr.get(i);
        ExprNodeDesc node = aggrDesc.getParameters().get(0);
        DataType dataType = getDataType(node);
        
        if (node instanceof ExprNodeColumnDesc) {
          FieldNamePair expr = null;
          if (aggrDesc.getDistinct()) {
            expr = allFields.get(distinctIndices.get(distinctIndex ++).get(0));
          } else {
            expr = allFields.get(schemaMap.get(((ExprNodeColumnDesc) node).getColumn()));
          }
          
          Builder exprBuilder = expr.toBuilder().clone();
          exprBuilder.setFieldAlias(alias);
          reduceFields.add(exprBuilder.build());
        } else if (node instanceof ExprNodeConstantDesc) {
          FieldNamePair.Builder fieldBuilder = FieldNamePair.newBuilder();
          fieldBuilder.setFieldAlias(alias);
          fieldBuilder.setFieldType(dataType);
          fieldBuilder.setExpr(ExprFactory.CONST(((ExprNodeConstantDesc) node).getValue().toString(), dataType));
          reduceFields.add(fieldBuilder.build());
        } else {
          FieldNamePair.Builder fieldBuilder = FieldNamePair.newBuilder();
          fieldBuilder.setFieldAlias(alias);
          fieldBuilder.setFieldType(dataType);
          fieldBuilder.setExpr(ExprFactory.FIELD(alias));
          reduceFields.add(fieldBuilder.build());
        }
        
        reduceByKeyTrans.addReduceField(alias, aggrDesc.getGenericUDAFName(), aggrDesc.getDistinct());   
      }
      
      op.setOutputValueFields(reduceFields);
    }
    
    reduceByKeyTrans.addChildOperator(op);
    
    return reduceByKeyTrans;    
  }
  
  /**
   * parse group operator with column witch is not key
   * @return ReduceByKeyTransformerOperator[GroupTransformerOperator]
   * @throws IdgsParseException 
   */
  private IdgsOperator genGroupByPlan(Operator<? extends OperatorDesc> operator, IdgsOperator op) throws IdgsParseException {
    GroupByOperator groupOp = (GroupByOperator) operator;
    ArrayList<ExprNodeDesc> grpKeyCols = groupOp.getConf().getKeys();
    
    ReduceSinkOperator reduceOp = (ReduceSinkOperator) operator.getParentOperators().get(0);
    ArrayList<ExprNodeDesc> keyCols = reduceOp.getConf().getKeyCols();
    
    // build alias mapping
    Map<String, String> mapping = new HashMap<String, String>();
    List<FieldNamePair> outputValueFields = op.getOutputValueFields();
    if (outputValueFields != null) {
      for (FieldNamePair field : outputValueFields) {
        if (field.getExpr().getName().equals("FIELD")) {
          mapping.put(field.getFieldAlias(), field.getExpr().getValue());
        }
      }
    }
    
    Map<String, ExprNodeConstantDesc> constKeyMap = new HashMap<String, ExprNodeConstantDesc>();
    Operator<? extends OperatorDesc> pOp = reduceOp.getParentOperators().get(0);
    if (pOp instanceof SelectOperator) {
      SelectOperator selOp = (SelectOperator) pOp;
      List<ExprNodeDesc> colList = selOp.getConf().getColList();
      List<String> colNames = selOp.getConf().getOutputColumnNames();
      for (int i = 0; i < colList.size(); ++ i) {
        ExprNodeDesc node = colList.get(i);
        if (node instanceof ExprNodeConstantDesc) {
          constKeyMap.put(colNames.get(i), (ExprNodeConstantDesc) node);
        }
      }
    }
    
    // add key fields
    List<FieldNamePair> keyFields = new ArrayList<FieldNamePair>();
    int index = 0;
    for (int i = 0; i < grpKeyCols.size(); ++ i, ++ index) {
      String alias = "_col" + index;
      ExprNodeDesc colNode = keyCols.get(i);
      if (colNode instanceof ExprNodeColumnDesc) {
        String colName = ((ExprNodeColumnDesc) colNode).getColumn();
        if (constKeyMap.containsKey(colName)) {
          ExprNodeConstantDesc constNode = constKeyMap.get(colName);
          DataType dataType = getDataType(constNode);
          FieldNamePair.Builder fieldBuilder = FieldNamePair.newBuilder();
          fieldBuilder.setFieldAlias(alias);
          fieldBuilder.setFieldType(dataType);
          fieldBuilder.setExpr(ExprFactory.CONST(constNode.getValue().toString(), dataType));
          keyFields.add(fieldBuilder.build());
          
          continue;
        }
      }
      keyFields.add(genFieldExpr(colNode, alias, mapping));
    }

    // add value fields from key column
    List<FieldNamePair> valueFields = new ArrayList<FieldNamePair>();
    for (int i = index; i < keyCols.size(); ++ i, ++ index) {
      ExprNodeDesc colNode = keyCols.get(i);
      String alias = "_col" + index;
      valueFields.add(genFieldExpr(colNode, alias, mapping));
    }
    
    // add value fields from value column
    ArrayList<ExprNodeDesc> valueCols = reduceOp.getConf().getValueCols();
    for (int i = 0; i < valueCols.size(); ++ i, ++ index) {
      ExprNodeDesc colNode = valueCols.get(i);
      String alias = "_col" + index;
      valueFields.add(genFieldExpr(colNode, alias, mapping));
    }
    
    op.setOutputKeyFields(keyFields);
    
    // build ReduceByKeyTransformer
    TransformerOperator groupTrans = new GroupTransformerOperator();
    ReduceByKeyTransformerOperator reduceByKeyTrans = new ReduceByKeyTransformerOperator();
    
    ArrayList<AggregationDesc> aggr = groupOp.getConf().getAggregators();
    if (aggr.isEmpty()) {
      op.setOutputValueFields(valueFields);
    } else {
      Map<String, Integer> schemaMap = new HashMap<String, Integer>();
      ArrayList<ColumnInfo> schemaList = reduceOp.getSchema().getSignature();
      for (int i = 0; i < schemaList.size(); ++ i) {
        ColumnInfo colInfo = schemaList.get(i);
        schemaMap.put(colInfo.getInternalName(), i);
      }

      List<FieldNamePair> allFields = new ArrayList<FieldNamePair>();
      allFields.addAll(keyFields);
      allFields.addAll(valueFields);
      
      List<FieldNamePair> opFields = new ArrayList<FieldNamePair>();
      List<FieldNamePair> grpFields = new ArrayList<FieldNamePair>();
      List<List<Integer>> distinctIndices = reduceOp.getConf().getDistinctColumnIndices();
      int aggrIndex = keyFields.size(), distinctIndex = 0;
      for (int i = 0; i < aggr.size(); ++ i) {
        String alias = "_col" + (aggrIndex ++);
        AggregationDesc aggrDesc = aggr.get(i);
        ExprNodeDesc node = aggrDesc.getParameters().get(0);
        DataType dataType = getDataType(node);
        
        if (node instanceof ExprNodeColumnDesc) {
          FieldNamePair expr = null;
          if (aggrDesc.getDistinct()) {
            expr = allFields.get(distinctIndices.get(distinctIndex ++).get(0));
          } else {
            expr = allFields.get(schemaMap.get(((ExprNodeColumnDesc) node).getColumn()));
          }
          
          Builder exprBuilder = expr.toBuilder().clone();
          exprBuilder.setFieldAlias(alias);
          opFields.add(exprBuilder.build());
        } else if (node instanceof ExprNodeConstantDesc) {
          FieldNamePair.Builder fieldBuilder = FieldNamePair.newBuilder();
          fieldBuilder.setFieldAlias(alias);
          fieldBuilder.setFieldType(dataType);
          fieldBuilder.setExpr(ExprFactory.CONST(((ExprNodeConstantDesc) node).getValue().toString(), dataType));
          opFields.add(fieldBuilder.build());
        } else {
          FieldNamePair.Builder fieldBuilder = FieldNamePair.newBuilder();
          fieldBuilder.setFieldAlias(alias);
          fieldBuilder.setFieldType(dataType);
          fieldBuilder.setExpr(ExprFactory.FIELD(alias));
          opFields.add(fieldBuilder.build());
        }
        
        reduceByKeyTrans.addReduceField(alias, aggrDesc.getGenericUDAFName(), aggrDesc.getDistinct());
        
        FieldNamePair.Builder grpBuilder = FieldNamePair.newBuilder();
        grpBuilder.setFieldAlias(alias);
        grpBuilder.setFieldType(dataType);
        grpBuilder.setExpr(ExprFactory.FIELD(alias));
        grpFields.add(grpBuilder.build());
      }
      
      op.setOutputValueFields(opFields);
      groupTrans.setOutputValueFields(grpFields);
    }
    
    groupTrans.addChildOperator(op);
    reduceByKeyTrans.addChildOperator(groupTrans);

    return reduceByKeyTrans;
  }
  
  /**
   * parse union operator
   * @return UnionTransformerOperator[TransformerOperator ...]
   * @throws IdgsParseException 
   */
  private IdgsOperator genUnionOperator(Operator<? extends OperatorDesc> operator, List<IdgsOperator> children) throws IdgsParseException {
    UnionTransformerOperator unionTrans = new UnionTransformerOperator();
    
    genFilterAndSelectExpr(operator, unionTrans);
    unionTrans.setChildrenOperators(children);
    
    return unionTrans;
  }
  
  /**
   * parse union operator
   * @return JoinTransformerOperator[TransformerOperator, TransformerOperator]
   * @throws IdgsParseException 
   */
  private IdgsOperator genJoinPlan(Operator<? extends OperatorDesc> operator, List<IdgsOperator> children) throws IdgsParseException {
    JoinOperator joinOp = (JoinOperator) operator;
    HashJoinTransformerOperator joinTrans = new HashJoinTransformerOperator();
    genFilterAndSelectExpr(joinOp, joinTrans);
    
    // build join operator list, if right join, change to left join
    List<Operator<? extends OperatorDesc>> joinOpList = null;
    if (joinOp.getConf().getConds()[0].getType() == JoinDesc.RIGHT_OUTER_JOIN) {
      joinOpList = new ArrayList<Operator<? extends OperatorDesc>>();
      for (int i = operator.getParentOperators().size() - 1; i >= 0; -- i) {
        joinOpList.add(operator.getParentOperators().get(i));
      }
    } else {
      joinOpList = operator.getParentOperators();
    }
    
    // parse join type
    switch (joinOp.getConf().getConds()[0].getType()) {
      case JoinDesc.INNER_JOIN:
        joinTrans.setJoinType(JoinType.INNER_JOIN);
        break;
      case JoinDesc.LEFT_OUTER_JOIN:
        joinTrans.setJoinType(JoinType.LEFT_JOIN);
        break;
      case JoinDesc.RIGHT_OUTER_JOIN:
        joinTrans.setJoinType(JoinType.LEFT_JOIN);
        break;
      case JoinDesc.FULL_OUTER_JOIN:
        joinTrans.setJoinType(JoinType.OUTER_JOIN);
        break;
      case JoinDesc.UNIQUE_JOIN:
        joinTrans.setJoinType(JoinType.INNER_JOIN);
        break;
      case JoinDesc.LEFT_SEMI_JOIN:
        joinTrans.setJoinType(JoinType.INNER_JOIN);
        break;
    }
    
    int valueIndex = 0;
    List<String> outputValueNames = joinOp.getConf().getOutputColumnNames();
    Map<Byte, List<ExprNodeDesc>> exprMap = joinOp.getConf().getExprs();
    
    for (int i = 0; i < joinOpList.size(); ++ i) {
      Operator<? extends OperatorDesc> op = joinOpList.get(i);
      ReduceSinkOperator reduceOp = (ReduceSinkOperator) op;
      ArrayList<ExprNodeDesc> keyCols = reduceOp.getConf().getKeyCols();
      
      ArrayList<String> outputKeyName = reduceOp.getConf().getOutputKeyColumnNames();
      List<FieldNamePair> keyFields = new ArrayList<FieldNamePair>();
      ArrayList<ExprNodeDesc> valueCols = reduceOp.getConf().getValueCols();
      List<FieldNamePair> valueFields = new ArrayList<FieldNamePair>();

      IdgsOperator chop = children.get(i);
      Map<String, String> mapping = new HashMap<String, String>();
      List<FieldNamePair> outputValueFields = chop.getOutputValueFields();
      if (outputValueFields != null) {
        for (FieldNamePair field : outputValueFields) {
          if (field.getExpr().getName().equals("FIELD")) {
            mapping.put(field.getFieldAlias(), field.getExpr().getValue()); 
          }
        }
      }
      
      List<ExprNodeDesc> nodeList = exprMap.get((byte) i);
      if (isReuseKey(chop, keyCols)) {
        for (int j = 0; j < keyCols.size(); ++ j) {
          ExprNodeDesc colNode = keyCols.get(j);
          String alias = outputKeyName.get(j);
          keyFields.add(genFieldExpr(colNode, alias, mapping));
        }
        
        int kIndex = 0, vIndex = 0;
        for (int j = 0; j < nodeList.size(); ++ j) {
          ExprNodeDesc node = nodeList.get(j);
          String alias = outputValueNames.get(valueIndex ++);
          if (node instanceof ExprNodeColumnDesc) {
            ExprNodeColumnDesc colNode = (ExprNodeColumnDesc) node;
            String colName = colNode.getColumn();
            if (colName.startsWith("KEY.")) {
              valueFields.add(genFieldExpr(keyCols.get(kIndex ++), alias, mapping));
            } else if (colName.startsWith("VALUE.")) {
              valueFields.add(genFieldExpr(valueCols.get(vIndex ++), alias, mapping));
            } else {
              LOG.error("error when gen join transformer, caused by column name " + colName + "is not in key or value fields.");
            }
          } else {
            LOG.error("error when gen join transformer, caused by expr node is not column node, but " + node.getName() + "(" + node.getExprString() + ")");
          }
        }
        
        chop.setOutputKeyFields(keyFields);
        chop.setOutputValueFields(valueFields);
        
        joinTrans.addChildOperator(chop);
      } else {
        List<FieldNamePair> grpKeyFields = new ArrayList<FieldNamePair>();
        for (int j = 0; j < keyCols.size(); ++ j) {
          ExprNodeDesc node = keyCols.get(j);
          DataType dataType = getDataType(node);
          
          String alias = outputKeyName.get(j);
          keyFields.add(genFieldExpr(node, alias, mapping));
          
          FieldNamePair.Builder fldBuilder = FieldNamePair.newBuilder();
          fldBuilder.setFieldAlias(alias);
          fldBuilder.setFieldType(dataType);
          fldBuilder.getExprBuilder().setName(ExprFactory.FIELD);
          fldBuilder.getExprBuilder().setValue(alias);
          grpKeyFields.add(fldBuilder.build());
        }
        
        List<FieldNamePair> grpValueFields = new ArrayList<FieldNamePair>();
        
        int kIndex = 0, vIndex = 0;
        for (int j = 0; j < nodeList.size(); ++ j) {
          ExprNodeDesc node = nodeList.get(j);
          DataType dataType = getDataType(node);
          
          String alias = outputValueNames.get(valueIndex ++);
          if (node instanceof ExprNodeColumnDesc) {
            ExprNodeColumnDesc colNode = (ExprNodeColumnDesc) node;
            String colName = colNode.getColumn();
            if (colName.startsWith("KEY.")) {
              valueFields.add(genFieldExpr(keyCols.get(kIndex ++), alias, mapping));
            } else if (colName.startsWith("VALUE.")) {
              valueFields.add(genFieldExpr(valueCols.get(vIndex ++), alias, mapping));
            } else {
              LOG.error("error when gen join transformer, caused by column name " + colName + "is not in key or value fields.");
            }
          } else {
            LOG.error("error when gen join transformer, caused by expr node is not column node, but " + node.getName() + "(" + node.getExprString() + ")");
          }
          
          FieldNamePair.Builder fldBuilder = FieldNamePair.newBuilder();
          fldBuilder.setFieldAlias(alias);
          fldBuilder.setFieldType(dataType);
          fldBuilder.getExprBuilder().setName(ExprFactory.FIELD);
          fldBuilder.getExprBuilder().setValue(alias);
          grpValueFields.add(fldBuilder.build());
        }
  
        chop.setOutputKeyFields(keyFields);
        chop.setOutputValueFields(valueFields);
        
        GroupTransformerOperator grpTrans = new GroupTransformerOperator();
        grpTrans.setOutputKeyFields(grpKeyFields);
        grpTrans.setOutputValueFields(grpValueFields);

        grpTrans.addChildOperator(chop);
        joinTrans.addChildOperator(grpTrans);
      }
    }
    
    return joinTrans;
  }
  
  /**
   * parse extract operator
   * @return FilterTransformerOperator[TransformerOperator]
   * @throws IdgsParseException 
   */
  private IdgsOperator genExtractPlan(Operator<? extends OperatorDesc> operator, List<IdgsOperator> children) {
    ExtractOperator extractOp = (ExtractOperator) operator;

    Map<String, String> fieldNames = new HashMap<String, String>();
    for (ColumnInfo c : extractOp.getSchema().getSignature()) {
      if (c.getAlias() != null) {
        fieldNames.put(c.getInternalName(), c.getAlias());
      }
    }
    
    IdgsOperator op = children.get(0);
    if (op.getOutputKeyFields() != null) {
      for (int i = 0; i < op.getOutputKeyFields().size(); ++ i) {
        FieldNamePair field = op.getOutputKeyFields().get(i);
        String internal = field.getFieldAlias();
        if (fieldNames.containsKey(internal)) {
          FieldNamePair.Builder builder = field.toBuilder();
          builder.setFieldAlias(fieldNames.get(internal));
          field = builder.build();
          op.getOutputKeyFields().set(i, field);
        }
      }
    }
    
    if (op.getOutputValueFields() != null) {
      for (int i = 0; i < op.getOutputValueFields().size(); ++ i) {
        FieldNamePair field = op.getOutputValueFields().get(i);
        String internal = field.getFieldAlias();
        if (fieldNames.containsKey(internal)) {
          FieldNamePair.Builder builder = field.toBuilder();
          builder.setFieldAlias(fieldNames.get(internal));
          field = builder.build();
          op.getOutputValueFields().set(i, field);
        }
      }
    }
    
    IdgsOperator filterTrans = new FilterTransformerOperator();
    filterTrans.setChildrenOperators(children);
    return filterTrans;
  }

  private void genFilterAndSelectExpr(Operator<? extends OperatorDesc> operator, IdgsOperator op) throws IdgsParseException {
    Operator<? extends OperatorDesc> childOp = operator.getChildOperators().get(0);
    if (childOp instanceof FilterOperator) {
      FilterOperator filterOp = (FilterOperator) childOp;
      op.setFilterExpr(genFilterExpr(filterOp));
      Operator<? extends OperatorDesc> filterChild = filterOp.getChildOperators().get(0);
      if (filterChild instanceof SelectOperator) {
        SelectOperator selectOp = (SelectOperator) filterChild;
        op.setOutputValueFields(genSelectExpr(selectOp));
      }
    } else if (childOp instanceof SelectOperator) {
      SelectOperator selectOp = (SelectOperator) childOp;
      op.setOutputValueFields(genSelectExpr(selectOp));
    }
  }
  
  private Expr genFilterExpr(FilterOperator filterOp) throws IdgsParseException {
    ExprNodeDesc node = filterOp.getConf().getPredicate();
    return ExprFactory.buildExpression(node);
  }
  
  private List<FieldNamePair> genSelectExpr(SelectOperator selectOp) throws IdgsParseException {
    List<ExprNodeDesc> colList = selectOp.getConf().getColList();
    if (colList.isEmpty()) {
      return null;
    }
    
    List<ColumnInfo> colInfoList = selectOp.getSchema().getSignature();
    List<FieldNamePair> fieldList = new ArrayList<FieldNamePair>();
    for (int i = 0; i < colList.size(); ++ i) {
      ExprNodeDesc node = colList.get(i);
      ColumnInfo colInfo = colInfoList.get(i);
      
      fieldList.add(genFieldExpr(node, (colInfo.getAlias() == null) ? colInfo.getInternalName() : colInfo.getAlias()));
    }
    
    return fieldList;
  }
  
  private FieldNamePair genFieldExpr(ExprNodeDesc colNode, String alias) throws IdgsParseException {
    DataType dataType = getDataType(colNode);
    
    FieldNamePair.Builder selBuilder = FieldNamePair.newBuilder();
    selBuilder.setFieldAlias(alias);
    selBuilder.setFieldType(dataType);
    selBuilder.setExpr(ExprFactory.buildExpression(colNode));
    return selBuilder.build();
  }
  
  private FieldNamePair genFieldExpr(ExprNodeDesc colNode, String alias, Map<String, String> mapping) throws IdgsParseException {
    DataType dataType = getDataType(colNode);
    
    FieldNamePair.Builder selBuilder = FieldNamePair.newBuilder();
    selBuilder.setFieldAlias(alias);
    selBuilder.setFieldType(dataType);
    selBuilder.setExpr(ExprFactory.buildExpression(colNode, mapping));
    return selBuilder.build();
  }
  
  private String toHiveOpString(Operator<? extends OperatorDesc> operator, String tab) {
    StringBuffer sb = new StringBuffer();
    sb.append(tab).append(operator.getName()).append("\n");
    
    if (operator.getParentOperators() != null) {
      for (Operator<? extends OperatorDesc> op : operator.getParentOperators()) {
        sb.append(toHiveOpString(op, "  " + tab));
      }
    }
    
    return sb.toString();
  }
  
  private boolean isReuseKey(IdgsOperator operator, ExprNodeDesc key) {
    List<ExprNodeDesc> keys = new ArrayList<ExprNodeDesc>();
    keys.add(key);
    return isReuseKey(operator, keys);
  }
  
  private boolean isReuseKey(IdgsOperator operator, List<ExprNodeDesc> keys) {
    if (operator instanceof DelegateOperator) {
      String keyType = ((DelegateOperator) operator).getKeyType();
      Descriptor descriptor = MessageHelper.getMessageDescriptor(keyType);
      if (descriptor.getFields().size() != keys.size()) {
        return false;
      } else {
        for (int i = 0; i < descriptor.getFields().size(); ++ i) {
          FieldDescriptor keyField = descriptor.getFields().get(i);
          ExprNodeDesc node = keys.get(i);
          if (node instanceof ExprNodeColumnDesc) {
            String column = ((ExprNodeColumnDesc) node).getColumn();
            if (!keyField.getName().equals(column)) {
              return false;
            }
          } else {
            return false;
          }
        }
        return true;
      }
    } else {
      List<FieldNamePair> keyFields = operator.getOutputKeyFields();
      if (keyFields == null) {
        if (operator.getChildrenOperators().size() != 1) {
          return false;
        } else {
          return isReuseKey(operator.getChildrenOperators().get(0), keys);
        }
      } else {
        if (keyFields.size() != keys.size()) {
          return false;
        } else {
          for (int i = 0; i < keyFields.size(); ++ i) {
            FieldNamePair keyField = keyFields.get(i);
            ExprNodeDesc node = keys.get(i);
            if (node instanceof ExprNodeColumnDesc) {
              String column = ((ExprNodeColumnDesc) node).getColumn();
              if (!keyField.getFieldAlias().equals(column)) {
                return false;
              }
            } else {
              return false;
            }
          }
          
          return true;
        }
      }
    }
  }
  
  private DataType getDataType(ExprNodeDesc node) throws IdgsParseException {
    String type = node.getTypeInfo().getTypeName();
    DataType dataType = TypeUtils.hiveToDataType(type);
    if (dataType == null) {
      throw new IdgsParseException("data type " + type + " of " + node + " is not supported.");
    }
    
    return dataType;
  }
  
}