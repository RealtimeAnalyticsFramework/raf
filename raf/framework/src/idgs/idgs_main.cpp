
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "idgs/application.h"
#include "idgs/signal_handler.h"


/// @todo for debug
#include "idgs/net/network_statistics.h"
#include "idgs/net/inner_tcp_server.h"

using namespace idgs;
using namespace std;

namespace {
  struct ApplicationSetting {
    ApplicationSetting():clusterConfig("") {}
    std::string clusterConfig;
  };
}

static void usage(const char* name);
static bool parseArguments(int argc, char* argv[], ApplicationSetting* setting);
static int run(int argc, char* argv[]);

int main(int argc, char* argv[]) {
  function_footprint();
  // google::InitGoogleLogging(argv[0]);
  LOG(INFO) << "Total CPU cores: " << std::thread::hardware_concurrency();
  try {
    return run(argc, argv);
  } catch (const std::exception& e) {
    LOG(ERROR) << "Got exception: " << e.what() << endl;
  } catch (...) {
    catchUnknownException();
    return 1;
  }
}

void monitor(Application& app) {
  if(unlikely(!app.isRunning())) {
    return;
  }
  /// @todo for debug
  auto network = idgs_application()->getRpcFramework()->getNetwork();
  static ::idgs::net::NetworkStatistics* stats = network->getNetworkStatistics();
  static ::idgs::net::InnerTcpServer* innerTcp = network->getInnerTcpServer();
  static idgs::actor::ActorMessageQueue* queue = app.getActorMessageQueue();

  std::string net = stats->toString();
  stats->reset();
  std::string netdump = innerTcp->toString();
  VLOG(4) << std::endl
      << net
      << netdump
      << "Actor: idle thread: " << queue->getIdleThread() << ", pending messages: " << queue->getPendingMessages();
}

int run(int argc, char* argv[]) {
  function_footprint();
  ApplicationSetting setting;
  // default value
  setting.clusterConfig = "conf/cluster.conf";

  if(!parseArguments(argc, argv, &setting)) {
    usage(argv[0]);
    return 1;
  }
  LOG(INFO) << "Start server with configuration: " << setting.clusterConfig;

  idgs::Application& app = * idgs_application();
  ResultCode rc;

  DVLOG(1) << "Loading configuration.";
  rc = app.init(setting.clusterConfig);
  if (rc != RC_SUCCESS) {
    LOG(ERROR) << "Failed to initialize server: " << getErrorDescription(rc);
    exit(1);
  }

  DVLOG(1) << "Server is starting.";
  rc = app.start();
  if (rc != RC_SUCCESS) {
    LOG(ERROR) << "Failed to start server: " << getErrorDescription(rc);
    exit(1);
  }


  DVLOG(1) << "";
  SignalHandler sh;
  sh.setup();
  LOG(INFO) << "=============================================================";
  LOG(INFO) << "Server is started, please press ctrl-c to exit.";
  LOG(INFO) << "=============================================================";

  // dead loop
  while(app.isRunning()) {
    sleep(1);
    monitor(app);
  }

  DVLOG(1) << "Server is shutting down.";
  rc = app.stop();
  if (rc != RC_SUCCESS) {
    LOG(ERROR) << "Failed to start server: " << getErrorDescription(rc);
    exit(1);
  }
  return 0;
}

void usage(const char* name) {
  cout << "Usage: " << name << " [options]" << endl;

  cout << "  -c <cluster_config_path> " << endl;
  cout << "  --cluster-config <cluster_config_path> " << endl;
  cout << "       specify the cluster configuration." << endl;
  cout << endl;
}

bool parseArguments(int argc, char* argv[], ApplicationSetting* setting) {
  int i = 1;
  for(; i < (argc - 1); ++i) {
    string opt(argv[i]);
    if(opt == "-c" || opt == "--cluster-config") {
      setting->clusterConfig = argv[++i];
    } else {
      cerr << "Unknown option: " << opt << endl;
      return false;
    }
  }
  if(i < argc) {
    cerr << "Unknown option or no value: " << argv[i] << endl;
    return false;
  }
  return true;
}
