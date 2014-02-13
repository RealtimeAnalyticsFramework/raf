/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.execution;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import idgs.client.ClientActorMessage;
import idgs.client.TcpClientInterface;
import idgs.client.TcpClientPool;
import idgs.exception.IdgsException;
import idgs.metadata.StoreMetadata;
import idgs.pb.PbRpcMessage.MemberIdConst;
import idgs.rdd.pb.PbRddCommon.RddResultCode;
import idgs.rdd.pb.PbRddService.CreateDelegateRddRequest;
import idgs.rdd.pb.PbRddService.CreateDelegateRddResponse;
import idgs.store.pb.PbStoreConfig.StoreConfig;
import idgs.util.ServerConst;

public class DelegateOperator extends IdgsOperator {

  private static Log LOG = LogFactory.getLog(DelegateOperator.class);
  
  private CreateDelegateRddRequest.Builder builder;
  
  private String storeName;
  
  public DelegateOperator(String storeName) {
    this.storeName = storeName;
    StoreConfig cfg = StoreMetadata.getInstance().getStoreConfig(storeName);
    keyType = cfg.getKeyType();
    valueType = cfg.getValueType();
  }
  
  protected String getName() {
    return ServerConst.DELEGATE;
  }
  
  @Override
  protected void buildRequest() {
    builder = CreateDelegateRddRequest.newBuilder();
    builder.setStoreName(storeName);
    builder.setRddName(getRddName());
  }
  
  @Override
  protected void processOp() throws IdgsException {
    TcpClientInterface client = null;
    
    try {
      client = TcpClientPool.getInstance().getClient();
      if (client == null) {
        throw new IdgsException("cannot connect to server. ");
      }
      
      CreateDelegateRddRequest request = builder.build();
      
      ClientActorMessage reqMsg = getDefaultClientActorMessage();
      reqMsg.setOperationName(ServerConst.CREATE_STORE_DELEGATE_RDD);
      reqMsg.setDestActorId(ServerConst.RDD_SERVICE_ACTOR);
      reqMsg.setDestMemberId(MemberIdConst.ANY_MEMBER_VALUE);
      reqMsg.setPayload(request);
      
      if (LOG.isDebugEnabled()) {
        LOG.debug("Delegate " + request.getStoreName() + " ===> " + reqMsg.toString());
      }
      
      ClientActorMessage respMsg = client.sendRecv(reqMsg);
      if (respMsg == null) {
        throw new IdgsException("no response found");
      }
      
      CreateDelegateRddResponse.Builder builder = CreateDelegateRddResponse.newBuilder();
      respMsg.parsePayload(builder);
      CreateDelegateRddResponse response = builder.build();
      
      if (response.getResultCode() != RddResultCode.RRC_SUCCESS) {
        throw new IdgsException("create store delegate error, caused by code " + response.getResultCode());
      }
    } catch (Exception ex) {
      throw new IdgsException("send or receive message error", ex);
    } finally {
      if (client != null) {
        client.close();
      }
    }
  }

}
