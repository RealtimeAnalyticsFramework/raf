/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once




namespace idgs {
namespace persist {
  typedef std::shared_ptr<thrift::HbaseClient> HbaseClientPtr;
  class HbaseClientPool {
  public:
    HbaseClientPool(const std::string& host, int port, size_t poolSize = 10);

    const std::string& getHost() const {
      return host;
    }

    const int getPort() const {
      return port;
    }

    const size_t getPoolSize() const {
      return pool.unsafe_size();
    }

    HbaseClientPtr getClient() {
      HbaseClientPtr client;
      pool.try_pop(client);
      return client;
    }

  private:
    const std::string& host;
    const int port;
    size_t poolSize;
    std::atomic<bool> initialized;
    tbb::concurrent_queue<HbaseClientPtr> pool;

    void initialize();

    void close();

  }; /// end class
} /// end namespace store
} /// end namespace idgs
