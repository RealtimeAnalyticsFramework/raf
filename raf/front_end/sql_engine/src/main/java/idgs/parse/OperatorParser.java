/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.parse;

import java.util.ArrayList;
import java.util.HashMap;
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
import org.apache.hadoop.hive.ql.parse.ParseContext;
import org.apache.hadoop.hive.ql.plan.AggregationDesc;
import org.apache.hadoop.hive.ql.plan.ExprNodeColumnDesc;
import org.apache.hadoop.hive.ql.plan.ExprNodeDesc;
import org.apache.hadoop.hive.ql.plan.JoinDesc;
import org.apache.hadoop.hive.ql.plan.OperatorDesc;

import protubuf.MessageHelper;

import com.google.protobuf.Descriptors.Descriptor;
import com.google.protobuf.Descriptors.FieldDescriptor;

import idgs.execution.ActionOperator;
import idgs.execution.CollectActionOperator;
import idgs.execution.DelegateOperator;
import idgs.execution.FilterTransformerOperator;
import idgs.execution.GroupTransformOperator;
import idgs.execution.HashJoinTransformerOperator;
import idgs.execution.IdgsOperator;
import idgs.execution.ReduceByKeyTransformerOperator;
import idgs.execution.ReduceTransformerOperator;
import idgs.execution.TopNActionOperator;
import idgs.execution.TransformerOperator;
import idgs.execution.UnionTransformerOperator;
import idgs.metadata.StoreMetadata;
import idgs.pb.PbExpr.Expr;
import idgs.pb.PbExpr.ExpressionType;
import idgs.rdd.pb.PbRddService.FieldNamePair;
import idgs.rdd.pb.PbRddTransform.JoinType;
import idgs.util.TypeUtils;

public class OperatorParser {
  
  private static Log LOG = LogFactory.getLog(OperatorParser.class);
  
  private ParseContext pctx;
  
  private FileSinkOperator sinkOp;
  
  public OperatorParser(ParseContext pctx, FileSinkOperator sinkOp) {
    this.pctx = pctx;
    this.sinkOp = sinkOp;
    if (LOG.isDebugEnabled()) {
      LOG.debug(toHiveOpString(sinkOp, ""));
    }
  }
  
  public ActionOperator parseOperator() {
    if (sinkOp.getParentOperators() == null || sinkOp.getParentOperators().isEmpty()) {
      return null;
    }
    
    ActionOperator topOp = genActionOperator();
    
    topOp.setChildrenOperators(parseOperator(sinkOp));
    
    if (topOp.getChildrenOperators() == null || topOp.getChildrenOperators().size() != 1) {
      return null;
    }
    
    IdgsOperator operator = topOp.getChildrenOperators().get(0);
    if (operator.getOutputKeyFields() != null || operator.getOutputValueFields() != null) {
      topOp.clearOperator();
      IdgsOperator filterTrans = new FilterTransformerOperator();
      filterTrans.addChildOperator(operator);
      topOp.addChildOperator(filterTrans);
    }
    
    if (LOG.isDebugEnabled()) {
      LOG.debug(topOp.toTreeString());
    }
    
    return topOp;
  }
  
  private ActionOperator genActionOperator() {
    Operator<? extends OperatorDesc> op = sinkOp.getParentOperators().get(0);
    if (op instanceof ExtractOperator) {
      ExtractOperator extractOp = (ExtractOperator) op;
      Map<String, String> fieldSchemas = new HashMap<String, String>();
      for (ColumnInfo colInfo : extractOp.getSchema().getSignature()) {
        fieldSchemas.put(colInfo.getInternalName(), colInfo.getAlias());
      }

      ReduceSinkOperator reduceOp = (ReduceSinkOperator) extractOp.getParentOperators().get(0);
      TopNActionOperator topOp = new TopNActionOperator();
      String order = reduceOp.getConf().getOrder();
      List<ExprNodeDesc> keyCols = reduceOp.getConf().getKeyCols();
      List<String> keyOutputNames = reduceOp.getConf().getOutputKeyColumnNames();
          
      for (int i = 0; i < keyCols.size(); ++ i) {
        ExprNodeDesc node = keyCols.get(i);
        FieldNamePair.Builder builder = FieldNamePair.newBuilder();
        builder.setFieldAlias(keyOutputNames.get(i));
        builder.setFieldType(TypeUtils.hiveToDataType(node.getTypeInfo().getTypeName()));
        builder.setExpr(ExprFactory.buildExpression(node, fieldSchemas));
        FieldNamePair field = builder.build();
        
        boolean desc = false;
        if (i < order.length()) {
          desc = (order.charAt(i) == '-');
        }
        topOp.addOrderField(field, desc);
      }
      
      return topOp;
    } else {
      return new CollectActionOperator();
    }
  }
  
