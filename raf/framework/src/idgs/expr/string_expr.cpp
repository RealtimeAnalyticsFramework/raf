
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "string_expr.h"
#include <regex.h>
#include <algorithm>

#include "idgs/util/utillity.h"

using namespace std;
using namespace protobuf;
using namespace idgs::pb;
using namespace idgs::actor;

namespace idgs {
namespace expr {

bool SubStrExpression::parse(const Expr& entryExp, const PbMessagePtr& key, const PbMessagePtr& value) {
  if (entryExp.expression_size() != 3 && entryExp.expression_size() != 2) {
    LOG(ERROR) << "Failed to parse expression. SUBSTR(source_string, start_position, [length])";
    return false;
  }

  return parseSubExpression(entryExp, key, value);
}

protobuf::PbVariant SubStrExpression::evaluate(ExpressionContext* ctx) const {
  auto value = children[0]->evaluate(ctx);
  std::string str = (std::string) value;

  auto expr_value = children[1]->evaluate(ctx);
  if (expr_value.type > 4) {
    LOG(ERROR) << "expression substr(string, start[, length]), start must be a integer";
    throw std::invalid_argument("Invalid argument exception");
  }

  size_t pos = (size_t) expr_value;
  if (children.size() == 2) {
    return protobuf::PbVariant(str.substr(pos));
  }

  expr_value = children[2]->evaluate(ctx);
  if (expr_value.type > 4) {
    LOG(ERROR) << "expression substr(string, start[, length]), length must be a integer";
    throw std::invalid_argument("Invalid argument exception");
  }
  size_t len = (size_t) expr_value;
  return protobuf::PbVariant(str.substr(pos, len));
}

PbVariant LikeExpression::evaluate(ExpressionContext* ctx) const {
  PbVariant lvar = leftChild->evaluate(ctx);
  PbVariant rvar = rightChild->evaluate(ctx);
  string lstr = (string) lvar;
  string rstr = (string) rvar;

  bool result = false;
  if (rstr == "") {
    result = (lstr == rstr);
  } else if (rstr == "%" || rstr == "%%") {
    result = true;
  } else {
    char fst = rstr.c_str()[0];
    char lst = rstr.c_str()[rstr.size() - 1];

    if (fst == '%' && lst == '%') {
      rstr = rstr.substr(1, rstr.size() - 2);
      result = (lstr.find(rstr) != std::string::npos);
    } else if (fst == '%') {
      rstr = rstr.substr(1, rstr.size() - 1);
      result = (lstr.compare(lstr.size() - rstr.size(), rstr.size(), rstr) == 0);
    } else if (lst == '%') {
      rstr = rstr.substr(0, rstr.size() - 1);
      result = (lstr.compare(0, rstr.size(), rstr) == 0);
    } else {
      result = (lstr == rstr);
    }
  }

  return PbVariant(result);
}

PbVariant LengthExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  string result = (string) value;
  return PbVariant(result.size());
}

PbVariant ReverseExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  string result = (string) value;
  reverse(result.begin(), result.end());
  return PbVariant(result);
}

PbVariant ConcatExpression::evaluate(ExpressionContext* ctx) const {
  string result = "";
  for (auto& expr : children) {
    auto value = expr->evaluate(ctx);
    result.append((string) value);
  }
  return PbVariant(result);
}

PbVariant ConcatWSExpression::evaluate(ExpressionContext* ctx) const {
  string result = "";
  if (children.size() > 1) {
    auto value = children[0]->evaluate(ctx);
    string seq = (string) value;

    value = children[1]->evaluate(ctx);
    result = (string) value;
    for (int32_t i = 2; i < children.size(); ++ i) {
      value = children[i]->evaluate(ctx);
      result.append(seq).append((string) value);
    }
  }
  return PbVariant(result);
}

PbVariant UpperExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  string result = (string) value;
  std::transform(result.begin(), result.end(), result.begin(), ::toupper);
  return PbVariant(result);
}

PbVariant LowerExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  string result = (string) value;
  std::transform(result.begin(), result.end(), result.begin(), ::tolower);
  return PbVariant(result);
}

