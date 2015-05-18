
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.client.util;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.google.protobuf.Message;
import com.google.protobuf.TextFormat;

public class TextSerdeImpl implements ProtoSerde {

  private static Log log = LogFactory.getLog(TextSerdeImpl.class);

  public String serialize(Message msg) {
    return TextFormat.printToString(msg);
  }

  public void deserialize(Message.Builder builder, String str) {
    try {
      TextFormat.merge(str, builder);
    } catch (com.google.protobuf.TextFormat.ParseException e) {
      log.error(e.getMessage());
    }
  }

  public byte[] serializeToByteArray(Message msg) {
    return serialize(msg).getBytes();
  }

  public void deserializeFromByteArray(Message.Builder builder, byte[] array) {
    try {
      TextFormat.merge(new String(array), builder);
    } catch (com.google.protobuf.TextFormat.ParseException e) {
      log.error(e.getMessage());
    }
  }
}
