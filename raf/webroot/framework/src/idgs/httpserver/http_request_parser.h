
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include <tuple>

namespace idgs {
namespace httpserver {

enum ParseStatus {
  Invalid,
  Complete,
  Indeterminate
};

class HttpRequest;

/// Parser for incoming requests.
class HttpRequestParser {
public:
  HttpRequestParser();

public:
  /// Reset to initial parser state.
  void reset();

  /// Parse some data. The  return value is Complete when a complete request
  /// has been parsed, Invalid if the data is invalid, indeterminate when more
  /// data is required. The InputIterator return value indicates how much of the
  /// input has been consumed.
  template <typename InputIterator>
  std::tuple<ParseStatus, InputIterator> parse(HttpRequest& req,
      InputIterator begin, InputIterator end) {
    while (begin != end) {
      ParseStatus result = consume(req, *begin++);
      if (result == Invalid || result == Complete)
        return std::make_tuple(result, begin);
    }
    ParseStatus result = Indeterminate;
    return std::make_tuple(result, begin);
  }

private:
  ParseStatus consume(HttpRequest& req, char input);
  static bool isChar(int c);
  static bool isCtl(int c);
  static bool isTspecial(int c);
  static bool isDigit(int c);

private:
  enum state
  {
    method_start,
    method,
    uri_start,
    uri,
    http_version_h,
    http_version_t_1,
    http_version_t_2,
    http_version_p,
    http_version_slash,
    http_version_major_start,
    http_version_major,
    http_version_minor_start,
    http_version_minor,
    expecting_newline_1,
    header_line_start,
    header_lws,
    header_name,
    space_before_header_value,
    header_value,
    expecting_newline_2,
    expecting_newline_3
  } state_;
};

} // namespace httpserver
} // namespace idgs