PbVariant TrimExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  string result = (string) value;
  if (!result.empty()) {
    size_t beg = result.find_first_not_of(" \a\b\f\n\r\t\v");
    size_t end = result.find_last_not_of(" \a\b\f\n\r\t\v");
    if (beg == std::string::npos) {
      result = "";
    } else {
      result = string(result, beg, end - beg + 1);
    }
  }
  return PbVariant(result);
}

PbVariant LTrimExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  string result = (string) value;
  if (!result.empty()) {
    size_t beg = result.find_first_not_of(" \a\b\f\n\r\t\v");
    size_t end = result.size() - 1;;
    if (beg == std::string::npos) {
      result = "";
    } else {
      result = string(result, beg, end - beg + 1);
    }
  }
  return PbVariant(result);
}

PbVariant RTrimExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  string result = (string) value;
  if (!result.empty()) {
    size_t end = result.find_last_not_of(" \a\b\f\n\r\t\v");
    if (end == std::string::npos) {
      result = "";
    } else {
      result = string(result, 0, end + 1);
    }
  }
  return PbVariant(result);
}

PbVariant SpaceExpression::evaluate(ExpressionContext* ctx) const {
  auto value = child->evaluate(ctx);
  if (value.type > 4) {
    LOG(ERROR) << "expression space(n), n must be a integer";
    throw std::invalid_argument("Invalid argument exception");
  }

  size_t n = (size_t) value;
  string result(n, ' ');
  return PbVariant(result);
}

PbVariant RepeatExpression::evaluate(ExpressionContext* ctx) const {
  auto value = rightChild->evaluate(ctx);
  if (value.type > 4) {
    LOG(ERROR) << "expression repeat(string, n), n must be a integer";
    throw std::invalid_argument("Invalid argument exception");
  }

  size_t n = (size_t) value;
  value = leftChild->evaluate(ctx);
  string str = (string) value;

  string result("");
  for (size_t i = 0; i < n; ++ i) {
    result.append(str);
  }
  return PbVariant(result);
}

PbVariant AsciiExpression::evaluate(ExpressionContext* ctx) const {
  string value = (string) child->evaluate(ctx);
  int32_t result = (value.empty()) ? -1 : value.c_str()[0];
  return PbVariant(result);
}

PbVariant LPadExpression::evaluate(ExpressionContext* ctx) const {
  auto value = middleChild->evaluate(ctx);
  if (value.type > 4) {
    LOG(ERROR) << "expression lpad(string, n, string), n must be a integer";
    throw std::invalid_argument("Invalid argument exception");
  }

  size_t n = (size_t) value;

  value = leftChild->evaluate(ctx);
  string result = (string) value;

  if (n < result.size()) {
    result = result.substr(0, n);
  } else if (n > result.size()) {
    size_t padLen = n - result.size();
    string pad = (string) rightChild->evaluate(ctx);
    string pads = "";
    while(pads.size() < padLen) {
      pads = pad + pads;
    }

    pads = pads.substr(0, padLen);
    result = pads + result;
  }

  return PbVariant(result);
}

PbVariant RPadExpression::evaluate(ExpressionContext* ctx) const {
  auto value = middleChild->evaluate(ctx);
  if (value.type > 4) {
    LOG(ERROR) << "expression rpad(string, n, string), n must be a integer";
    throw std::invalid_argument("Invalid argument exception");
  }

  size_t n = (size_t) value;

  value = leftChild->evaluate(ctx);
  string result = (string) value;

  if (n < result.size()) {
    result = result.substr(0, n);
  } else if (n > result.size()) {
    string pad = (string) rightChild->evaluate(ctx);
    while(result.size() < n) {
      result = result + pad;
    }

    result = result.substr(0, n);
  }

  return PbVariant(result);
}

bool LocateExpression::parse(const Expr& entryExp, const PbMessagePtr& key, const PbMessagePtr& value) {
  if (entryExp.expression_size() != 3 && entryExp.expression_size() != 2) {
    LOG(ERROR) << "Failed to parse expression. SUBSTR(source_string, start_position, [length])";
    return false;
  }

  return parseSubExpression(entryExp, key, value);
}

