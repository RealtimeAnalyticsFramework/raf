/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "client_pool.h"
#include "idgs/store/data_store.h"
#include "idgs/actor/actor_descriptor_mgr.h"

using namespace protobuf;
using namespace idgs::store;

namespace idgs {
namespace client {

TcpClientPool::TcpClientPool() :
    clientConfig(new idgs::client::pb::ClientConfig), availableServers(0) {
  idgs::expr::ExpressionFactory::init();
  isLoaded.store(NOT_INITIALIZED);
}

TcpClientPool::~TcpClientPool() {
  function_footprint();
  close();
}

#define MAX_TRY_TIME_GET_CLIENT_FROM_POOL 10

idgs::ResultCode TcpClientPool::loadConfig(const ClientSetting& setting) {
  idgs::ResultCode code = RC_CLIENT_POOL_IS_AREADY_INIT;
  if (isLoaded.compare_and_swap(INITIALIZED, NOT_INITIALIZED) == NOT_INITIALIZED) {
    clientConfig.reset(new idgs::client::pb::ClientConfig);
    JsonMessage parser;
    code = parser.parseJsonFromFile(clientConfig.get(), setting.clientConfig);

    if (code != RC_SUCCESS) {
      DLOG(ERROR)<< "Failed to parse configuration " << setting.clientConfig << " error: " << getErrorDescription(code);
      return code;
    }

    if (setting.storeConfig != "") {
      code = ::idgs::util::singleton<DataStore>::getInstance().loadCfgFile(setting.storeConfig);
      if (code != RC_SUCCESS) {
        DLOG(ERROR)<< "Pars data store configuration " << setting.storeConfig << " error: " << getErrorDescription(code);;
        return code;
      }
    }

    bool isSynchClient = !clientConfig->async_client();
    availableServers.reserve(clientConfig->server_addresses_size());

    DVLOG(2) << "The client configured server number is " << clientConfig->server_addresses_size();

    for (int i = 0; i < clientConfig->server_addresses_size(); ++i) {
      DVLOG(2) << "try to connect to server: " << clientConfig->server_addresses(i).address() << ":"
                  << clientConfig->server_addresses(i).port();
      TcpClientPtr client;
      if (isSynchClient) {
        client.reset(new TcpSynchronousClient(clientConfig->server_addresses(i)));
      } else {
        client.reset(new TcpAsynchClient(clientConfig->server_addresses(i)));
      }
      client->setId(i);
      code = client->initialize();
      if (code != RC_SUCCESS) {
        DLOG(WARNING)<< " The client " << client.get() << " to server  ["
        << clientConfig->server_addresses(i).address() << ":"
        << clientConfig->server_addresses(i).port() << "]" << " is not available";
      } else {
        DVLOG(2) << "The client " << client.get() << " to server  [" << client->getServerAddress().address()
        << ":" << client->getServerAddress().port() << "]" << " is ready";
        availableServers.push_back(clientConfig->server_addresses(i));
      }
      client->close();
    }

    if (availableServers.empty()) {
      DLOG(ERROR)<< "All the servers is not available";
      return RC_CLIENT_ALL_SERVERS_NOT_AVAILABLE;
    }

    DVLOG(2) << " The available server size is " << availableServers.size();

    pool.set_capacity(clientConfig->pool_size());

    size_t serverSize = availableServers.size();
    for (int step = 0; step < pool.capacity(); ++step) {
      int serverIndex = step % serverSize;
      idgs::client::pb::Endpoint& server = availableServers.at(serverIndex);

      TcpClientPtr client;
      if (isSynchClient) {
        client.reset(new TcpSynchronousClient(server));
      } else {
        client.reset(new TcpAsynchClient(server));
      }

      client->setId(step);
      code = client->initialize();

      // login
      idgs::net::ClientLogin clientLogin;
      DVLOG(2) << "send client login " << sizeof(clientLogin) << " byte size, content is : "
                  << dumpBinaryBuffer2(reinterpret_cast<char*>(&clientLogin), sizeof(clientLogin));
      asio::write(*client->getSocket(), asio::buffer(reinterpret_cast<char*>(&clientLogin), sizeof(clientLogin)),
          asio::transfer_all());

      if (code != RC_SUCCESS) {
        DLOG(WARNING)<< " The client " << client.get() << " to server  ["
        << client->getServerAddress().address() << ":"
        << client->getServerAddress().port() << "]" << " is not available";
      } else {
        DVLOG(2) << "Put client " << client.get() << " with socket " << client->getSocket().get()
        << " at step " << step << " with serverIndex " << serverIndex << " with server " <<
        client->getServerAddress().address() << ":" << client->getServerAddress().port();
        pool.push(client);
      }
    }
    VLOG(6) << toString();

    // load module descriptor
    for (int i = 0; i < clientConfig->modules_size(); ++i) {
      auto m = clientConfig->modules(i);
      auto name = m.name();
      auto descriptor = m.descriptor_file();
      auto config = m.config_file();
      code = ::idgs::util::singleton<idgs::actor::ActorDescriptorMgr>::getInstance().loadModuleActorDescriptor(
          descriptor);
      if (code != RC_SUCCESS) {
        LOG(ERROR)<< "Failed to load module descriptor file, " << getErrorDescription(code) << ", file: " << descriptor;
        continue;
      }
      if (name == "store") {
        code = ::idgs::util::singleton<DataStore>::getInstance().loadCfgFile(config);
        if (code != RC_SUCCESS) {
          LOG(ERROR)<< "Failed to parse store config file,  " << getErrorDescription(code) << ", file: " << config;
          continue;
        }
      }
    }
    code = RC_SUCCESS;
  }
  return code;
}

std::shared_ptr<TcpClientInterface> TcpClientPool::getTcpClient(idgs::ResultCode &code) {
  TcpClientPtr client(NULL);
  std::shared_ptr<TcpClientInterface> clientInterface(NULL);
  size_t try_count = 0;
  size_t count = 0;
  do {
    ++try_count;
    if (pool.try_pop(client)) {
      ++count;
      if (!client->getSocket()->is_open()) {
        LOG(ERROR)<< "socket "<< client->getSocket().get() << " has closed";
        toString();
        continue;
      }
      DVLOG(2) << "get client " << client.get();
      clientInterface.reset(new TcpClientInterface(client));
      code = RC_SUCCESS;
      break;
    } else {
      LOG(ERROR)<< "pop a client error, current available client = " << pool.size() << ", please check the configured pool size";
      code = RC_ERROR;
    }
  } while(count < pool.size() && try_count < MAX_TRY_TIME_GET_CLIENT_FROM_POOL);
  return clientInterface;
}

void TcpClientPool::close() {
  DVLOG(2) << "call TcpClientPool close";
  function_footprint();
  if (isLoaded.compare_and_swap(NOT_INITIALIZED, INITIALIZED) == INITIALIZED) {
    DVLOG(2) << " clear pool with size: " << pool.size();
    pool.clear();
    clientConfig.reset();
    availableServers.clear();
  } else {
    DVLOG(2) << "Client Pool Is not initialized or is already closed";
  }
}

bool TcpClientPool::putBack(TcpClientPtr client) {
  if (isLoaded.load() == NOT_INITIALIZED) {
    DLOG(WARNING)<< "Client Pool Is not initialized";
    return false;
  }

  if(!pool.try_push(client)) {
    DLOG(WARNING) << "Try to recycle extra client";
    return false;
  }

  return true;
}

std::string TcpClientPool::toString() {
  std::stringstream ss;
  ss << "Client Pool(size:  " << size() << ")" << std::endl;
  typedef tbb::concurrent_bounded_queue<TcpClientPtr>::iterator iter;
  for (iter e = pool.unsafe_begin(); e != pool.unsafe_end(); ++e) {
    auto socket = (*e)->getSocket().get();
    std::string flag = socket->is_open() ? " connected" : " closed";
    ss << "client " << (*e).get()->getId() << flag << " to server: " << (*e)->getServerAddress().address() << ":"
        << (*e)->getServerAddress().port() << std::endl;
  }
  return ss.str();
}
}
}

