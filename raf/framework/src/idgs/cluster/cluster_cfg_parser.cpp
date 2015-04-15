
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "idgs/cluster/cluster_cfg_parser.h"
#include "idgs/util/utillity.h"
#include "idgs/net/network_interface.h"
#include "cluster_const.h"

namespace idgs {
namespace cluster {

static ResultCode checkConfig(idgs::pb::ClusterConfig& config) {
  char* temp;
  {
    if ((temp = getenv(ENV_VAR_GROUP))) {
      config.set_group_name(temp);
    }
  } /// end check thread_count

  {
    uint32_t thread_count = config.thread_count();
    if ((temp = getenv(ENV_VAR_THREAD_COUNT))) {
      try {
        thread_count = atoi(temp);
      } catch (std::exception& ex) {
        LOG(ERROR)<< ex.what();
        LOG(ERROR) << ENV_VAR_THREAD_COUNT << " should be an integer number";
        return RC_CLUSTER_ERR_CFG;
      }
    }
    if (thread_count <= 0) {
      LOG(ERROR)<< "cluster config error, thread_count can not less than 0";
      return RC_CLUSTER_ERR_CFG;
    }
    config.set_thread_count(thread_count);
  } /// end check thread_count

  {
    uint32_t io_thread_count = config.io_thread_count();
    if ((temp = getenv(ENV_VAR_IO_THREAD_COUNT))) {
      try {
        io_thread_count = atoi(temp);
      } catch (std::exception& ex) {
        LOG(ERROR)<< ex.what();
        LOG(ERROR) << ENV_VAR_IO_THREAD_COUNT << " should be an integer number";
        return RC_CLUSTER_ERR_CFG;
      }
    }
    if (io_thread_count <= 0) {
      LOG(ERROR)<< "cluster config error, io_thread_count can not less than 0";
      return RC_CLUSTER_ERR_CFG;
    }
    config.set_io_thread_count(io_thread_count);
  } /// end check io_thread_count

  /// max idle thread
  {
    uint32_t max_idle_thread = config.max_idle_thread();
    if ((temp = getenv(ENV_VAR_MAX_IDLE_THREAD))) {
      try {
        max_idle_thread = atoi(temp);
      } catch (std::exception& ex) {
        LOG(ERROR)<< ex.what();
        LOG(ERROR) << ENV_VAR_MAX_IDLE_THREAD << " should be an integer number";
        return RC_CLUSTER_ERR_CFG;
      }
    }
    if (max_idle_thread <= 0) {
      LOG(ERROR)<< "cluster config error, io_thread_count can not less than 0";
      return RC_CLUSTER_ERR_CFG;
    }
    if (max_idle_thread >= config.thread_count()) {
      max_idle_thread = config.thread_count() - 1;
    }
    config.set_max_idle_thread(max_idle_thread);
  } /// end check io_thread_count

  /// repartition batch
  {
    uint32_t repartition_batch = config.repartition_batch();
    if ((temp = getenv(ENV_VAR_REPARTITION_BATCH))) {
      try {
        repartition_batch = atoi(temp);
      } catch (std::exception& ex) {
        LOG(ERROR)<< ex.what();
        LOG(ERROR) << ENV_VAR_REPARTITION_BATCH << " should be an integer number";
        return RC_CLUSTER_ERR_CFG;
      }
    }
    if (repartition_batch <= 0) {
      LOG(ERROR)<< "cluster config error, io_thread_count can not less than 0";
      return RC_CLUSTER_ERR_CFG;
    }
    config.set_repartition_batch(repartition_batch);
  } /// end check io_thread_count

  {
    /// mtu
    uint32_t mtu = config.mtu();
    if ((temp = getenv(ENV_VAR_MTU))) {
      try {
        mtu = atoi(temp);
      } catch (std::exception& ex) {
        LOG(ERROR)<< ex.what();
        LOG(ERROR) << ENV_VAR_MTU << " should be an integer number";
        return RC_CLUSTER_ERR_CFG;
      }
    }
//    if (mtu < 0) {
//      LOG(ERROR)<< "cluster config error, mtu can not less than 0";
//      return RC_CLUSTER_ERR_CFG;
//    }
    config.set_mtu(mtu);
  } /// end check mtu

  {
    /// group name
    if ((temp = getenv(ENV_VAR_GROUP_NAME))) {
      config.set_group_name(temp);
    }
  } /// end check mtu

  {
    /// tcp batch
    uint32_t tcp_batch = config.tcp_batch();
    if ((temp = getenv(ENV_VAR_TCP_BATCH))) {
      try {
        tcp_batch = atoi(temp);
      } catch (std::exception& ex) {
        LOG(ERROR)<< ex.what();
        LOG(ERROR) << ENV_VAR_TCP_BATCH << " should be an integer number";
        return RC_CLUSTER_ERR_CFG;
      }
    }
    if (tcp_batch <= 0) {
      LOG(ERROR)<< "cluster config error, " << ENV_VAR_TCP_BATCH << " can not less than 0";
      return RC_CLUSTER_ERR_CFG;
    }
    config.set_tcp_batch(tcp_batch);
  } /// end check tcp_batch

  {
    if (config.reserved_member_size() <= 0) {
      LOG(ERROR)<< "cluster config error, reserved_member_size can not less than 0";
      return RC_CLUSTER_ERR_CFG;
    }
  } //// end check reserved_member_size

  {
    uint32_t partitionCount = config.partition_count();
    if (getenv(ENV_VAR_PARTITION_COUNT)) {
      try {
        partitionCount = atoi(getenv(ENV_VAR_PARTITION_COUNT));
      } catch (std::exception& ex) {
        LOG(ERROR)<< ex.what();
        LOG(ERROR) << ENV_VAR_IO_THREAD_COUNT << " should be an integer number";
        return RC_CLUSTER_ERR_CFG;
      }
    }
    if (partitionCount <= 0) {
      LOG(ERROR)<< "cluster config error, partition_count: " << partitionCount << " can not less than 0";
      return RC_CLUSTER_ERR_CFG;
    }
    if (!sys::isPrime(partitionCount)) {
      LOG(ERROR)<< "cluster config error, partition_count: " << partitionCount << " is not a prime number";
//      return RC_CLUSTER_ERR_CFG;
    }
    config.set_partition_count(partitionCount);
  } //// end check partition_count
  {
    if (config.max_replica_count() < 1) {
      LOG(ERROR)<< "cluster config error, max_replica_count can not less than 1";
      return RC_CLUSTER_ERR_CFG;
    }
  } //// end max_replica_count

  {
    if (str::trim(config.group_name()).length() <= 0) {
      LOG(ERROR)<< "cluster config error, member's group_name can not be empty";
      return RC_CLUSTER_ERR_CFG;
    }
  } //// end group_name

  {
    std::string ip = config.member().public_address().host();
    if (getenv(ENV_VAR_IP)) {
      ip = getenv(ENV_VAR_IP);
      ip = str::trim(ip);
    }
    if (str::trim(ip).length() <= 0 || str::trim(ip).compare("0.0.0.0")/* default */ == 0) {
      std::vector<std::string> interface_addrs;
      idgs::net::network::getInterfaceAddress(interface_addrs);
      ip = interface_addrs.at(0);
    }
    uint32_t port = config.member().public_address().port();
    if (getenv(ENV_VAR_PORT)) {
      try {
        port = atoi(getenv(ENV_VAR_PORT));
      } catch (std::exception& ex) {
        LOG(ERROR)<< ex.what();
        LOG(ERROR) << ENV_VAR_PORT << " should be an integer number";
        return RC_CLUSTER_ERR_CFG;
      }
    }
    if (port <= 0) {
      LOG(ERROR)<< "cluster config error, port can not less than 0";
      return RC_CLUSTER_ERR_CFG;
    }
    if (port > 65535) {
      LOG(ERROR)<< "cluster config error, port can not great than 65535";
      return RC_CLUSTER_ERR_CFG;
    }
    std::string innerIp = config.member().inner_address().host();
    if (getenv(ENV_VAR_INNERIP)) {
      innerIp = getenv(ENV_VAR_INNERIP);
    }
    if (str::trim(innerIp).length() <= 0 || str::trim(innerIp).compare("0.0.0.0")/* default */ == 0) {
      std::vector<std::string> interface_addrs;
      idgs::net::network::getInterfaceAddress(interface_addrs);
      innerIp = interface_addrs.at(0);
    }
    uint32_t innerPort = config.member().inner_address().port();
    if (getenv(ENV_VAR_INNERPORT)) {
      try {
        innerPort = atoi(getenv(ENV_VAR_INNERPORT));
      } catch (std::exception& ex) {
        LOG(ERROR)<< ex.what();
        LOG(ERROR) << ENV_VAR_INNERPORT << " should be an integer number";
        return RC_CLUSTER_ERR_CFG;
      }
    }
    if (innerPort <= 0) {
      LOG(ERROR)<< "cluster config error, innerPort can not less than 0";
      return RC_CLUSTER_ERR_CFG;
    }
    if (innerPort > 65535) {
      LOG(ERROR)<< "cluster config error, innerPort can not great than 65535";
      return RC_CLUSTER_ERR_CFG;
    }
    if (ip == innerIp && port == innerPort) {
      ++innerPort;
    }
    config.mutable_member()->mutable_public_address()->set_host(ip);
    config.mutable_member()->mutable_public_address()->set_port(port);
    config.mutable_member()->mutable_inner_address()->set_host(innerIp);
    config.mutable_member()->mutable_inner_address()->set_port(innerPort);
  } /// end check ip, port, innerIp, innerPort
  {
    uint32_t weight = config.member().weight();
    if (getenv(ENV_VAR_LOAD_FACTOR)) {
      try {
        weight = atoi(getenv(ENV_VAR_LOAD_FACTOR));
      } catch (std::exception& ex) {
        LOG(ERROR)<< ex.what();
        LOG(ERROR) << ENV_VAR_LOAD_FACTOR << " should be an integer number";
        return RC_CLUSTER_ERR_CFG;
      }
    }
    if (weight <= 0) {
      LOG(ERROR)<< "cluster config error, weight can not less than 0";
      return RC_CLUSTER_ERR_CFG;
    }
    config.mutable_member()->set_weight(weight);
  } /// end check weight
  {
    bool local_store = config.member().service().local_store();
    const char* str_local_store = getenv(ENV_VAR_IS_LOCAL_STORE);
    if (str_local_store && str::trim(str_local_store).length() > 0) {
      local_store = (strcmp(str_local_store, "true") == 0 || strcmp(str_local_store, "TRUE") == 0) ? true : false;
    }
    config.mutable_member()->mutable_service()->set_local_store(local_store);
  } /// end check local_store
  {
    bool dist_computing = config.member().service().dist_computing();
    const char* str_dist_computing = getenv(ENV_VAR_IS_DIST_COMPUTING);
    if (str_dist_computing && str::trim(str_dist_computing).length() > 0) {
      dist_computing =
          (strcmp(str_dist_computing, "true") == 0 || strcmp(str_dist_computing, "TRUE") == 0) ? true : false;
    }
    config.mutable_member()->mutable_service()->set_dist_computing(dist_computing);
  } /// end check dist_computing
  {
    bool client_agent = config.member().service().client_agent();
    const char* str_client_agent = getenv(ENV_VAR_IS_CLIENT_AGENT);
    if (str_client_agent && str::trim(str_client_agent).length() > 0) {
      client_agent = (strcmp(str_client_agent, "true") == 0 || strcmp(str_client_agent, "TRUE") == 0) ? true : false;
    }
    config.mutable_member()->mutable_service()->set_client_agent(client_agent);
  } /// end check client_agent
  {
    bool admin_console = config.member().service().client_agent();
    const char* str_admin_console = getenv(ENV_VAR_IS_ADMIN_CONSOLE);
    if (str_admin_console && str::trim(str_admin_console).length() > 0) {
      admin_console = (strcmp(str_admin_console, "true") == 0 || strcmp(str_admin_console, "TRUE") == 0) ? true : false;
    }
    config.mutable_member()->mutable_service()->set_admin_console(admin_console);
  } /// end check admin_console
  {
    for (int i = 0; i < config.modules_size(); ++i) {
      std::string env_var_name = ENV_VAR_MODULE_PREFIX + config.modules(i).name();
      const char* env_var_value = getenv(env_var_name.c_str());
      DVLOG_IF(1, env_var_value != NULL) << env_var_name << " = " << env_var_value;
      if (env_var_value) {
        std::string temp = idgs::str::toLower(std::string(env_var_value));
        if(temp == "false" || temp == "f" || temp == "no" || temp == "n"  ) {
          LOG(INFO) << "module " << config.modules(i).name() << " is disabled";
          config.mutable_modules(i)->set_enable(false);
        }
      }
    }
  } /// end check load module
  {
    if (config.timeout_config().msg_delivery_timeout() <= 0) {
      LOG(ERROR)<< "cluster config error, msg_delivery_timeout can not less than 0";
      return RC_CLUSTER_ERR_CFG;
    }
    if(config.timeout_config().data_access_timeout() <= 0) {
      LOG(ERROR) << "cluster config error, data_access_timeout can not less than 0";
      return RC_CLUSTER_ERR_CFG;
    }
    if(config.timeout_config().computing_timeout() <= 0) {
      LOG(ERROR) << "cluster config error, computing_timeout can not less than 0";
      return RC_CLUSTER_ERR_CFG;
    }
  } /// end check timeout parameters
  return RC_OK;
}

ResultCode ClusterCfgParser::parse(idgs::pb::ClusterConfig& config, const char* xmlFile) {
  LOG(INFO)<< "load cluster config file, file path: " << xmlFile;
  ResultCode rs = (ResultCode)idgs::parseIdgsConfig(&config, std::string(xmlFile));
  if(rs != RC_OK) {
    LOG(ERROR) << "parse config file error, error msg: " << getErrorDescription(rs);
    return rs;
  }
  rs = checkConfig(config);
  DVLOG(7) << config.DebugString();
  return rs;
}

} // end namespace cluster
} // end namespace idgs