PbVariant LocateExpression::evaluate(ExpressionContext* ctx) const {
  string substr = (string) children[0]->evaluate(ctx);
  string str = (string) children[1]->evaluate(ctx);
  int64_t pos = 1;

  if (children.size() == 3) {
    auto posValue = children[2]->evaluate(ctx);
    if (posValue.type > 4) {
      LOG(ERROR) << "expression locate(substr, str[, pos]), pos must be a integer";
      throw std::invalid_argument("Invalid argument exception");
    }
    pos = (int64_t) posValue;
  }

  size_t result = (pos < 1) ? 0 : str.find(substr, pos - 1) + 1;
  return protobuf::PbVariant(result);
}

PbVariant InStrExpression::evaluate(ExpressionContext* ctx) const {
  string str = (string) leftChild->evaluate(ctx);
  string substr = (string) rightChild->evaluate(ctx);

  size_t result = str.find(substr) + 1;
  return protobuf::PbVariant(result);
}

PbVariant FindInSetExpression::evaluate(ExpressionContext* ctx) const {
  string str = (string) leftChild->evaluate(ctx);
  string strlist = (string) rightChild->evaluate(ctx);

  int32_t result = 0;
  if (strlist.empty()) {
    result = 0;
  } else if (str.find_first_of(",") != string::npos) {
    result = -1;
  } else {
    str = "," + str + ",";
    if (strlist.c_str()[0] != ',') {
      strlist = "," + strlist;
    }

    if (strlist.c_str()[strlist.size() - 1] != ',') {
      strlist = strlist + ",";
    }

    int32_t pos = strlist.find(str);
    if (pos != std::string::npos) {
      int32_t cpos = -1;
      while (cpos != pos) {
        cpos = strlist.find(",", cpos + 1);
        ++ result;
      }
    }
  }

  return PbVariant(result);
}

bool ParseURLExpression::parse(const Expr& entryExp, const PbMessagePtr& key, const PbMessagePtr& value) {
  if (entryExp.expression_size() != 3 && entryExp.expression_size() != 2) {
    LOG(ERROR) << "Failed to parse expression. parse_url(source_string, option, [query_key])";
    return false;
  }

  return parseSubExpression(entryExp, key, value);
}

PbVariant ParseURLExpression::evaluate(ExpressionContext* ctx) const {
  string url = children[0]->evaluate(ctx);
  string option = children[1]->evaluate(ctx);

  auto pos = url.find("://");
  if (pos == std::string::npos) {
    return PbVariant(string("invalid URL"));
  }

  std::string result = "";

  auto pos_user = url.find("@", pos + 3);
  auto pos_path = url.find("/", pos + 3);
  auto pos_query = url.find("?", pos + 3);
  auto pos_ref = url.find("#", pos + 3);

  if (option == "HOST") {
    auto start = (pos_user == std::string::npos) ? pos + 3 : pos_user + 1;
    auto end = (pos_path == std::string::npos) ? ((pos_query == std::string::npos) ? ((pos_ref == std::string::npos) ? url.size() : pos_ref) : pos_query) : pos_path;
    result = url.substr(start, end - start);
  } else if (option == "PATH") {
    if (pos_path == std::string::npos) {
      result = "/";
    } else {
      auto end = (pos_query == std::string::npos) ? ((pos_ref == std::string::npos) ? url.size() : pos_ref) : pos_query;
      result = url.substr(pos_path, end - pos_path);
    }
  } else if (option == "QUERY") {
    if (pos_query == std::string::npos) {
      result = "";
    } else {
      auto end = (pos_ref == std::string::npos) ? url.size() : pos_ref;
      result = url.substr(pos_query + 1, end - pos_query - 1);
    }

    if (children.size() == 3) {
      string key = children[2]->evaluate(ctx);
      auto start = result.find(key);
      if (start == std::string::npos) {
        result = "";
      } else {
        start = result.find("=", start + 1);
        auto end = result.find("&", start + 1);
        result = (end == std::string::npos) ? result.substr(start + 1) : result.substr(start + 1, end - start - 1);
      }
    }
  } else if (option == "REF") {
    result = (pos_ref == std::string::npos) ? "": url.substr(pos_ref + 1);
  } else if (option == "PROTOCOL") {
    result = url.substr(0, pos);
  } else if (option == "FILE") {
    auto start = (pos_path == std::string::npos) ? ((pos_query == std::string::npos) ? url.size() : pos_query) : pos_path;
    auto end = (pos_ref == std::string::npos) ? url.size() : pos_ref;
    result = url.substr(start, end - start);
  } else if (option == "AUTHORITY") {
    auto end = (pos_path == std::string::npos) ? ((pos_query == std::string::npos) ? ((pos_ref == std::string::npos) ? url.size() : pos_ref) : pos_query) : pos_path;
    result = url.substr(pos + 3, end - pos - 3);
  } else if (option == "USERINFO") {
    result = (pos_user == std::string::npos) ? "" : url.substr(pos + 3, pos_user - pos - 3);
  }

  return PbVariant(result);
}

