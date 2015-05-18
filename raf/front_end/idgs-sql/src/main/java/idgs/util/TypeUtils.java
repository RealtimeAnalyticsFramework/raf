/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.util;

import java.util.HashMap;
import java.util.Map;

import com.google.protobuf.Descriptors.FieldDescriptor.JavaType;

import idgs.pb.PbExpr.DataType;

public class TypeUtils {
  
  private static Map<String, DataType> hiveToDataType;
  
  private static Map<JavaType, String> javaToHiveType;
  
  private static Map<DataType, String> dataTypeToProto;
  
  private static Map<JavaType, DataType> javaTypeToDataType;
  
  static {
    hiveToDataType = new HashMap<String, DataType>();
    hiveToDataType.put("boolean", DataType.BOOL);
    hiveToDataType.put("string", DataType.STRING);
    hiveToDataType.put("double", DataType.DOUBLE);
    hiveToDataType.put("tinyint", DataType.INT32);
    hiveToDataType.put("smallint", DataType.INT32);
    hiveToDataType.put("int", DataType.INT32);
    hiveToDataType.put("float", DataType.FLOAT);
    hiveToDataType.put("bigint", DataType.INT64);
    hiveToDataType.put("binary", DataType.STRING);
    hiveToDataType.put("timestamp", DataType.STRING);
    hiveToDataType.put("date", DataType.STRING);
    hiveToDataType.put("decimal", DataType.DOUBLE);
    
    javaToHiveType = new HashMap<JavaType, String>();
    javaToHiveType.put(JavaType.BOOLEAN, "boolean");
    javaToHiveType.put(JavaType.BYTE_STRING, "string");
    javaToHiveType.put(JavaType.DOUBLE, "double");
    javaToHiveType.put(JavaType.ENUM, "int");
    javaToHiveType.put(JavaType.FLOAT, "float");
    javaToHiveType.put(JavaType.INT, "int");
    javaToHiveType.put(JavaType.LONG, "bigint");
    javaToHiveType.put(JavaType.STRING, "string");
    
    dataTypeToProto = new HashMap<DataType, String>();
    dataTypeToProto.put(DataType.BOOL, "bool");
    dataTypeToProto.put(DataType.STRING, "string");
    dataTypeToProto.put(DataType.DOUBLE, "double");
    dataTypeToProto.put(DataType.INT32, "int32");
    dataTypeToProto.put(DataType.ENUM, "enum");
    dataTypeToProto.put(DataType.UINT32, "uint32");
    dataTypeToProto.put(DataType.FLOAT, "float");
    dataTypeToProto.put(DataType.INT64, "int64");
    dataTypeToProto.put(DataType.UINT64, "uint64");
    dataTypeToProto.put(DataType.BYTES, "bytes");
    
    javaTypeToDataType = new HashMap<JavaType, DataType>();
    javaTypeToDataType.put(JavaType.BOOLEAN, DataType.BOOL);
    javaTypeToDataType.put(JavaType.BYTE_STRING, DataType.BYTES);
    javaTypeToDataType.put(JavaType.DOUBLE, DataType.DOUBLE);
    javaTypeToDataType.put(JavaType.ENUM, DataType.ENUM);
    javaTypeToDataType.put(JavaType.FLOAT, DataType.FLOAT);
    javaTypeToDataType.put(JavaType.INT, DataType.INT32);
    javaTypeToDataType.put(JavaType.LONG, DataType.INT64);
    javaTypeToDataType.put(JavaType.STRING, DataType.STRING);
  }
  
  public static DataType hiveToDataType(String hiveType) {
    int pos = hiveType.indexOf("(");
    String type = (pos == -1) ? hiveType : hiveType.substring(0, pos);
    return hiveToDataType.get(type);
  }
  
  public static String javaTypeToHive(JavaType javaType) {
    return javaToHiveType.get(javaType);
  }
  
  public static String dataTypeToProto(DataType dataType) {
    return dataTypeToProto.get(dataType);
  }
  
  public static DataType javaTypeToDataType(JavaType javaType) {
    return javaTypeToDataType.get(javaType);
  }
  
}
