/*
 Copyright (c) <2012>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#include "sql_http_servlet.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <sstream>
#include <memory>

#include "idgs/util/utillity.h"


namespace idgs {
namespace httpserver {

DaemonProcess::DaemonProcess() {
  pipe_in[0] = -1;
  pipe_in[1] = -1;
  pipe_out[0] = -1;
  pipe_out[1] = -1;
  pid = -1;
}

DaemonProcess::~DaemonProcess() {
  stop();
}

int DaemonProcess::start(const std::string& cmd) {
  LOG(INFO) << "Start SQL cli";
  if (pipe(pipe_in) == -1) {
    LOG(ERROR) << "Failed to create pipe for stdout";
    return 1;
  }
  if (pipe(pipe_out) == -1) {
    LOG(ERROR) << "Failed to create pipe for stderr";
    return 1;
  }

  pid = fork();
  if (pid == -1) {
    LOG(ERROR) << "Failed to fork process";
    return 1;
  }

  if (pid == 0) {
    // in child process
    close(pipe_in[1]);
    close(pipe_out[0]);

    if(dup2(pipe_in[0], 0) == -1) {
      LOG(ERROR) << "Failed to dup2 stdin";
    }
    if(dup2(pipe_out[1], 1) == -1) {
      LOG(ERROR) << "Failed to dup2 stdout";
    }
    if(dup2(pipe_out[1], 2) == -1) {
      LOG(ERROR) << "Failed to dup2 stderr";
    }

    if (execlp("/bin/bash", "/bin/bash", cmd.c_str(), NULL) < 0) {
      LOG(ERROR) << "Failed to exec: " << cmd << ", error: " << strerror(errno);
    }
    exit(1);
  } else {
    // in parent process
    close(pipe_in[0]);
    close(pipe_out[1]);

  }
  recv();
  return 0;
}


int DaemonProcess::stop() {
  if (pid > 0) {
    LOG(INFO) << "Stop SQL daemon";
    int status;
    kill(pid, 2); // sig INT
    waitpid(pid, &status, 0);

    close(pipe_in[1]);
    close(pipe_out[0]);
    pid = -1;
  }
  return 0;
}

enum State {
  NORMAL,
  TOKEN_I,   // i
  TOKEN_D,   // d
  TOKEN_G,   // g
  TOKEN_S,   // s
  TOKEN_END  // >
};

std::string DaemonProcess::recv() {
  if (pid < 0) {
    return "";
  }
  State s = NORMAL;
  std::stringstream ss;
  char buff;
  int ret;
  while (s != TOKEN_END) {
    ret = read(pipe_out[0], &buff, 1);
    if (ret == 1) {
      ss << buff;
      switch (s) {
      case NORMAL:
        if (buff == 'i') {
          s = TOKEN_I;
        }
        break;
      case TOKEN_I:   // i
        if (buff == 'd') {
          s = TOKEN_D;
        } else {
          s = NORMAL;
        }
        break;
      case TOKEN_D:   // d
        if (buff == 'g') {
          s = TOKEN_G;
        } else {
          s = NORMAL;
        }
        break;
      case TOKEN_G:   // g
        if (buff == 's') {
          s = TOKEN_S;
        } else {
          s = NORMAL;
        }
        break;
      case TOKEN_S:   // s
        if (buff == '>') {
          s = TOKEN_END;
        } else {
          s = NORMAL;
        }
        break;
      case TOKEN_END:  // >
        // never reach
        break;
      }

    } if (ret < 0) {
      LOG(ERROR) << "Failed to read, reason: " << strerror(errno);
      break;
    } else {
      // read nothing
      continue;
    }
  } // while
  return ss.str();

}

int DaemonProcess::sendrecv(const std::string& request, std::string& response) {
  if (pid < 0) {
    return 1;
  }
  int offset = 0;
  int sz = 0;
  LOG(INFO) << "send sql: " << request;
  while (offset + sz < request.length()) {
    sz = write(pipe_in[1], request.c_str() + offset, request.length());
    offset += sz;
  }
  if(*request.rbegin() != '\n') {
    sz = write(pipe_in[1], "\n", 1);
  }

  response = recv();
  return 0;
}



std::shared_ptr<DaemonProcess> DaemonProcessFactory::getProcess() {
  if (!proc) {
    proc = std::make_shared<DaemonProcess>();
    proc->start("/root/cvsroot/idgs/dist/bin/idgs-sql-cli.sh");
  }
  return proc;
}


SqlHttpServlet::SqlHttpServlet() {
}

SqlHttpServlet::~SqlHttpServlet() {
}

void SqlHttpServlet::doPost(HttpRequest& request, HttpResponse& response) {
  std::string sql = request.getBody();

  sql = idgs::str::trim(sql);

  LOG(INFO) << "SQL: " << sql;

  std::lock_guard<std::mutex> guard(lock);
  auto proc = factory.getProcess();
  std::string stdout;
  proc->sendrecv(sql, stdout);

  response.setContent(stdout);
  response.setStatus(idgs::httpserver::HttpResponse::ok);

  response.addHeader(HTTP_HEADER_CONTENT_TYPE, CONTENT_TYPE_TEXT_PLAIN);

}

} // namespace httpserver
} // namespace idgs