bool GetJsonObjectExpression::parsePath(std::string& path, int32_t& index) {
  index = -1;
  auto start = path.find("[");
  auto end = path.find("]");
  if (start != string::npos && end != string::npos) {
    if (end != path.size() - 1) {
      return false;
    }

    auto sIndex = path.substr(start + 1, end - start - 1);
    if (sIndex == "*") {
      path = path.substr(0, start);
      return true;
    }

    std::stringstream ss(sIndex);
    if (!(ss >> index)) {
      return false;
    }
    path = path.substr(0, start);
    return true;
  } else if (start == string::npos && end == string::npos) {
    return true;
  } else {
    return false;
  }
}

std::string GetJsonObjectExpression::toJson(const yajl_val& node) {
  if (node == NULL) {
    return "null";
  }

  if (YAJL_IS_OBJECT(node)) {
    string result("{");
    for (int32_t i = 0; i < node->u.object.len; ++ i) {
      string keys(node->u.object.keys[i]);
      result += keys + "=" + toJson(node->u.object.values[i]);
    }
    result += "}";
    return result;
  } else if (YAJL_IS_ARRAY(node)) {
    string result("[");
    for (int32_t i = 0; i < node->u.array.len; ++ i) {
      if (i > 0) {
        result += ",";
      }
      result += toJson(node->u.array.values[i]);
    }
    result += "]";
    return result;
  } else if (YAJL_IS_TRUE(node)) {
    return "true";
  } else if (YAJL_IS_FALSE(node)) {
    return "false";
  } else if (YAJL_IS_NUMBER(node)) {
    return string(node->u.number.r);
  } else if (YAJL_IS_STRING(node)) {
    return string(node->u.string);
  } else {
    return "";
  }
}

yajl_val GetJsonObjectExpression::extract(const yajl_val& node, const std::string& path) {
  if (path == "$" || path == "R") {
    return node;
  }

  string key = path;
  int index = -1;
  if (!parsePath(key, index)) {
    return NULL;
  }

  if (YAJL_IS_OBJECT(node)) {
    for (int32_t i = 0; i < node->u.object.len; ++ i) {
      string nodekey(node->u.object.keys[i]);
      auto nd = node->u.object.values[i];

      if (nodekey == key) {
        if (YAJL_IS_ARRAY(nd) && index >= 0) {
          nd = (index < nd->u.array.len) ? nd->u.array.values[index] : NULL;
        } else if (YAJL_IS_OBJECT(nd) && index > -1) {
          nd = NULL;
        } else if (YAJL_IS_ARRAY(nd) && index == -1) {
          nd = NULL;
        }

        return nd;
      }
    }
  } else if (YAJL_IS_ARRAY(node) && index > 0) {
    return (index < node->u.array.len) ? node->u.array.values[index] : NULL;
  }

  return NULL;
}

