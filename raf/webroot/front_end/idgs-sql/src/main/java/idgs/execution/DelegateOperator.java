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
import idgs.rdd.pb.PbRddService.CreateDelegateRddRequest;
import idgs.rdd.pb.PbRddService.CreateDelegateRddResponse;
import idgs.store.pb.PbStoreConfig.StoreConfig;
import idgs.util.ServerConst;

public class DelegateOperator extends IdgsOperator {

  private static Log LOG = LogFactory.getLog(DelegateOperator.class);
  
  private String schemaName;
  
  private String storeName;
  
  public DelegateOperator(String schemaName, StoreConfig cfg) {
    this.schemaName = schemaName;
    this.storeName = cfg.getName();
    keyType = cfg.getKeyType();
    valueType = cfg.getValueType();
  }
  
  protected String getName() {
    return ServerConst.DELEGATE;
  }
  
  @Override
  protected ClientActorMessage buildRequest() throws IdgsException {
    CreateDelegateRddRequest.Builder builder = CreateDelegateRddRequest.newBuilder();
    builder.setSchemaName(schemaName);
    builder.setStoreName(storeName);
    builder.setRddName(getRddName());
    
    CreateDelegateRddRequest request = builder.build();
    
    ClientActorMessage requestMsg = buildRddRequestMessage(ServerConst.CREATE_STORE_DELEGATE_RDD, request);
    if (LOG.isDebugEnabled()) {
      LOG.debug("Delegate " + request.getStoreName() + " ===> " + requestMsg.toString());
    }
    
    return requestMsg;
  }
  
  @Override
  protected void processResponse(ClientActorMessage responseMsg) throws IdgsException {
    CreateDelegateRddResponse.Builder builder = CreateDelegateRddResponse.newBuilder();
    responseMsg.parsePayload(builder);
    CreateDelegateRddResponse response = builder.build();
    
    if (response.getResultCode() != RddResultCode.RRC_SUCCESS) {
      throw new IdgsException("create store delegate error, caused by " + response.getResultCode().toString());
    }
  }

}