  private List<IdgsOperator> parseOperator(Operator<? extends OperatorDesc> operator) {
    List<IdgsOperator> children = null;
    if (operator.getParentOperators() != null) {
      children = new ArrayList<IdgsOperator>();
      for (Operator<? extends OperatorDesc> parentOp : operator.getParentOperators()) {
        children.addAll(parseOperator(parentOp));
      }
    }
    
    List<IdgsOperator> ret = null;
    IdgsOperator oper = null;
    if (operator instanceof TableScanOperator) {
      oper = genTablePlan(operator);
    } else if (operator instanceof GroupByOperator) {
      oper = genGroupByPlan(operator, children);
    } else if (operator instanceof UnionOperator) {
      oper = genUnionOperator(operator, children);
    } else if (operator instanceof JoinOperator) {
      oper = genJoinPlan(operator, children);
    } else if (operator instanceof ExtractOperator) {
      if (operator.getChildOperators().size() == 1 && operator.getChildOperators().get(0) instanceof FileSinkOperator) {
        oper = genExtractPlan(operator, children);
      } else {
        return children;
      }
    } else if (operator instanceof LimitOperator) {
    } else {
      return children;
    }
    
    ret = new ArrayList<IdgsOperator>();
    ret.add(oper);
    
    return ret;
  }
  
  private IdgsOperator genTablePlan(Operator<? extends OperatorDesc> operator) {
    TableScanOperator tabOp = (TableScanOperator) operator;
    Table tbl = pctx.getTopToTable().get(tabOp);
    String tableName = tbl.getTableName();
    String storeName = StoreMetadata.getInstance().getStoreName(tableName);
    
    IdgsOperator delegate = new DelegateOperator(storeName);
    genFilterAndSelectExpr(operator, delegate);
    
    return delegate;
  }
  
  private IdgsOperator genReducePlan(Operator<? extends OperatorDesc> operator, IdgsOperator op) {
    ReduceSinkOperator reduceOp = (ReduceSinkOperator) operator.getParentOperators().get(0);
    ArrayList<ExprNodeDesc> valueCols = reduceOp.getConf().getValueCols();
    List<FieldNamePair> valueFields = new ArrayList<FieldNamePair>();
    List<FieldNamePair> reduceFields = new ArrayList<FieldNamePair>();
    for (int i = 0; i < valueCols.size(); ++ i) {
      ExprNodeDesc colNode = valueCols.get(i);
      String alias = "_col" + i;
      valueFields.add(genFieldExpr(colNode, alias));
      FieldNamePair.Builder fldBuilder = FieldNamePair.newBuilder();
      fldBuilder.setFieldAlias(alias);
      fldBuilder.setFieldType(TypeUtils.hiveToDataType(colNode.getTypeInfo().getTypeName()));
      fldBuilder.getExprBuilder().setType(ExpressionType.FIELD);
      fldBuilder.getExprBuilder().setValue(alias);
      reduceFields.add(fldBuilder.build());
    }

    op.setOutputValueFields(valueFields);
    
    ReduceTransformerOperator reduceTrans = new ReduceTransformerOperator();
    ReduceByKeyTransformerOperator reduceByKeyTrans = new ReduceByKeyTransformerOperator();

    GroupByOperator groupOp = (GroupByOperator) operator;
    ArrayList<AggregationDesc> aggr = groupOp.getConf().getAggregators();
    for (int i = 0; i < aggr.size(); ++ i) {
      AggregationDesc aggrDesc = aggr.get(i);
      String fieldName = "_col" + i;
      String reduceType = aggrDesc.getGenericUDAFName().toUpperCase();
      String reduceByKeyType = aggrDesc.getGenericUDAFName().toUpperCase();
      if (reduceType.equals("COUNT")) {
        reduceByKeyType = "SUM";
      }
      
      if (reduceType.equals("AVG")) {
        
      }
      
      reduceTrans.addReduceField(fieldName, reduceType, aggrDesc.getDistinct());
      reduceByKeyTrans.addReduceField(fieldName, reduceByKeyType, aggrDesc.getDistinct());
    }

    reduceTrans.setKeyType("idgs.pb.Integer");
    reduceTrans.setOutputValueFields(reduceFields);
    genFilterAndSelectExpr(groupOp, reduceByKeyTrans);
    
    reduceTrans.addChildOperator(op);
    reduceByKeyTrans.addChildOperator(reduceTrans);
    
    return reduceByKeyTrans;
  }
  
