
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $

#include <fstream>

#include "idgs/util/utillity.h"
#include "idgs/client/client_pool.h"
#include "idgs/client/command_parser.h"


using namespace std;
using namespace idgs;
using namespace idgs::pb;
using namespace idgs::client;

// print usage
void usage(const char* name);

// parse arguments
bool parseArguments(int argc, char* argv[], idgs::client::ClientSetting* setting);

// execute a command
bool execute(const string& cmd);

// execute a script file
bool executeScript(const string& filename);

//command : samples/shell/client -c [client_config_file] -s [datastore_config_file] -f [script_file]

ClientSetting setting;

int main(int argc, char **argv) {
  setting.clientConfig = "conf/client.conf";
  setting.storeConfig = "conf/data_store.conf";
  setting.scriptFile = "";

  google::InstallFailureSignalHandler();

  // parse arguments
  if (!parseArguments(argc, argv, &setting)) {
    usage(argv[0]);
    exit(1);
  }

  ResultCode code = getTcpClientPool().loadConfig(setting);
  if (code != RC_SUCCESS) {
    DLOG(ERROR) << "Initial ClientPool error: " << getErrorDescription(code);
    exit(1);
  }

  // show shell help
  if (!setting.silient) {
    cout << "" << endl;
    cout << "##############################################################################" << endl;
    cout << "#                        Start idgs client shell.                            #" << endl;
    cout << "#    Type 'help;' to show command, Type 'quit;' or CTRL + C to exit.         #" << endl;
    cout << "##############################################################################" << endl;
    cout << "" << endl;
  }

  unsigned long start, end;
  // execute script file
  if (strcmp(setting.scriptFile.c_str(), "") != 0) {
    start = sys::getCurrentTime();
    if (!executeScript(setting.scriptFile)) {
      exit(1);
    }

    end = sys::getCurrentTime();
    if (!setting.silient) {
      cout << "Done at " << (end - start) / 1000.0 << "s" << endl;
    }
    exit(0);
  }

  cout << "idgs(client)>";

  // catch input from console
  char ch, prech = 0;
  stringstream command;
  while (1) {
    ch = getchar();
    if (ch == '\n') {
      // catch a command
      if (prech == ';') {
        string cmd = command.str().substr(0, command.str().length() - 1);
        // quit client
        if (strcmp(cmd.c_str(), "quit") == 0) {
          exit(0);
        // print help info
        } else if (strcmp(cmd.c_str(), "help") == 0) {
          cout << "Command must be end with ';'. Example : " << endl;
          cout << "\tinsert name=[store name] key=[json of key] value= [json of value];" << endl;
          cout << "\tupdate name=[store name] key=[json of key] value= [json of value];" << endl;
          cout << "\tget name=[store name] key=[json of key];" << endl;
          cout << "\tdelete name=[store name] key=[json of key];" << endl;
          cout << "\tload [script file];" << endl;
        // handle a script file
        } else if (strcmp(cmd.substr(0, 5).c_str(), "load ") == 0) {
          start = sys::getCurrentTime();
          string filename = cmd.substr(5, cmd.length());
          if (!executeScript(filename)) {
            exit(1);
          }

          end = sys::getCurrentTime();

          if (!setting.silient) {
            cout << "Done at " << (end - start) / 1000.0 << "s" << endl;
          }
        // handle a command
        } else {
          start = sys::getCurrentTime();
          if (!execute(cmd)) {
            exit(1);
          }

          end = sys::getCurrentTime();
          if (!setting.silient) {
            cout << "Done at " << (end - start) / 1000.0 << "s" << endl;
          }
        }

        command.str("");
        if (!setting.silient) {
          cout << endl;
        }
      }

      if (!setting.silient) {
        cout << "idgs(client)>";
      }
    } else {
      command << ch;
    }

    prech = ch;
  }
  getTcpClientPool().close();
}

bool parsePbJsonPayload(std::string& outPut, ClientActorMessagePtr msg) {
  // outPut = protobuf::ProtobufJson::toIndentJsonString(msg->getRpcMessage()->payload());
  outPut = msg->getRpcMessage()->payload();
  return true;
}


