
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/

#include "hbase_client_pool.h"


namespace idgs {
namespace persist {

HbaseClientPool::HbaseClientPool(const std::string& _host, int _port, size_t _poolSize) : host(_host), port(_port), poolSize(_poolSize) {
  initialized.store(false);
  initialize();
  initialized.store(true);
}

void HbaseClientPool::initialize() {
  for(auto i = 0; i< poolSize; ++i) {
    boost::shared_ptr<TTransport> socket(new TSocket(host, port));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    auto client = std::make_shared<thrift::HbaseClient>(protocol);
    pool.push(client);
  }
}

void HbaseClientPool::close() {
  for(auto it = pool.unsafe_begin(); it != pool.unsafe_end(); ++it) {
    (*it)->getInputProtocol()->getTransport()->close();
    (*it)->getOutputProtocol()->getTransport()->close();
  }
}

} /// end namespace store
} /// end namespace idgs
