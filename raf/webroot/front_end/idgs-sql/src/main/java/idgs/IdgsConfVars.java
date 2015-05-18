/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs;

import java.util.HashMap;
import java.util.Map;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.hive.conf.HiveConf;

class ConfVar {

  public String varname;

  public Class<?> valClass;

  public String defaultVal;

  public Integer defaultIntVal;

  public Long defaultLongVal;

  public Float defaultFloatVal;

  public Boolean defaultBoolVal;
  
  public Object defaultObject;

  public ConfVar(String varname, Class<?> valClass, String defaultVal,
      Integer defaultIntVal, Long defaultLongVal, Float defaultFloatVal,
      Boolean defaultBoolVal, Object defaultObject) {
    this.varname = varname;
    this.valClass = valClass;
    this.defaultVal = defaultVal;
    this.defaultIntVal = defaultIntVal;
    this.defaultLongVal = defaultLongVal;
    this.defaultFloatVal = defaultFloatVal;
    this.defaultBoolVal = defaultBoolVal;
    this.defaultObject = defaultObject;
  }

  public ConfVar(String varname, String defaultVal) {
    this(varname, String.class, defaultVal, 0, 0L, 0F, false, null);
  }

  public ConfVar(String varname, Integer defaultVal) {
    this(varname, Integer.class, null, defaultVal, 0L, 0F, false, null);
  }

  public ConfVar(String varname, Long defaultVal) {
    this(varname, Long.class, null, 0, defaultVal, 0F, false, null);
  }

  public ConfVar(String varname, Float defaultVal) {
    this(varname, Float.class, null, 0, 0L, defaultVal, false, null);
  }

  public ConfVar(String varname, Boolean defaultVal) {
    this(varname, Boolean.class, null, 0, 0L, 0F, defaultVal, null);
  }
  
  public ConfVar(String varname, Object defaultVal) {
    this(varname, Object.class, null, 0, 0L, 0F, null, defaultVal);
  }

}

public class IdgsConfVars {
  
  private static Map<ConfVar, Object> confVar = new HashMap<ConfVar, Object>();

  public static ConfVar CLIPROMPT = new ConfVar("idgs.cli.prompt", "idgs");
  
  public static ConfVar PRINT_RESULT = new ConfVar("idgs.print.result", true);
  
  public static ConfVar IDGS_LOG4J = new ConfVar("idgs.log4j.file", "idgs-log4j.properties");
  
  public static ConfVar CLIENT_CONFIG_FILE = new ConfVar("idgs.client.config", "conf/client.conf");
  
  public static ConfVar CLIENT_PB_CONFIG = new ConfVar("idgs.client.pb.config", (Object) null);
  
  public static ConfVar STORE_CONFIG_FILE = new ConfVar("idgs.store.config", "conf/data_store.conf");
  
  public static ConfVar COLUMN_SEPERATOR = new ConfVar("idgs.column.seperator", new String(new byte[] {(byte) 1}));
  
  public static ConfVar LOAD_BATCH_SIZE = new ConfVar("idgs.load.batch.size", new Integer(50));
  
  public static ConfVar LOAD_THREAD_COUNT = new ConfVar("idgs.load.threadcount", new Integer(10));
  
  public static ConfVar EXEC_MODE = new ConfVar("idgs.exec.mode", "idgs");
  
  public static ConfVar STORE_LOADER_CLASS = new ConfVar("idgs.store.loader.class", "idgs.metadata.LocalStoreLoader");

  // This is created for testing. Hive's test script assumes a certain output
  // format. To pass the test scripts, we need to use Hive's EXPLAIN.
  public static ConfVar EXPLAIN_MODE = new ConfVar("idgs.explain.mode", "idgs");

  // If true, keys that are NULL are equal. For strict SQL standard, set this to
  // true.
  public static ConfVar JOIN_CHECK_NULL = new ConfVar("idgs.join.checknull", true);

  // Specify the initial capacity for ArrayLists used to represent columns in
  // columnar
  // cache. The default -1 for non-local mode means that Shark will try to
  // estimate
  // the number of rows by using: partition_size / (num_columns *
  // avg_field_size).
  public static ConfVar COLUMN_INITIALSIZE = new ConfVar("idgs.columnar.cache.initialSize", (System.getenv("MASTER") == null) ? 100 : -1);

  // Default storage level for cached tables.
  public static ConfVar STORAGE_LEVEL = new ConfVar("idgs.cache.storageLevel", "MEMORY_AND_DISK");

  // If true, then cache any table whose name ends in "_cached".
  public static ConfVar CHECK_TABLENAME_FLAG = new ConfVar("idgs.cache.flag.checkTableName", true);

  // Prune map splits for cached tables based on predicates in queries.
  public static ConfVar MAP_PRUNING = new ConfVar("idgs.mappruning", true);

  // Print debug information for map pruning.
  public static ConfVar MAP_PRUNING_PRINT_DEBUG = new ConfVar("idgs.mappruning.debug", false);

  // If true, then query plans are compressed before being sent
  public static ConfVar COMPRESS_QUERY_PLAN = new ConfVar("idgs.compressQueryPlan", true);