bool parsePaylaod(std::string& outPut, ClientActorMessagePtr msg) {
  if (msg->getSerdesType() == PB_JSON) {
    return parsePbJsonPayload(outPut, msg);
  } else {
    outPut = msg->getRpcMessage()->payload();
    return true;
  }
}



bool execute(const string& cmd) {
  CommandParser parser;
  Command command;
  ResultCode code = parser.parse(cmd, &command);
  if (code != RC_SUCCESS) {
    cout << "Error : " << getErrorDescription(code);
    return false;
  }
  auto client = getTcpClientPool().getTcpClient(code);
  if (code != RC_SUCCESS) {
    DLOG(ERROR) << "Get TcpSynchronousClient error: " << getErrorDescription(code);;
    exit(1);
  }
  ClientActorMessagePtr clientActorMsg = std::make_shared<ClientActorMessage>();
  code = command.toActorMsg(clientActorMsg);
  if (code != RC_SUCCESS) {
    DLOG(ERROR) << "Parse command error: " << getErrorDescription(code) << ", command is : " << cmd;
    exit(1);
  }

  // send message, wait and get response message
  ClientActorMessagePtr tcpResponse;
  code = client->sendRecv(clientActorMsg, tcpResponse);

  if (code != RC_SUCCESS) {
    cout << "execute the command error: " << getErrorDescription(code) << endl;
    return false;
  }

  string result;
  if (!parsePaylaod(result, tcpResponse)) {
    cout << "get result error!! >> " << tcpResponse->getPayload()->DebugString();
    return false;
  }

  cout << result;

  return true;
}




bool executeScript(const string& filename/*, const ClientSetting& setting, auto client*/) {
  // whether file exists
  if (access(filename.c_str(), 0) != 0) {
    cout << "File not found." << endl;
    return false;
  }

  // whether file can read
  if (access(filename.c_str(), 4) != 0) {
    cout << "File cannot read." << endl;
    return false;
  }

  bool success = true;

  // load file
  ifstream fs(filename);
  if (fs.eof()) {
    success = false;
  } else {
    int32_t len = 1024;
    while (!fs.eof()) {
      // read a line as command
      char line[len];
      fs.getline(line, len - 1);
      string cmd(line);

      string command = str::trim(cmd);
      if (command.length() == 0 || command.c_str()[0] == '#') {
        continue;
      }

      if (!setting.silient) {
        cout << endl;
        cout << "idgs(client)>" << command << endl;
      }
      // parse command and execute operation
      if (!execute(command.substr(0, command.length() - 1))) {
        success = false;
        break;
      }
    }
  }

  fs.close();
  return success;
}

void usage(const char* name) {
  cout << "Usage: " << name << " [options]" << endl;

  // client config
  cout << "  -c <client_config_path> " << endl;
  cout << "  --client-config <client_config_path> " << endl;
  cout << "       specify the client configuration." << endl;
  cout << endl;

  // store config
  cout << "  -s <store_config_path> " << endl;
  cout << "  --store-config <store_config_path> " << endl;
  cout << "       specify the store configuration." << endl;
  cout << endl;

  // script file
  cout << "  -f <script_file> " << endl;
  cout << "  --script_file <script_file> " << endl;
  cout << "       specify the file of script." << endl;
  cout << endl;

  // help
  cout << "  -h " << endl;
  cout << "  --help " << endl;
  cout << "       for help." << endl;
  cout << endl;
}

bool parseArguments(int argc, char* argv[], idgs::client::ClientSetting* setting) {
  int i = 1;
  for(; i < (argc - 1); ++i) {
    string opt(argv[i]);
    // parse client config
    if(opt == "-c" || opt == "--cluster-config") {
      setting->clientConfig = argv[++i];
    // parse store config
    } else if (opt == "-s" || opt == "--store-config") {
      setting->storeConfig = argv[++i];
    // parse script file
    } else if (opt == "-f" || opt == "--script_file") {
      setting->scriptFile = argv[++i];
      setting->silient = true;
    // parse help command
    } else if (opt == "-h" || opt == "--help") {
      usage(argv[0]);
      exit(0);
    // other arg error.
    } else {
      cerr << "Unknown option: " << opt << endl;
      return false;
    }
  }

  return true;
}