  private IdgsOperator genReduceWithOutGroup(Operator<? extends OperatorDesc> operator, IdgsOperator op) {
    ReduceSinkOperator reduceOp = (ReduceSinkOperator) operator.getParentOperators().get(0);
    ArrayList<ExprNodeDesc> keyCols = reduceOp.getConf().getKeyCols();
    
    Map<String, String> mapping = new HashMap<String, String>();
    for (FieldNamePair field : op.getOutputValueFields()) {
      mapping.put(field.getFieldAlias(), field.getExpr().getValue());
    }

    int index = 0;
    List<FieldNamePair> keyFields = new ArrayList<FieldNamePair>();
    for (int i = 0; i < keyCols.size(); ++ i) {
      keyFields.add(genFieldExpr(keyCols.get(i), "_col" + (index ++), mapping));
    }
    
    int valueIndex = index;
    ArrayList<ExprNodeDesc> valueCols = reduceOp.getConf().getValueCols();
    List<FieldNamePair> valueFields = new ArrayList<FieldNamePair>();
    List<FieldNamePair> grpFields = new ArrayList<FieldNamePair>();
    for (ExprNodeDesc colNode : valueCols) {
      String alias = "_col" + (index ++);
      valueFields.add(genFieldExpr(colNode, alias, mapping));
      FieldNamePair.Builder fldBuilder = FieldNamePair.newBuilder();
      fldBuilder.setFieldAlias(alias);
      fldBuilder.setFieldType(TypeUtils.hiveToDataType(colNode.getTypeInfo().getTypeName()));
      fldBuilder.getExprBuilder().setType(ExpressionType.FIELD);
      fldBuilder.getExprBuilder().setValue(alias);
      grpFields.add(fldBuilder.build());
    }
    
    op.setOutputKeyFields(keyFields);
    op.setOutputValueFields(valueFields);

    ReduceByKeyTransformerOperator reduceByKeyTrans = new ReduceByKeyTransformerOperator();
    
    GroupByOperator groupOp = (GroupByOperator) operator;
    ArrayList<AggregationDesc> aggr = groupOp.getConf().getAggregators();
    for (int i = 0; i < aggr.size(); ++ i) {
      AggregationDesc aggrDesc = aggr.get(i);
      String fieldName = "_col" + (valueIndex ++);
      reduceByKeyTrans.addReduceField(fieldName, aggrDesc.getGenericUDAFName(), aggrDesc.getDistinct());   
    }
    
    genFilterAndSelectExpr(groupOp, reduceByKeyTrans);
    
    reduceByKeyTrans.addChildOperator(op);
    
    return reduceByKeyTrans;
  }
  
