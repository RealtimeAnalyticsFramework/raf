/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.execution;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import idgs.client.ClientActorMessage;
import idgs.exception.IdgsException;
import idgs.rdd.pb.PbRddCommon.RddResultCode;
import idgs.rdd.pb.PbRddInternal.RddResponse;
import idgs.rdd.pb.PbRddService.DestroyRddRequest;
import idgs.util.ServerConst;

public class DestroyOperator extends IdgsOperator {

  private static Log LOG = LogFactory.getLog(DestroyOperator.class);
  
  @Override
  protected String getName() {
    return ServerConst.DESTROY;
  }

  @Override
  protected ClientActorMessage buildRequest() throws IdgsException {
    DestroyRddRequest.Builder builder = DestroyRddRequest.newBuilder();
    IdgsOperator op = getChildrenOperators().get(0);
    if (op instanceof ActionOperator) {
      op = op.getChildrenOperators().get(0);
    }
    builder.setRddName(op.getRddName());
    setRddName(op.getRddName());
    
    DestroyRddRequest request = builder.build();
    
    ClientActorMessage requestMsg = buildRddRequestMessage(ServerConst.RDD_DESTROY, request);
    if (LOG.isDebugEnabled()) {
      LOG.debug(ServerConst.RDD_DESTROY + " ===> " + requestMsg.toString());
    }
    
    return requestMsg;
  }

  @Override
  protected void processResponse(ClientActorMessage responseMsg) throws IdgsException {
    LOG.debug("Got destroy response for RDD " + getRddName() + ".");
    RddResponse.Builder responseBuilder = RddResponse.newBuilder();
    if (!responseMsg.parsePayload(responseBuilder)) {
      String err = "cannot parse payload of response for destroy of RDD " + getRddName();
      LOG.error(err);
      throw new IdgsException(err);
    }
    
    RddResponse response = responseBuilder.build();
    if (response.getResultCode() != RddResultCode.RRC_SUCCESS) {
      String err = "execute destory RDD " + getRddName() + " error, caused by " + response.getResultCode().toString();
      LOG.error(err);
      throw new IdgsException(err);
    }
  }

}
