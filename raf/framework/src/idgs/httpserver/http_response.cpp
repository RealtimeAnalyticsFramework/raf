
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "http_response.h"

namespace idgs {
namespace http {
namespace server {

namespace misc_strings {

const char name_value_separator[] = { ':', ' ' };
const char crlf[] = { '\r', '\n' };

} // namespace misc_strings

namespace status_strings {

const std::string ok =
  "HTTP/1.0 200 OK\r\n";
const std::string created =
  "HTTP/1.0 201 Created\r\n";
const std::string accepted =
  "HTTP/1.0 202 Accepted\r\n";
const std::string no_content =
  "HTTP/1.0 204 No Content\r\n";
const std::string multiple_choices =
  "HTTP/1.0 300 Multiple Choices\r\n";
const std::string moved_permanently =
  "HTTP/1.0 301 Moved Permanently\r\n";
const std::string moved_temporarily =
  "HTTP/1.0 302 Moved Temporarily\r\n";
const std::string not_modified =
  "HTTP/1.0 304 Not Modified\r\n";
const std::string bad_request =
  "HTTP/1.0 400 Bad Request\r\n";
const std::string unauthorized =
  "HTTP/1.0 401 Unauthorized\r\n";
const std::string forbidden =
  "HTTP/1.0 403 Forbidden\r\n";
const std::string not_found =
  "HTTP/1.0 404 Not Found\r\n";
const std::string internal_server_error =
  "HTTP/1.0 500 Internal Server Error\r\n";
const std::string not_implemented =
  "HTTP/1.0 501 Not Implemented\r\n";
const std::string bad_gateway =
  "HTTP/1.0 502 Bad Gateway\r\n";
const std::string service_unavailable =
  "HTTP/1.0 503 Service Unavailable\r\n";

asio::const_buffer toBuffer(HttpResponse::StatusType status) {
  switch (status) {
  case HttpResponse::ok:
    return asio::buffer(ok);
  case HttpResponse::created:
    return asio::buffer(created);
  case HttpResponse::multiple_choices:
    return asio::buffer(multiple_choices);
  case HttpResponse::moved_permanently:
    return asio::buffer(moved_permanently);
  case HttpResponse::moved_temporarily:
    return asio::buffer(moved_temporarily);
  case HttpResponse::accepted:
    return asio::buffer(accepted);
  case HttpResponse::no_content:
    return asio::buffer(no_content);
  case HttpResponse::not_modified:
    return asio::buffer(not_modified);
  case HttpResponse::not_implemented:
    return asio::buffer(not_implemented);
  case HttpResponse::bad_gateway:
    return asio::buffer(bad_gateway);
  case HttpResponse::service_unavailable:
    return asio::buffer(service_unavailable);
  case HttpResponse::bad_request:
    return asio::buffer(bad_request);
  case HttpResponse::unauthorized:
    return asio::buffer(unauthorized);
  case HttpResponse::forbidden:
    return asio::buffer(forbidden);
  case HttpResponse::not_found:
    return asio::buffer(not_found);
  case HttpResponse::internal_server_error:
    return asio::buffer(internal_server_error);
  default:
    return asio::buffer(internal_server_error);
  }
}

} // namespace status_strings

namespace stock_replies {

const char ok[] = "";
const char created[] =
  "<html>"
  "<head><title>Created</title></head>"
  "<body><h1>201 Created</h1></body>"
  "</html>";
const char accepted[] =
  "<html>"
  "<head><title>Accepted</title></head>"
  "<body><h1>202 Accepted</h1></body>"
  "</html>";
const char no_content[] =
  "<html>"
  "<head><title>No Content</title></head>"
  "<body><h1>204 Content</h1></body>"
  "</html>";
const char multiple_choices[] =
  "<html>"
  "<head><title>Multiple Choices</title></head>"
  "<body><h1>300 Multiple Choices</h1></body>"
  "</html>";
const char moved_permanently[] =
  "<html>"
  "<head><title>Moved Permanently</title></head>"
  "<body><h1>301 Moved Permanently</h1></body>"
  "</html>";
const char moved_temporarily[] =
  "<!DOCTYPE HTML>"
  "<html lang=\"en-US\">"
  "<head>"
  "<meta charset=\"UTF-8\">"
  "<meta http-equiv=\"refresh\" content=\"1;url=http://example.com\">"
  "<script type=\"text/javascript\">"
  " window.location.href = \"http://example.com\""
  "</script>"
  "<title>Page Redirection</title>"
  "</head>"
  "<body>"
  "If you are not redirected automatically, follow the <a href='http://example.com'>link to example.com</a>"
  "</body>"
  "</html>";
const char not_modified[] =
  "<html>"
  "<head><title>Not Modified</title></head>"
  "<body><h1>304 Not Modified</h1></body>"
  "</html>";
const char bad_request[] =
  "<html>"
  "<head><title>Bad Request</title></head>"
  "<body><h1>400 Bad Request</h1></body>"
  "</html>";
const char unauthorized[] =
  "<html>"
  "<head><title>Unauthorized</title></head>"
  "<body><h1>401 Unauthorized</h1></body>"
  "</html>";
const char forbidden[] =
  "<html>"
  "<head><title>Forbidden</title></head>"
  "<body><h1>403 Forbidden</h1></body>"
  "</html>";
const char not_found[] =
  "<html>"
  "<head><title>Not Found</title></head>"
  "<body><h1>404 Not Found</h1></body>"
  "</html>";
const char internal_server_error[] =
  "<html>"
  "<head><title>Internal Server Error</title></head>"
  "<body><h1>500 Internal Server Error</h1></body>"
  "</html>";
const char not_implemented[] =
  "<html>"
  "<head><title>Not Implemented</title></head>"
  "<body><h1>501 Not Implemented</h1></body>"
  "</html>";
const char bad_gateway[] =
  "<html>"
  "<head><title>Bad Gateway</title></head>"
  "<body><h1>502 Bad Gateway</h1></body>"
  "</html>";
const char service_unavailable[] =
  "<html>"
  "<head><title>Service Unavailable</title></head>"
  "<body><h1>503 Service Unavailable</h1></body>"
  "</html>";

std::string toString(HttpResponse::StatusType status) {
  switch (status) {
  case HttpResponse::ok:
    return ok;
  case HttpResponse::created:
    return created;
  case HttpResponse::accepted:
    return accepted;
  case HttpResponse::no_content:
    return no_content;
  case HttpResponse::multiple_choices:
    return multiple_choices;
  case HttpResponse::moved_permanently:
    return moved_permanently;
  case HttpResponse::moved_temporarily:
    return moved_temporarily;
  case HttpResponse::not_modified:
    return not_modified;
  case HttpResponse::bad_request:
    return bad_request;
  case HttpResponse::unauthorized:
    return unauthorized;
  case HttpResponse::forbidden:
    return forbidden;
  case HttpResponse::not_found:
    return not_found;
  case HttpResponse::internal_server_error:
    return internal_server_error;
  case HttpResponse::not_implemented:
    return not_implemented;
  case HttpResponse::bad_gateway:
    return bad_gateway;
  case HttpResponse::service_unavailable:
    return service_unavailable;
  default:
    return internal_server_error;
  }
}

} // namespace stock_replies

HttpResponse HttpResponse::createReply(HttpResponse::StatusType status) {
  HttpResponse rep;
  rep.status = status;
  rep.setContent(stock_replies::toString(status));
  rep.headers.resize(2);
  rep.headers[1].header_name = "Content-Type";
  rep.headers[1].header_value = "text/html";
  rep.headers[0].header_name = "Content-Length";
  rep.headers[0].header_value = std::to_string(rep.content.size());
  return rep;
}

HttpResponse HttpResponse::createReply(HttpResponse::StatusType status, const std::string& body) {
  HttpResponse rep;
  rep.status = status;
  rep.content = body;
  rep.headers.resize(2);
  rep.headers[0].header_name = "Content-Length";
  rep.headers[0].header_value = std::to_string(rep.content.size());
  rep.headers[1].header_name = "Content-Type";
  rep.headers[1].header_value = "application/json";
  return rep;
}

std::vector<asio::const_buffer> HttpResponse::toBuffers() {
  std::vector<asio::const_buffer> buffers;
  buffers.push_back(status_strings::toBuffer(status));
  for (std::size_t i = 0; i < headers.size(); ++i) {
    HttpHeader& h = headers[i];
    buffers.push_back(asio::buffer(h.header_name));
    buffers.push_back(asio::buffer(misc_strings::name_value_separator));
    buffers.push_back(asio::buffer(h.header_value));
    buffers.push_back(asio::buffer(misc_strings::crlf));
  }
  buffers.push_back(asio::buffer(misc_strings::crlf));
  buffers.push_back(asio::buffer(content));
  return buffers;
}

HttpResponse HttpResponse::stockRedirectRespons(std::string& redUri) {
  HttpResponse rep;
  rep.status = HttpResponse::moved_temporarily;
  rep.content = stock_replies::toString(rep.status);
  int pos = -1;
  std::string holdStr = "example.com";
  while((pos = rep.content.find(holdStr)) != -1) {
    rep.content.replace(pos, holdStr.size(), redUri);
  }
  rep.headers.resize(2);
  rep.headers[0].header_name = "Content-Length";
  rep.headers[0].header_value = std::to_string(rep.content.size());
  rep.headers[1].header_name = "Content-Type";
  rep.headers[1].header_value = "text/html";
  return rep;
}

} // namespace server
} // namespace http
}