  private IdgsOperator genReduceWithGroup(Operator<? extends OperatorDesc> operator, IdgsOperator op) {
    ReduceSinkOperator reduceOp = (ReduceSinkOperator) operator.getParentOperators().get(0);
    ArrayList<ExprNodeDesc> keyCols = reduceOp.getConf().getKeyCols();
    
    Map<String, String> mapping = new HashMap<String, String>();
    for (FieldNamePair field : op.getOutputValueFields()) {
      mapping.put(field.getFieldAlias(), field.getExpr().getValue());
    }

    int index = 0;
    List<FieldNamePair> keyFields = new ArrayList<FieldNamePair>();
    for (int i = 0; i < keyCols.size(); ++ i) {
      keyFields.add(genFieldExpr(keyCols.get(i), "_col" + (index ++), mapping));
    }
    
    int valueIndex = index;
    ArrayList<ExprNodeDesc> valueCols = reduceOp.getConf().getValueCols();
    List<FieldNamePair> valueFields = new ArrayList<FieldNamePair>();
    List<FieldNamePair> grpFields = new ArrayList<FieldNamePair>();
    for (ExprNodeDesc colNode : valueCols) {
      String alias = "_col" + (index ++);
      valueFields.add(genFieldExpr(colNode, alias, mapping));
      FieldNamePair.Builder fldBuilder = FieldNamePair.newBuilder();
      fldBuilder.setFieldAlias(alias);
      fldBuilder.setFieldType(TypeUtils.hiveToDataType(colNode.getTypeInfo().getTypeName()));
      fldBuilder.getExprBuilder().setType(ExpressionType.FIELD);
      fldBuilder.getExprBuilder().setValue(alias);
      grpFields.add(fldBuilder.build());
    }
    
    op.setOutputKeyFields(keyFields);
    op.setOutputValueFields(valueFields);

    ReduceByKeyTransformerOperator reduceByKeyTrans = new ReduceByKeyTransformerOperator();
    TransformerOperator groupTrans = new GroupTransformOperator();
    groupTrans.setOutputValueFields(grpFields);
    
    GroupByOperator groupOp = (GroupByOperator) operator;
    ArrayList<AggregationDesc> aggr = groupOp.getConf().getAggregators();
    for (int i = 0; i < aggr.size(); ++ i) {
      AggregationDesc aggrDesc = aggr.get(i);
      String fieldName = "_col" + (valueIndex ++);
      reduceByKeyTrans.addReduceField(fieldName, aggrDesc.getGenericUDAFName(), aggrDesc.getDistinct());   
    }
    
    genFilterAndSelectExpr(groupOp, reduceByKeyTrans);
    
    reduceByKeyTrans.addChildOperator(groupTrans);
    groupTrans.addChildOperator(op);
    
    return reduceByKeyTrans;
  }
  
  private IdgsOperator genGroupByPlan(Operator<? extends OperatorDesc> operator, List<IdgsOperator> children) {
    IdgsOperator op = children.get(0);
    ReduceSinkOperator reduceOp = (ReduceSinkOperator) operator.getParentOperators().get(0);
    ArrayList<ExprNodeDesc> keyCols = reduceOp.getConf().getKeyCols();
    
    if (keyCols == null || keyCols.isEmpty()) {
      return genReducePlan(operator, op);
    } else if (isReuseKey(op, keyCols)) {
      return genReduceWithOutGroup(operator, op);
    } else {
      return genReduceWithGroup(operator, op);
    }
  }
  
  private IdgsOperator genUnionOperator(Operator<? extends OperatorDesc> operator, List<IdgsOperator> children) {
    UnionTransformerOperator unionTrans = new UnionTransformerOperator();
    
    genFilterAndSelectExpr(operator, unionTrans);
    unionTrans.setChildrenOperators(children);
    
    return unionTrans;
  }
  
