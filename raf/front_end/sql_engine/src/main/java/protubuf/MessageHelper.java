/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package protubuf;

import idgs.exception.IdgsException;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.google.protobuf.DescriptorProtos.FileDescriptorProto;
import com.google.protobuf.DescriptorProtos.FileDescriptorSet;
import com.google.protobuf.Descriptors.Descriptor;
import com.google.protobuf.Descriptors.DescriptorValidationException;
import com.google.protobuf.Descriptors.FileDescriptor;
import com.google.protobuf.DynamicMessage;

public class MessageHelper {

  private static Log log = LogFactory.getLog(MessageHelper.class);
  
  private static Map<String, Descriptor> cache;
  
  static {
    cache = new HashMap<String, Descriptor>();
  }
  
  public static void registerMessage(String protoFile) throws IdgsException {
    String source = new File(protoFile).getParent();
    String cmd = "protoc -I=" + source + " --descriptor_set_out=" + protoFile + ".desc " + protoFile;
    log.info(cmd);
    
    Runtime run = Runtime.getRuntime();
    Process p = null;
    try {
      p = run.exec(cmd);
    } catch (IOException e) {
      log.error("create desc file of proto file " + protoFile + " error");
      throw new IdgsException(e);
    }
    
    try {
      if (p.waitFor() != 0) {
        if (p.exitValue() == 1) {
          log.error("create desc file of proto file " + protoFile + " error.");
          throw new IdgsException("create desc file of proto file " + protoFile + " error.");
        }
      }
    } catch (InterruptedException e) {
      log.error("create desc file of proto file " + protoFile + " error.");
      throw new IdgsException(e);
    }
    
    File file = new File(protoFile + ".desc");
    FileInputStream fin = null;
    try {
      log.info("register file " + protoFile);
      fin = new FileInputStream(file);
      FileDescriptorSet descriptorSet = FileDescriptorSet.parseFrom(fin);
      for (FileDescriptorProto fdp : descriptorSet.getFileList()) {
        FileDescriptor fd = FileDescriptor.buildFrom(fdp, new FileDescriptor[] {});
        for (Descriptor descriptor : fd.getMessageTypes()) {
          log.info("register message " + descriptor.getFullName());
          cache.put(descriptor.getFullName(), descriptor);
        }
      }
    } catch (FileNotFoundException e) {
      log.error("desc file of proto file " + protoFile + " is not found.");
      throw new IdgsException(e);
    } catch (IOException e) {
      log.error("parse desc file of proto file " + protoFile + " error");
      throw new IdgsException(e);
    } catch (DescriptorValidationException e) {
      log.error("register proto file " + protoFile + " error");
      throw new IdgsException(e);
    } finally {
      if (fin != null) {
        try {
          fin.close();
        } catch (IOException e) {
          throw new IdgsException(e);
        }
      }
    }
    
    file.delete();
  }
  
  public static void unRegisterMessage(String typeName) throws IdgsException {
    cache.remove(typeName);
  }
  
  public static DynamicMessage.Builder createMessageBuilder(String typeName) {
    Descriptor descriptor = cache.get(typeName);
    if (descriptor == null) {
      return null;
    } else {
      return DynamicMessage.newBuilder(descriptor);
    }
  }
  
  public static DynamicMessage createMessage(String typeName) {
    Descriptor descriptor = cache.get(typeName);
    if (descriptor == null) {
      return null;
    } else {
      return DynamicMessage.getDefaultInstance(descriptor);
    }
  }
  
  public static Descriptor getMessageDescriptor(String typeName) {
    return cache.get(typeName);
  }
  
  public static boolean isMessageRegistered(String typeName) {
    return cache.containsKey(typeName);
  }
  
}