PbVariant GetJsonObjectExpression::evaluate(ExpressionContext* ctx) const {
  static string null = "null";
  auto jsonValue = leftChild->evaluate(ctx);
  string json = jsonValue;
  auto pathValue = rightChild->evaluate(ctx);
  string path = (string) pathValue;

  if (json.empty()) {
    return jsonValue;
  }

  if (path == "$" || path == "R" || path.empty()) {
    return jsonValue;
  }

  char begin = * path.begin();
  if (begin != '$' && begin != 'R') {
    LOG(ERROR) << "invalid json path.";
    return PbVariant(null);
  }

  // parse json string
  char err[255];
  yajl_val root = yajl_tree_parse(json.c_str(), err, 255);
  // whether occur error
  if (strcmp(err, "") != 0) {
    LOG(ERROR) << err;
    yajl_tree_free(root);
    return PbVariant(null);
  }

  string result;
  vector<string> fields;
  str::split(path, ".", fields);
  yajl_val node = root;

  for (int32_t i = 0; i < fields.size(); ++ i) {
    node = extract(node, fields[i]);
    if (node == NULL) {
      break;
    }
  }

  result = toJson(node);

  yajl_tree_free(root);

  return PbVariant(result);
}

PbVariant RegExpExpression::evaluate(ExpressionContext* ctx) const {
  string value = leftChild->evaluate(ctx);
  string pattern = rightChild->evaluate(ctx);

  regmatch_t p[1];
  regex_t reg;
  bool result = false;
  auto ret = regcomp(&reg, pattern.c_str(), REG_EXTENDED);
  if (ret == REG_NOERROR) {
    ret = regexec(&reg, value.c_str(), 1, p, 0);
    result = (ret == REG_NOERROR);
  }

  regfree(&reg);

  return PbVariant(result);
}

PbVariant RegExpReplaceExpression::evaluate(ExpressionContext* ctx) const {
  string result = leftChild->evaluate(ctx);
  string pattern = middleChild->evaluate(ctx);
  string replace = rightChild->evaluate(ctx);
  string value = result;

  regmatch_t p[1];
  regex_t reg;
  auto ret = regcomp(&reg, pattern.c_str(), REG_EXTENDED);
  if (ret == REG_NOERROR) {
    while (regexec(&reg, value.c_str(), 1, p, 0) == REG_NOERROR) {
      string replaced = value.substr(p[0].rm_so, p[0].rm_eo - p[0].rm_so);
      auto pos = result.find(replaced);
      result.replace(pos, replaced.size(), replace);

      if (p[0].rm_eo >= value.size()) {
        break;
      }
      value = value.substr(p[0].rm_eo);
    }
  }

  regfree(&reg);

  return PbVariant(result);
}

bool RegExpExtractExpression::parse(const Expr& entryExp, const PbMessagePtr& key, const PbMessagePtr& value) {
  if (entryExp.expression_size() != 3 && entryExp.expression_size() != 2) {
    LOG(ERROR) << "Failed to parse expression. regex_extract(source_string, regex[, start_position])";
    return false;
  }

  return parseSubExpression(entryExp, key, value);
}

PbVariant RegExpExtractExpression::evaluate(ExpressionContext* ctx) const {
  int32_t pos = 1;
  if (children.size() == 3) {
    auto position = children[2]->evaluate(ctx);
    if (position.type > 4) {
      LOG(ERROR) << "expression regex_extract(source_string, regex[, start_position]), start_position must be a number";
      throw std::invalid_argument("Invalid argument exception");
    }
    pos = (int32_t) position;
  }

  string value = (string) children[0]->evaluate(ctx);
  string pattern = (string) children[1]->evaluate(ctx);

  string result = "";
  int32_t size = pos + 1;
  regmatch_t p[size];
  regex_t reg;
  auto ret = regcomp(&reg, pattern.c_str(), REG_EXTENDED);
  if (ret == REG_NOERROR) {
    ret = regexec(&reg, value.c_str(), size, p, 0);
    if (ret == REG_NOERROR) {
      if (p[pos].rm_so != -1 && p[pos].rm_eo != -1) {
        result = value.substr(p[pos].rm_so, p[pos].rm_eo - p[pos].rm_so);
      }
    }
  }

  regfree(&reg);

  return PbVariant(result);
}


} // namespace expr
} // namespace idgs 
