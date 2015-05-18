/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package protobuf;

import com.google.protobuf.DynamicMessage;

import idgs.exception.IdgsException;
import protubuf.MessageHelper;
import junit.framework.TestCase;

public class MessageHelperUT extends TestCase {

  private String testProtoFile = null;

  public MessageHelperUT() {
    String home = System.getenv("IDGS_HOME");
    if (home == null) {
      testProtoFile = this.getClass().getResource("conf/tpch.proto").getPath();
    } else {
      testProtoFile = home + "/conf/tpch.proto";
    }
  }

  public void testRegisterMessage() {
    try {
      MessageHelper.registerMessage(testProtoFile);
    } catch (IdgsException e) {
      e.printStackTrace();
      fail(e.getMessage());
    }
  }

  public void testIsMessageRegistered() {
    try {
      MessageHelper.registerMessage(testProtoFile);
    } catch (IdgsException e) {
      e.printStackTrace();
    }

    String typeName = "idgs.sample.tpch.pb.Customer";
    boolean isReg = MessageHelper.isMessageRegistered(typeName);
    assertEquals(true, isReg);

    typeName = "idgs.sample.tpch.pb.CustomerTest";
    isReg = MessageHelper.isMessageRegistered(typeName);
    assertEquals(false, isReg);
  }

  public void testCreateMessage() {
    try {
      MessageHelper.registerMessage(testProtoFile);
    } catch (IdgsException e) {
      e.printStackTrace();
    }

    String typeName = "idgs.sample.tpch.pb.Customer";
    DynamicMessage message = MessageHelper.createMessage(typeName);
    assertNotNull(message);

    typeName = "idgs.sample.tpch.pb.CustomerTest";
    message = MessageHelper.createMessage(typeName);
    assertNull(message);
  }

}
