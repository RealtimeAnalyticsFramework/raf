/*
 Copyright (c) <2012>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#include "shell_http_servlet.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <fstream>

namespace idgs {
namespace httpserver {

ShellHttpServlet::ShellHttpServlet() {
}

ShellHttpServlet::~ShellHttpServlet() {
}

namespace {
int exec_command(const std::string& command, const std::string& argument, std::string* stdout, std::string* stderr = NULL) {
 int pipeout[2];
 int pipeerr[2];
 pid_t cpid;

 if (pipe(pipeout) == -1) {
   LOG(ERROR) << "Failed to create pipe for stdout";
   return 1;
 }
 if (stderr && pipe(pipeerr) == -1) {
   LOG(ERROR) << "Failed to create pipe for stderr";
   return 1;
 }

 cpid = fork();
 if (cpid == -1) {
   LOG(ERROR) << "Failed to fork process";
   return 1;
 }

 if (cpid == 0) {
   // in child process
   close(pipeout[0]);
   if(dup2(pipeout[1], 1) == -1) {
     LOG(ERROR) << "Failed to dup2 stdout";
   }

   if (stderr) {
     close(pipeerr[0]);
     if(dup2(pipeerr[1], 2) == -1) {
       LOG(ERROR) << "Failed to dup2 stderr";
     }
     close(pipeerr[1]);
   } else {
     if(dup2(pipeout[1], 2) == -1) {
       LOG(ERROR) << "Failed to dup2 stderr";
     }
   }
   close(pipeout[1]);

   if (execlp(command.c_str(), command.c_str(), argument.c_str(), NULL) < 0) {
     LOG(ERROR) << "Failed to exec: " << command;
   }
   exit(0);
 } else {
   // in parent process
   close(pipeout[1]);
   if (stderr) {
     close(pipeerr[1]);
   }
   char bufout;
   std::stringstream ssout;
   char buferr;
   std::stringstream sserr;

   while (read(pipeout[0], &bufout, 1) > 0) {
     ssout.put(bufout);
   }
   *stdout = ssout.str();
   close(pipeout[0]);

   if (stderr) {
     while (read(pipeerr[0], &buferr, 1) > 0) {
       sserr.put(buferr);
     }
     *stderr = sserr.str();
     close(pipeerr[0]);
   }

   int status;
   waitpid(cpid, &status, 0);
 }
 return 0;
}
} // namespace


void ShellHttpServlet::doPost(HttpRequest& request, HttpResponse& response) {
  char * tmpfile = tempnam("/tmp", "ecf_");
  std::string script_file = tmpfile;
  free(tmpfile);

  std::ofstream ofs(script_file.c_str());
  ofs << request.getBody();
  ofs << std::endl;
  ofs.close();

  LOG(INFO) << "file: " << script_file << ", script: " << request.getBody();

  std::string stdout;
  {
    std::lock_guard<std::mutex> guard(lock);
    exec_command("bash", script_file, &stdout, NULL);
  }

  response.setContent(stdout);
  response.setStatus(idgs::httpserver::HttpResponse::ok);

  response.addHeader(HTTP_HEADER_CONTENT_TYPE, CONTENT_TYPE_TEXT_PLAIN);

  remove(script_file.c_str());
  return;
}


} // namespace httpserver
} // namespace idgs
