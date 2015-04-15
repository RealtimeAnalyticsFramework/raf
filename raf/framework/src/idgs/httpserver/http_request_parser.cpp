
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "http_request_parser.h"
#include "http_request.h"

namespace idgs{
namespace httpserver {

HttpRequestParser::HttpRequestParser()
  : state_(method_start) {
}

void HttpRequestParser::reset() {
  state_ = method_start;
}

bool HttpRequestParser::isChar(int c) {
  return c >= 0 && c <= 127;
}

ParseStatus HttpRequestParser::consume(HttpRequest& req, char input) {
  switch (state_) {
    case method_start:
      if (!isChar(input) || isCtl(input) || isTspecial(input)) {
        return Invalid;
      } else {
        state_ = method;
        req.getMethod().push_back(input);
        return Indeterminate;
      }
    case method:
      if (input == ' ') {
        state_ = uri;
        return Indeterminate;
      } else if (!isChar(input) || isCtl(input) || isTspecial(input)) {
        return Invalid;
      } else {
        req.getMethod().push_back(input);
        return Indeterminate;
      }
    case uri_start:
      if (isCtl(input)) {
        return Invalid;
      } else {
        state_ = uri;
        req.getUri().push_back(input);
        return Indeterminate;
      }
    case uri:
      if (input == ' ') {
        state_ = http_version_h;
        return Indeterminate;
      } else if (isCtl(input)) {
        return Invalid;
      } else {
        req.getUri().push_back(input);
        return Indeterminate;
      }
    case http_version_h:
      if (input == 'H') {
        state_ = http_version_t_1;
        return Indeterminate;
      } else {
        return Invalid;
      }
    case http_version_t_1:
      if (input == 'T') {
        state_ = http_version_t_2;
        return Indeterminate;
      } else {
        return Invalid;
      }
    case http_version_major:
      if (input == '.') {
        state_ = http_version_minor_start;
        return Indeterminate;
      } else if (isDigit(input)) {
        req.setHttpVersionMajor(
            req.getHttpVersionMajor() * 10 + input - '0');
        return Indeterminate;
      } else {
        return Invalid;
      }
    case http_version_minor_start:
      if (isDigit(input)) {
        req.setHttpVersionMinor(
            req.getHttpVersionMinor() * 10 + input - '0');
        state_ = http_version_minor;
        return Indeterminate;
      } else {
        return Invalid;
      }
    case http_version_t_2:
      if (input == 'T') {
        state_ = http_version_p;
        return Indeterminate;
      } else {
        return Invalid;
      }
    case expecting_newline_3:
      return (input == '\n') ? Complete : Invalid;
    case http_version_p:
      if (input == 'P') {
        state_ = http_version_slash;
        return Indeterminate;
      } else {
        return Invalid;
      }
    case http_version_slash:
      if (input == '/') {
        req.setHttpVersionMajor(0);
        req.setHttpVersionMinor(0);
        state_ = http_version_major_start;
        return Indeterminate;
      } else {
        return Invalid;
      }
    case http_version_major_start:
      if (isDigit(input)) {
        req.setHttpVersionMajor(
            req.getHttpVersionMajor() * 10 + input - '0');
        state_ = http_version_major;
        return Indeterminate;
      } else {
        return Invalid;
      }
    case expecting_newline_2:
      if (input == '\n') {
        state_ = header_line_start;
        return Indeterminate;
      } else {
        return Invalid;
      }
    case header_lws:
      if (input == '\r') {
        state_ = expecting_newline_2;
        return Indeterminate;
      } else if (input == ' ' || input == '\t') {
        return Indeterminate;
      } else if (isCtl(input)) {
        return Invalid;
      } else {
        state_ = header_value;
        req.getHeaders().back().header_value.push_back(input);
        return Indeterminate;
      }
    case header_value:
      if (input == '\r') {
        state_ = expecting_newline_2;
        return Indeterminate;
      } else if (isCtl(input)) {
        return Invalid;
      } else {
        req.getHeaders().back().header_value.push_back(input);
        return Indeterminate;
      }
    case http_version_minor:
      if (input == '\r') {
        state_ = expecting_newline_1;
        return Indeterminate;
      } else if (isDigit(input)) {
        req.setHttpVersionMinor(
            req.getHttpVersionMinor() * 10 + input - '0');
        return Indeterminate;
      } else {
        return Invalid;
      }
    case expecting_newline_1:
      if (input == '\n') {
        state_ = header_line_start;
        return Indeterminate;
      } else {
        return Invalid;
      }
    case header_line_start:
      if (input == '\r') {
        state_ = expecting_newline_3;
        return Indeterminate;
      } else if (!req.getHeaders().empty()
          && (input == ' ' || input == '\t')) {
        state_ = header_lws;
        return Indeterminate;
      } else if (!isChar(input) || isCtl(input) || isTspecial(input)) {
        return Invalid;
      } else {
        req.getHeaders().push_back(HttpHeader());
        req.getHeaders().back().header_name.push_back(input);
        state_ = header_name;
        return Indeterminate;
      }
    case header_name:
      if (input == ':') {
        state_ = space_before_header_value;
        return Indeterminate;
      } else if (!isChar(input) || isCtl(input) || isTspecial(input)) {
        return Invalid;
      } else {
        req.getHeaders().back().header_name.push_back(input);
        return Indeterminate;
      }
    case space_before_header_value:
      if (input == ' ') {
        state_ = header_value;
        return Indeterminate;
      } else {
        return Invalid;
      }
    default:
      return Invalid;
}
}

bool HttpRequestParser::isTspecial(int c) {
  switch (c) {
  case '(': case ')': case '<': case '>': case '@':
  case ',': case ';': case ':': case '\\': case '"':
  case '/': case '[': case ']': case '?': case '=':
  case '{': case '}': case ' ': case '\t':
    return true;
  default:
    return false;
  }
}


bool HttpRequestParser::isCtl(int c) {
  return (c >= 0 && c <= 31) || (c == 127);
}



bool HttpRequestParser::isDigit(int c) {
  return c >= '0' && c <= '9';
}

} // namespace httpserver
} // namespace idgs