  private IdgsOperator genJoinPlan(Operator<? extends OperatorDesc> operator, List<IdgsOperator> children) {
    JoinOperator joinOp = (JoinOperator) operator;
    HashJoinTransformerOperator joinTrans = new HashJoinTransformerOperator();
    genFilterAndSelectExpr(joinOp, joinTrans);
    
    List<Operator<? extends OperatorDesc>> joinOpList = null;
    if (joinOp.getConf().getConds()[0].getType() == JoinDesc.RIGHT_OUTER_JOIN) {
      joinOpList = new ArrayList<Operator<? extends OperatorDesc>>();
      for (int i = operator.getParentOperators().size() - 1; i >= 0; -- i) {
        joinOpList.add(operator.getParentOperators().get(i));
      }
    } else {
      joinOpList = operator.getParentOperators();
    }
    
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
    for (int i = 0; i < joinOpList.size(); ++ i) {
      Operator<? extends OperatorDesc> op = joinOpList.get(i);
      ReduceSinkOperator reduceOp = (ReduceSinkOperator) op;
      ArrayList<ExprNodeDesc> keyCols = reduceOp.getConf().getKeyCols();
      
      IdgsOperator chop = children.get(i);
      Map<String, String> mapping = new HashMap<String, String>();
      if (chop.getOutputValueFields() != null) {
        for (FieldNamePair field : chop.getOutputValueFields()) {
          mapping.put(field.getFieldAlias(), field.getExpr().getValue());
        }
      }
      
      ArrayList<String> outputKeyName = reduceOp.getConf().getOutputKeyColumnNames();
      List<FieldNamePair> keyFields = new ArrayList<FieldNamePair>();
      ArrayList<ExprNodeDesc> valueCols = reduceOp.getConf().getValueCols();
      List<FieldNamePair> valueFields = new ArrayList<FieldNamePair>();
      
      if (isReuseKey(chop, keyCols)) {
        for (int j = 0; j < keyCols.size(); ++ j) {
          ExprNodeDesc colNode = keyCols.get(j);
          String alias = outputKeyName.get(j);
          keyFields.add(genFieldExpr(colNode, alias, mapping));
          
          joinTrans.setJoinField(i, j, outputKeyName.get(j));
        }
        
        for (ExprNodeDesc colNode : valueCols) {
          String alias = outputValueNames.get(valueIndex ++);
          valueFields.add(genFieldExpr(colNode, alias, mapping));
        }
        
        chop.setOutputKeyFields(keyFields);
        chop.setOutputValueFields(valueFields);
        
        joinTrans.addChildOperator(chop);
      } else {
        List<FieldNamePair> grpKeyFields = new ArrayList<FieldNamePair>();
        for (int j = 0; j < keyCols.size(); ++ j) {
          ExprNodeDesc colNode = keyCols.get(j);
          String alias = outputKeyName.get(j);
          keyFields.add(genFieldExpr(colNode, alias, mapping));
          
          joinTrans.setJoinField(i, j, alias);
          
          FieldNamePair.Builder fldBuilder = FieldNamePair.newBuilder();
          fldBuilder.setFieldAlias(alias);
          fldBuilder.setFieldType(TypeUtils.hiveToDataType(colNode.getTypeInfo().getTypeName()));
          fldBuilder.getExprBuilder().setType(ExpressionType.FIELD);
          fldBuilder.getExprBuilder().setValue(alias);
          grpKeyFields.add(fldBuilder.build());
        }
        
        List<FieldNamePair> grpValueFields = new ArrayList<FieldNamePair>();
        for (ExprNodeDesc colNode : valueCols) {
          String alias = outputValueNames.get(valueIndex ++);
          valueFields.add(genFieldExpr(colNode, alias, mapping));
          
          FieldNamePair.Builder fldBuilder = FieldNamePair.newBuilder();
          fldBuilder.setFieldAlias(alias);
          fldBuilder.setFieldType(TypeUtils.hiveToDataType(colNode.getTypeInfo().getTypeName()));
          fldBuilder.getExprBuilder().setType(ExpressionType.FIELD);
          fldBuilder.getExprBuilder().setValue(alias);
          grpValueFields.add(fldBuilder.build());
        }
  
        chop.setOutputKeyFields(keyFields);
        chop.setOutputValueFields(valueFields);
        
        GroupTransformOperator grpTrans = new GroupTransformOperator();
        grpTrans.setOutputKeyFields(grpKeyFields);
        grpTrans.setOutputValueFields(grpValueFields);
        grpTrans.addChildOperator(chop);
        
        joinTrans.addChildOperator(grpTrans);
      }
    }
    
    return joinTrans;
  }
  
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

  private void genFilterAndSelectExpr(Operator<? extends OperatorDesc> operator, IdgsOperator op) {
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
  
  private Expr genFilterExpr(FilterOperator filterOp) {
    ExprNodeDesc node = filterOp.getConf().getPredicate();
    return ExprFactory.buildExpression(node);
  }
  
  private List<FieldNamePair> genSelectExpr(SelectOperator selectOp) {
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
  
  private FieldNamePair genFieldExpr(ExprNodeDesc colNode, String alias) {
    FieldNamePair.Builder selBuilder = FieldNamePair.newBuilder();
    selBuilder.setFieldAlias(alias);
    selBuilder.setFieldType(TypeUtils.hiveToDataType(colNode.getTypeInfo().getTypeName()));
    selBuilder.setExpr(ExprFactory.buildExpression(colNode));
    return selBuilder.build();
  }
  
  private FieldNamePair genFieldExpr(ExprNodeDesc colNode, String alias, Map<String, String> mapping) {
    FieldNamePair.Builder selBuilder = FieldNamePair.newBuilder();
    selBuilder.setFieldAlias(alias);
    selBuilder.setFieldType(TypeUtils.hiveToDataType(colNode.getTypeInfo().getTypeName()));
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
  
  private boolean isReuseKey(IdgsOperator operator, ArrayList<ExprNodeDesc> keys) {
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
  
}