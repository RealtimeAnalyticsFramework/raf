
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.client.util;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.google.protobuf.Message;
import com.googlecode.protobuf.format.JsonFormat;
import com.googlecode.protobuf.format.JsonFormat.ParseException;

public class JsonUtil {
  
  private static Log log = LogFactory.getLog(JsonUtil.class);

  /**
   * convert json string to protobuf
   * 
   * @param builder
   * @param jsonString
   * @throws ParseException
   */
  public static void jsonToPb(Message.Builder builder, String jsonString) throws ParseException {
    JsonFormat.merge(jsonString, builder);
  }
  
  /**
   * read json file content to StringBuilder
   * @param file
   * @return
   * @throws IOException
   */
  public static StringBuilder getJsonFromFile(File file) throws IOException {
    BufferedReader br = null;
    StringBuilder builder = new StringBuilder();
    String line = null;
    FileInputStream fs = new FileInputStream(file);
    try {
      br = new BufferedReader(new InputStreamReader(fs));
      while ((line = br.readLine()) != null) {
        builder.append(line).append("\r\n");
      }
    } catch (IOException e) {
      log.error(e.getMessage(), e);
      throw e;
    } finally {
      if (br != null) {
        try {
          br.close();
          fs.close();
        } catch (IOException e) {
          // do nothing
        }
      }
    }
    return builder;
  }
  
  /**
   * read json file content to StringBuilder
   * @param fileName
   * @return
   * @throws IOException
   */
  public static StringBuilder getJsonFromFile(String fileName) throws IOException {
    return getJsonFromFile(new File(fileName));
  }

  /**
   * json file to protobuf
   * 
   * @param builder
   * @param file
   * @throws ParseException
   * @throws FileNotFoundException
   */
  public static void jsonFileToPb(Message.Builder builder, File file) throws ParseException, IOException {
    jsonToPb(builder, getJsonFromFile(file).toString());
  }

  /**
   * json file to protobuf
   * 
   * @param builder
   * @param fileName
   * @throws ParseException
   * @throws FileNotFoundException
   */
  public static void jsonFileToPb(Message.Builder builder, String fileName) throws ParseException, IOException {
    jsonFileToPb(builder, new File(fileName));
  }

  /**
   * protobuf to json
   * 
   * @param msg
   * @return
   */
  public static String pbToJson(final Message.Builder builder) {
    return JsonFormat.printToString(builder.build());
  }

}