  public static void initializeWithDefaults(Configuration conf) {
    if (conf.get(CLIPROMPT.varname) == null) {
      conf.set(CLIPROMPT.varname, CLIPROMPT.defaultVal);
    }
    if (conf.get(EXEC_MODE.varname) == null) {
      conf.set(EXEC_MODE.varname, EXEC_MODE.defaultVal);
    }
    if (conf.get(EXPLAIN_MODE.varname) == null) {
      conf.set(EXPLAIN_MODE.varname, EXPLAIN_MODE.defaultVal);
    }
    if (conf.get(COLUMN_INITIALSIZE.varname) == null) {
      conf.setInt(COLUMN_INITIALSIZE.varname, COLUMN_INITIALSIZE.defaultIntVal);
    }
    if (conf.get(CHECK_TABLENAME_FLAG.varname) == null) {
      conf.setBoolean(CHECK_TABLENAME_FLAG.varname, CHECK_TABLENAME_FLAG.defaultBoolVal);
    }
    if (conf.get(COMPRESS_QUERY_PLAN.varname) == null) {
      conf.setBoolean(COMPRESS_QUERY_PLAN.varname, COMPRESS_QUERY_PLAN.defaultBoolVal);
    }
    if (conf.get(MAP_PRUNING.varname) == null) {
      conf.setBoolean(MAP_PRUNING.varname, MAP_PRUNING.defaultBoolVal);
    }
    if (conf.get(MAP_PRUNING_PRINT_DEBUG.varname) == null) {
      conf.setBoolean(MAP_PRUNING_PRINT_DEBUG.varname, MAP_PRUNING_PRINT_DEBUG.defaultBoolVal);
    }
  }

  private static void require(Boolean requirement) {
    if (!requirement) {
      throw new IllegalArgumentException("requirement failed");
    }
  }

  public static Integer getIntVar(Configuration conf, ConfVar variable) {
    require(variable.valClass == Integer.class);
    return conf.getInt(variable.varname, variable.defaultIntVal);
  }

  public static Long getLongVar(Configuration conf, ConfVar variable) {
    require(variable.valClass == Long.class);
    return conf.getLong(variable.varname, variable.defaultLongVal);
  }

  public static Float getFloatVar(Configuration conf, ConfVar variable) {
    require(variable.valClass == Float.class);
    return conf.getFloat(variable.varname, variable.defaultFloatVal);
  }

  public static Boolean getBoolVar(Configuration conf, ConfVar variable) {
    require(variable.valClass == Boolean.class);
    return conf.getBoolean(variable.varname, variable.defaultBoolVal);
  }

  public static String getVar(Configuration conf, ConfVar variable) {
    require(variable.valClass == String.class);
    return conf.get(variable.varname, variable.defaultVal);
  }
  
  public static Object getObjectVar(Configuration conf, ConfVar variable) {
    String className = conf.get(variable.varname);
    if (className != null) {
      require(variable.valClass.getName() == className);
    } else {
      return null;
    }
    
    if (confVar.containsKey(variable)) {
      return confVar.get(variable);
    } else {
      return variable.defaultObject;
    }
  }
  
  public static void setVar(Configuration conf, ConfVar variable, String value) {
    require(variable.valClass == String.class);
    conf.set(variable.varname, value);
  }
  
  public static void setBoolVar(Configuration conf, ConfVar variable, Boolean value) {
    require(variable.valClass == Boolean.class);
    conf.setBoolean(variable.varname, value);
  }
  
  public static void setObjectVar(Configuration conf, ConfVar variable, Object value) {
    conf.set(variable.varname, value.getClass().getName());
    confVar.put(variable, value);
  }

  public static Integer getIntVar(Configuration conf, HiveConf.ConfVars variable) {
    return HiveConf.getIntVar(conf, variable);
  }

  public static Long getLongVar(Configuration conf, HiveConf.ConfVars variable) {
    return HiveConf.getLongVar(conf, variable);
  }

  public static Long getLongVar(Configuration conf, HiveConf.ConfVars variable, Long defaultVal) {
    return HiveConf.getLongVar(conf, variable, defaultVal);
  }

  public static Float getFloatVar(Configuration conf, HiveConf.ConfVars variable) {
    return HiveConf.getFloatVar(conf, variable);
  }

  public static Float getFloatVar(Configuration conf, HiveConf.ConfVars variable, Float defaultVal) {
    return HiveConf.getFloatVar(conf, variable, defaultVal);
  }

  public static Boolean getBoolVar(Configuration conf, HiveConf.ConfVars variable) {
    return HiveConf.getBoolVar(conf, variable);
  }

  public static Boolean getBoolVar(Configuration conf, HiveConf.ConfVars variable, Boolean defaultVal) {
    return HiveConf.getBoolVar(conf, variable, defaultVal);
  }

  public static String getVar(Configuration conf, HiveConf.ConfVars variable) {
    return HiveConf.getVar(conf, variable);
  }

  public static String getVar(Configuration conf, HiveConf.ConfVars variable, String defaultVal) {
    return HiveConf.getVar(conf, variable, defaultVal);
  }
  
}
