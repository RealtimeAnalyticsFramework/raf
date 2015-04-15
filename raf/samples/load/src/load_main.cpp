/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include "loader_factory.h"

using namespace std;
using namespace idgs;
using namespace idgs::client;

bool parseArguments(int argc, char* argv[], LoaderSettings* settings) {
  int i = 1;
  for (; i < (argc - 1); ++i) {
    string opt(argv[i]);
    if (opt == "-p" || opt == "--path") {
      settings->data_path = argv[++i];
    } else if (opt == "-s" || opt == "--standalone") {
      settings->standalone = atoi(argv[++i]);
    } else if (opt == "-c" || opt == "--config") {
      settings->client_cfg_file = argv[++i];
    } else if (opt == "-m" || opt == "--mapper") {
      settings->mapper_cfg_file = argv[++i];
    } else if (opt == "-o" || opt == "--output") {
      settings->output_file = argv[++i];
    } else if (opt == "-t" || opt == "--thread") {
      settings->thread_count = atoi(argv[++i]);
    } else {
      cerr << "Unknown option: " << opt << endl;
      return false;
    }
  }
  if (i < argc) {
    cerr << "Unknown option or no value: " << argv[i] << endl;
    return false;
  }
  return true;
}

void usage(const char* name) {
  cout << "Usage: " << name << " [options]" << endl;

  cout << "  -p <data_path> " << endl;
  cout << "  --path <data_path> " << endl;
  cout << "       specify the path of tpch/ssb data files." << endl;
  cout << endl;

  cout << "  -s <0|1|2> " << endl;
  cout << "  --standalone <0|1|2> " << endl;
  cout << "       standalone, in-cluster, or linecrud client." << endl;
  cout << endl;

  cout << "  -c <config_file> " << endl;
  cout << "  --config <config_file> " << endl;
  cout << "       specify the cluster/client configuration." << endl;
  cout << endl;

  cout << "  -m <mapper_config_file> " << endl;
  cout << "  --mapper <mapper_config_file> " << endl;
  cout << "       specify the store file maper configuration." << endl;
  cout << endl;

  cout << "  -o <output_file> " << endl;
  cout << "  --output <output_file> " << endl;
  cout << "       specify the result file" << endl;
  cout << endl;

  cout << "  -t <thread_count> " << endl;
  cout << "  --thread <thread_count> " << endl;
  cout << "       specify the thread count." << endl;
  cout << endl;
}

int main(int argc, char* argv[]) {
  LoaderSettings settings;
  google::InstallFailureSignalHandler();

  if (!parseArguments(argc, argv, &settings)) {
    usage(argv[0]);
    //override param
    char* str_load_files_dir = getenv("LOAD_FILES_DIR");
    if (str_load_files_dir) {
      settings.data_path = str_load_files_dir;
    }
    return 1;
  }
  /// create loader by factory
  std::unique_ptr<Loader> loader_ptr(LoaderFactory::createLoader(settings.standalone));

  /// initialize
  idgs::ResultCode rc = loader_ptr->init(&settings);
  if (rc != RC_OK) {
    LOG(ERROR)<< "loader init error, " << idgs::getErrorDescription(rc);
    return 1;
  }

  /// config
  loader_ptr->config();

  /// import
  loader_ptr->import();

  return 0;
}
