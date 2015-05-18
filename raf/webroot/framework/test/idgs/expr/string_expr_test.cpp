/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__)
#include "idgs_gch.h"
#endif // GNUC_ $

#include <gtest/gtest.h>
#include "expr_common.h"
#include "idgs/expr/expression_helper.h"

using namespace std;
using namespace idgs;
using namespace idgs::expr;
using namespace idgs::pb;
using namespace protobuf;

TEST(string_expr, substr) {
  ExpressionFactory::init();
  LOG(INFO) << "test substr(string, start[, length])";

  PbVariant result;
  std::string str = "hello, world";

  LOG(INFO) << "1. test substr(string, start);";
  auto code = exprEvaluate(result, SUBSTR(CONST("hello, world"), CONST("2", idgs::pb::INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(str.substr(2), (string) result);

  LOG(INFO) << "2. test substring(string, start, length);";
  code = exprEvaluate(result, EXPR("SUBSTRING", CONST("hello, world"), CONST("2", idgs::pb::INT32), CONST("3", idgs::pb::INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(str.substr(2, 3), (string) result);

  LOG(INFO) << "3. test exception";
  code = exprEvaluate(result, SUBSTR(CONST("hello, world"), CONST("-2", idgs::pb::INT32)));
  EXPECT_EQ(RC_ERROR, code);

  code = exprEvaluate(result, SUBSTR(CONST("hello, world"), CONST("abc")));
  EXPECT_EQ(RC_ERROR, code);
}

TEST(string_expr, like) {
  LOG(INFO) << "test like(string, like_string)";

  PbVariant result;

  LOG(INFO) << "1. test like both left and right;";
  auto code = exprEvaluate(result, LIKE(CONST("hello, world"), CONST("%llo%")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_TRUE((bool) result);

  code = exprEvaluate(result, LIKE(CONST("hello, world"), CONST("%ool%")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_FALSE((bool) result);

  LOG(INFO) << "2. test left like;";
  code = exprEvaluate(result, LIKE(CONST("hello, world"), CONST("%rld")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_TRUE((bool) result);

  code = exprEvaluate(result, LIKE(CONST("hello, world"), CONST("%llo")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_FALSE((bool) result);

  LOG(INFO) << "3. test right like;";
  code = exprEvaluate(result, LIKE(CONST("hello, world"), CONST("he%")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_TRUE((bool) result);

  code = exprEvaluate(result, LIKE(CONST("hello, world"), CONST("world%")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_FALSE((bool) result);

  LOG(INFO) << "4. test equal like;";
  code = exprEvaluate(result, LIKE(CONST("hello, world"), CONST("hello, world")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_TRUE((bool) result);

  code = exprEvaluate(result, LIKE(CONST("hello, world"), CONST("hello")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_FALSE((bool) result);
}

TEST(string_expr, length) {
  LOG(INFO) << "test length(string)";

  PbVariant result;

  LOG(INFO) << "1. test length of string 'hello, world';";
  string str = "hello, world";
  auto code = exprEvaluate(result, EXPR("LENGTH", CONST("hello, world")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(str.size(), (size_t) result);

  LOG(INFO) << "2. test empty string;";
  code = exprEvaluate(result, EXPR("LENGTH", CONST("")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(0, (size_t) result);
}

TEST(string_expr, reverse) {
  LOG(INFO) << "test length(string)";

  PbVariant result;

  LOG(INFO) << "1. test length of string 'hello, world';";
  auto code = exprEvaluate(result, EXPR("REVERSE", CONST("hello, world")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("dlrow ,olleh", (string) result);

  LOG(INFO) << "2. test empty string;";
  code = exprEvaluate(result, EXPR("REVERSE", CONST("")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("", (string) result);
}

TEST(string_expr, concat) {
  LOG(INFO) << "test concat(string, string ...)";

  PbVariant result;

  LOG(INFO) << "1. test hello + , + world";
  auto code = exprEvaluate(result, EXPR("CONCAT", CONST("hello"), CONST(","), CONST(" world")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("hello, world", (string) result);
}

TEST(string_expr, concat_ws) {
  LOG(INFO) << "test concat_ws(string, string ...)";

  PbVariant result;

  LOG(INFO) << "1. test hello and world with seperate ,";
  auto code = exprEvaluate(result, EXPR("CONCAT_WS", CONST(", "), CONST("hello"), CONST("world")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("hello, world", (string) result);
}

TEST(string_expr, upper) {
  LOG(INFO) << "test upper(string)";

  PbVariant result;

  LOG(INFO) << "1. test upper(hello, world)";
  auto code = exprEvaluate(result, EXPR("UPPER", CONST("hello, world")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("HELLO, WORLD", (string) result);

  LOG(INFO) << "2. test ucase(hello, world)";
  code = exprEvaluate(result, EXPR("UCASE", CONST("hello, world")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("HELLO, WORLD", (string) result);
}

TEST(string_expr, lower) {
  LOG(INFO) << "test lower(string)";

  PbVariant result;

  LOG(INFO) << "1. test lower(HELLO, WORLD)";
  auto code = exprEvaluate(result, EXPR("LOWER", CONST("HELLO, WORLD")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("hello, world", (string) result);

  LOG(INFO) << "2. test lcase(hello, WORLD)";
  code = exprEvaluate(result, EXPR("LCASE", CONST("hello, WORLD")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("hello, world", (string) result);
}

TEST(string_expr, trim) {
  LOG(INFO) << "test trim(string), ltrim(string), rtrim(string)";

  PbVariant result;

  LOG(INFO) << "1. test trim '  hello, world   '";
  auto code = exprEvaluate(result, EXPR("TRIM", CONST("  hello, world   ")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("hello, world", (string) result);

  LOG(INFO) << "2. test ltrim '  hello, world   '";
  code = exprEvaluate(result, EXPR("LTRIM", CONST("  hello, world   ")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("hello, world   ", (string) result);

  LOG(INFO) << "3. test rtrim '  hello, world   '";
  code = exprEvaluate(result, EXPR("RTRIM", CONST("  hello, world   ")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("  hello, world", (string) result);

  LOG(INFO) << "4. test trim space";
  code = exprEvaluate(result, EXPR("RTRIM", CONST("   ")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_TRUE(((string) result).empty());
}

TEST(string_expr, space) {
  LOG(INFO) << "test space(int n)";

  PbVariant result;

  LOG(INFO) << "1. test space(10)";
  auto code = exprEvaluate(result, EXPR("SPACE", CONST("10", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(10, ((string) result).size());

  LOG(INFO) << "2. test error";
  code = exprEvaluate(result, EXPR("SPACE", CONST("abc")));
  EXPECT_EQ(RC_ERROR, code);
}

TEST(string_expr, repeat) {
  LOG(INFO) << "test repeat(string str, int n)";

  PbVariant result;

  LOG(INFO) << "1. test space('abc', 5)";
  auto code = exprEvaluate(result, EXPR("REPEAT", CONST("abc"), CONST("5", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("abcabcabcabcabc", (string) result);

  LOG(INFO) << "2. test error";
  code = exprEvaluate(result, EXPR("REPEAT", CONST("abc"), CONST("abc")));
  EXPECT_EQ(RC_ERROR, code);
}

TEST(string_expr, ascii) {
  LOG(INFO) << "test ascii(string)";

  PbVariant result;

  LOG(INFO) << "1. test ascii(hello, world)";
  auto code = exprEvaluate(result, EXPR("ASCII", CONST("hello, world")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(104, (int32_t) result);
  EXPECT_EQ('h', (char) ((int32_t) result));

  LOG(INFO) << "2. test empty string";
  code = exprEvaluate(result, EXPR("ASCII", CONST("")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(-1, (int32_t) result);
}

TEST(string_expr, pad) {
  LOG(INFO) << "test lpad(string, n[, string]), rpad(string, n[, string])";

  PbVariant result;

  LOG(INFO) << "1. test lpad(hello, world, 20, 0)";
  auto code = exprEvaluate(result, EXPR("LPAD", CONST("hello, world"), CONST("20", INT32), CONST("0")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("00000000hello, world", (string) result);

  LOG(INFO) << "2. test lpad(hello, world, 20)";
  code = exprEvaluate(result, EXPR("LPAD", CONST("hello, world"), CONST("20", INT32), CONST(" ")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("        hello, world", (string) result);

  LOG(INFO) << "3. test rpad(hello, world, 20, 0)";
  code = exprEvaluate(result, EXPR("RPAD", CONST("hello, world"), CONST("20", INT32), CONST("0")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("hello, world00000000", (string) result);

  LOG(INFO) << "4. test error";
  code = exprEvaluate(result, EXPR("RPAD", CONST("hello, world"), CONST("aabc"), CONST("0")));
  EXPECT_EQ(RC_ERROR, code);
}

TEST(string_expr, locate) {
  LOG(INFO) << "test locate(substr, str[, pos])";

  PbVariant result;

  LOG(INFO) << "1. test locate('hello', 'hello, world')";
  auto code = exprEvaluate(result, EXPR("LOCATE", CONST("hello"), CONST("hello, world")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(1, (int32_t) result);

  LOG(INFO) << "2. test locate('world', 'hello, world')";
  code = exprEvaluate(result, EXPR("LOCATE", CONST("world"), CONST("hello, world")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(8, (int32_t) result);

  LOG(INFO) << "3. test locate('world', 'hello, world', 8)";
  code = exprEvaluate(result, EXPR("LOCATE", CONST("world"), CONST("hello, world"), CONST("8", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(8, (int32_t) result);

  LOG(INFO) << "4. test locate('world', 'hello, world', 9)";
  code = exprEvaluate(result, EXPR("LOCATE", CONST("world"), CONST("hello, world"), CONST("9", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(0, (int32_t) result);
}

TEST(string_expr, instr) {
  LOG(INFO) << "test instr(str, substr)";

  PbVariant result;

  LOG(INFO) << "1. test instr('hello, world', 'hello')";
  auto code = exprEvaluate(result, EXPR("INSTR", CONST("hello, world"), CONST("hello")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(1, (int32_t) result);

  LOG(INFO) << "2. test instr('hello, world', 'world')";
  code = exprEvaluate(result, EXPR("INSTR", CONST("hello, world"), CONST("world")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(8, (int32_t) result);

  LOG(INFO) << "3. test instr('world', 'hello, world')";
  code = exprEvaluate(result, EXPR("INSTR", CONST("world"), CONST("hello, world")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(0, (int32_t) result);
}

TEST(string_expr, findinset) {
  LOG(INFO) << "test findinset(string str, string strlist)";

  PbVariant result;

  LOG(INFO) << "1. test findinset('hello', 'hello,world')";
  auto code = exprEvaluate(result, EXPR("FIND_IN_SET", CONST("hello"), CONST("hello,world")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(1, (int32_t) result);

  LOG(INFO) << "2. test findinset('world', 'hello,world')";
  code = exprEvaluate(result, EXPR("FIND_IN_SET", CONST("world"), CONST("hello,world")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(2, (int32_t) result);

  LOG(INFO) << "3. test findinset('llo', 'hello,world')";
  code = exprEvaluate(result, EXPR("FIND_IN_SET", CONST("llo"), CONST("hello,world")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(0, (int32_t) result);
}

TEST(string_expr, parse_url) {
  LOG(INFO) << "test parse_url(string a, string b)";

  PbVariant result;

  LOG(INFO) << "1. test parse_url('http://root@www.intel.com/news/index.html?key=1&value=2#ref', 'HOST')";
  auto code = exprEvaluate(result, EXPR("PARSE_URL", CONST("http://root@www.intel.com/news/index.html?key=1&value=2#ref"), CONST("HOST")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("www.intel.com", (string) result);

  LOG(INFO) << "2. test parse_url('http://root@www.intel.com/news/index.html?key=1&value=2#ref', 'PATH')";
  code = exprEvaluate(result, EXPR("PARSE_URL", CONST("http://root@www.intel.com/news/index.html?key=1&value=2#ref"), CONST("PATH")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("/news/index.html", (string) result);

  LOG(INFO) << "3. test parse_url('http://root@www.intel.com/news/index.html?key=1&value=2#ref', 'QUERY')";
  code = exprEvaluate(result, EXPR("PARSE_URL", CONST("http://root@www.intel.com/news/index.html?key=1&value=2#ref"), CONST("QUERY")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("key=1&value=2", (string) result);

  LOG(INFO) << "4. test parse_url('http://root@www.intel.com/news/index.html?key=1&value=2#ref', 'QUERY', 'value')";
  code = exprEvaluate(result, EXPR("PARSE_URL", CONST("http://root@www.intel.com/news/index.html?key=1&value=2#ref"), CONST("QUERY"), CONST("value")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("2", (string) result);

  LOG(INFO) << "5. test parse_url('http://root@www.intel.com/news/index.html?key=1&value=2#ref', 'REF')";
  code = exprEvaluate(result, EXPR("PARSE_URL", CONST("http://root@www.intel.com/news/index.html?key=1&value=2#ref"), CONST("REF")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("ref", (string) result);

  LOG(INFO) << "6. test parse_url('http://root@www.intel.com/news/index.html?key=1&value=2#ref', 'PROTOCOL')";
  code = exprEvaluate(result, EXPR("PARSE_URL", CONST("http://root@www.intel.com/news/index.html?key=1&value=2#ref"), CONST("PROTOCOL")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("http", (string) result);

  LOG(INFO) << "7. test parse_url('http://root@www.intel.com/news/index.html?key=1&value=2#ref', 'FILE')";
  code = exprEvaluate(result, EXPR("PARSE_URL", CONST("http://root@www.intel.com/news/index.html?key=1&value=2#ref"), CONST("FILE")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("/news/index.html?key=1&value=2", (string) result);

  LOG(INFO) << "8. test parse_url('http://root@www.intel.com/news/index.html?key=1&value=2#ref', 'AUTHORITY')";
  code = exprEvaluate(result, EXPR("PARSE_URL", CONST("http://root@www.intel.com/news/index.html?key=1&value=2#ref"), CONST("AUTHORITY")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("root@www.intel.com", (string) result);

  LOG(INFO) << "9. test parse_url('http://root@www.intel.com/news/index.html?key=1&value=2#ref', 'USERINFO')";
  code = exprEvaluate(result, EXPR("PARSE_URL", CONST("http://root@www.intel.com/news/index.html?key=1&value=2#ref"), CONST("USERINFO")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("root", (string) result);
}

TEST(string_expr, regexp) {
  LOG(INFO) << "test regexp(string str, string pattern), regexp_replace(string str, string pattern, string replcement), regexp_extract(string str, string pattern[, int pos])";

  PbVariant result;

  LOG(INFO) << "1. test regexp('A0B12C345D678E90F', '[0-9]')";
  auto code = exprEvaluate(result, EXPR("REGEXP", CONST("A0B12C345D678E90F"), CONST("[0-9]")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_TRUE((bool) result);

  LOG(INFO) << "2. test rlike('A0B12C345D678E90F', '[a-z]')";
  code = exprEvaluate(result, EXPR("RLIKE", CONST("A0B12C345D678E90F"), CONST("[a-z]")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_FALSE((bool) result);

  LOG(INFO) << "3. test regexp_replace('A0B12C345D678E90F', '[a-z]', '')";
  code = exprEvaluate(result, EXPR("REGEXP_REPLACE", CONST("A0B12C345D678E90F"), CONST("[0-9]"), CONST("")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("ABCDEF", (string) result);

  LOG(INFO) << "3. test regexp_extract('foothebar', 'foo(.*?)(bar)')";
  code = exprEvaluate(result, EXPR("REGEXP_EXTRACT", CONST("foothebar"), CONST("foo(.*?)(bar)")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("the", (string) result);

  LOG(INFO) << "4. test regexp_extract('foothebar', 'foo(.*?)(bar)', 2)";
  code = exprEvaluate(result, EXPR("REGEXP_EXTRACT", CONST("foothebar"), CONST("foo(.*?)(bar)"), CONST("2", INT32)));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("bar", (string) result);
}

TEST(string_expr, get_json_object) {
  LOG(INFO) << "test get_json_object(string json, string path)";

  PbVariant result;

  string json = "{\"name\":{\"first\":\"first name\", \"last\":\"last name\", \"middle\":\"middle name\"}}";

  LOG(INFO) << "1. test get_json_object('" << json << "', '$')";
  auto code = exprEvaluate(result, EXPR("GET_JSON_OBJECT", CONST(json.c_str()), CONST("$")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ(json, (string) result);

  LOG(INFO) << "2. test get_json_object('" << json << "', '$.name')";
  code = exprEvaluate(result, EXPR("GET_JSON_OBJECT", CONST(json.c_str()), CONST("$.name")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("{first=first namelast=last namemiddle=middle name}", (string) result);

  LOG(INFO) << "3. test get_json_object('" << json << "', '$.name1')";
  code = exprEvaluate(result, EXPR("GET_JSON_OBJECT", CONST(json.c_str()), CONST("$.name1")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("null", (string) result);

  LOG(INFO) << "4. test get_json_object('" << json << "', '$.name.middle')";
  code = exprEvaluate(result, EXPR("GET_JSON_OBJECT", CONST(json.c_str()), CONST("$.name.middle")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("middle name", (string) result);

  LOG(INFO) << "5. test get_json_object('" << json << "', '$.name.middle.last')";
  code = exprEvaluate(result, EXPR("GET_JSON_OBJECT", CONST(json.c_str()), CONST("$.name.middle.last")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("null", (string) result);

  json = "{\"name\":[\"first name\", \"last name\", \"middle name\"]}";
  LOG(INFO) << "6. test get_json_object('" << json << "', '$.name[1]')";
  code = exprEvaluate(result, EXPR("GET_JSON_OBJECT", CONST(json.c_str()), CONST("$.name[1]")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("last name", (string) result);

  LOG(INFO) << "7. test get_json_object('" << json << "', '$.name[4]')";
  code = exprEvaluate(result, EXPR("GET_JSON_OBJECT", CONST(json.c_str()), CONST("$.name[4]")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("null", (string) result);

  LOG(INFO) << "8. test get_json_object('" << json << "', '$.name[abc]')";
  code = exprEvaluate(result, EXPR("GET_JSON_OBJECT", CONST(json.c_str()), CONST("$.name[abc]")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("null", (string) result);

  json = "[{\"examination\":{\"Math\":80,\"English\":90}}, {\"examination\":{\"Math\":75,\"English\":95}}]";
  LOG(INFO) << "9. test get_json_object('" << json << "', '$[1].examination.English')";
  code = exprEvaluate(result, EXPR("GET_JSON_OBJECT", CONST(json.c_str()), CONST("$[1].examination.English")));
  EXPECT_EQ(RC_SUCCESS, code);
  EXPECT_EQ("95", (string) result);
}
