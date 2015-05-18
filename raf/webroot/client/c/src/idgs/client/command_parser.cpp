/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */

#include "command_parser.h"

namespace idgs {
namespace client {

/// parse a string command to Command Object
idgs::ResultCode CommandParser::parse(const std::string& str_cmd, Command* command) {
  // 1. parse actorId
  size_t pos = str_cmd.find(" ");
  command->actorId = str_cmd.substr(0, pos);
  DVLOG(2) << "command: " << command->toString();

  // 2. parse opName
  std::string subString = str_cmd.substr(pos + 1, str_cmd.size());
  pos = subString.find(" ");
  command->opName = subString.substr(0, pos);
  DVLOG(2) << "command: " << command->toString();

  // 3. parse payload
  idgs::ResultCode rc;
  size_t begin = command->opName.length() + 1;
  subString = subString.substr(begin, str_cmd.size());

  std::string left_string;
  command->payload = nextJsonStr(subString, &rc, left_string);
  DVLOG(2) << "command: " << command->toString();
  if (rc != RC_OK) {
    LOG(ERROR)<< "parse payload error, payload should be a valid json string begin with '{', end with '}' ";
    return rc;
  }

  // 4. parse attachment
  subString = left_string;
  while (rc == RC_OK && subString != "") {
    if (subString.at(0) == ' ') {
      subString.erase(0, 1);
    }

    size_t key_end_pos = subString.find("=");
    if (key_end_pos == 0) {
      LOG(ERROR)<< "parse attachment's key error, attachment's key should not be empty";
      return RC_SYNTAX_ERROR;
    }

    std::string key = subString.substr(0, key_end_pos);
    if (subString.at(key_end_pos + 1) == ' ') {
      subString.erase(key_end_pos + 1, 1);
    }
    subString = subString.substr(key_end_pos + 1, str_cmd.size());
    std::string value = nextJsonStr(subString, &rc, left_string);
    if (rc != RC_OK) {
      LOG(ERROR)<< "parse attachment's [" << subString <<"] error, attachment's value should be a valid json string begin with '{', end with '}' ";
      return rc;
    }
    command->attachments.insert(std::pair<std::string, std::string>(key, value));
    DVLOG(2) << "put key->" << key << "|| value->" << value;
    subString = left_string;
    /*        pos += end;
     leftString = leftString.substr(pos, str_cmd.size());*/
  }
  DVLOG(2) << "command: " << command->toString();
  return RC_SUCCESS;
}

std::string CommandParser::nextJsonStr(const std::string& str_cmd, idgs::ResultCode* rc, std::string& leftString) {
  //LOG(INFO) << "str_cmd " << str_cmd;
  std::string tmpString = str_cmd;
  size_t start = tmpString.find("{");
  if (start != 0) { /// not start with "{"
    *rc = RC_SYNTAX_ERROR;
    return "";
  }
  size_t pos = 1;
  tmpString = tmpString.substr(pos, str_cmd.size());
  //LOG(INFO) << " leftString>>" << tmpString;
  size_t end = pos;
  size_t counter = 1;

  bool syntax_right = false;
  while (pos < str_cmd.size()) {
    tmpString.at(pos);
    if (tmpString.at(pos) == '{') {
      ++counter;
    }
    if (tmpString.at(pos) == '}') {
      --counter;
      if (counter == 0) {
        syntax_right = true;
        end = pos + 2;
        break;
      }
    }
    ++pos;
  }
  if (!syntax_right) {
    *rc = RC_SYNTAX_ERROR;
    return "";
  }

  *rc = RC_SUCCESS;
  if (end < str_cmd.size()) {
    leftString = str_cmd.substr(end, str_cmd.size()); //+3 means by pass "}[space] character
  } else {
    leftString = "";
  }

  return str_cmd.substr(0, end);

}

std::string CommandParser::subJsonStr(const std::string& str_cmd, size_t begin, size_t* end, idgs::ResultCode* rc) {
  DVLOG(2) << "str_cmd " << str_cmd << " >> begin " << begin << " >> end " << *end;
  std::string leftString = str_cmd.substr(begin, str_cmd.size());
  DVLOG(2) << " leftString>>" << leftString;
  size_t start = leftString.find("{");
  DVLOG(2) << " start is " << start;
  if (start != 0) { /// not start with "{"
    *rc = RC_SYNTAX_ERROR;
    return "";
  }
  size_t pos = start + 1;
  leftString = leftString.substr(pos, str_cmd.size());
  DVLOG(2) << " leftString>>" << leftString;
  *end = pos;
  size_t counter = 0;
  ++counter;
  bool syntax_right = false;
  while (pos < str_cmd.size()) {
    leftString.at(pos);
    if (leftString.at(pos) == '{') {
      ++counter;
    }
    if (leftString.at(pos) == '}') {
      --counter;
      if (counter == 0) {
        syntax_right = true;
        *end = pos;
        break;
      }
    }
    ++pos;
    //leftString = leftString.substr(pos, str_cmd.size());
  }
  if (!syntax_right) {
    *rc = RC_SYNTAX_ERROR;
    return "";
  }

  *rc = RC_SUCCESS;
  return str_cmd.substr(begin + 1, *end);
}
}
}
