
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "date_expr.h"


using namespace std;
using namespace protobuf;
using namespace idgs::pb;
using namespace idgs::actor;

namespace idgs {
namespace expr {

time_t stringToDateTime(struct tm* datetime, const string& sDateTime, const string& pattern) {
  datetime->tm_year = 0;
  datetime->tm_mon = 0;
  datetime->tm_mday = 0;
  datetime->tm_hour = 0;
  datetime->tm_min = 0;
  datetime->tm_sec = 0;
  datetime->tm_isdst = -1;

  strptime(sDateTime.c_str(), pattern.c_str(), datetime);

  return mktime(datetime);
}

string datetimeToString(const struct tm* datetime, const string& pattern) {
  char fmttime[128];
  strftime(fmttime, 128, pattern.c_str(), datetime);
  return string(fmttime);
}

std::string formatToDate(const struct tm* datetime) {
  string year = to_string(datetime->tm_year + 1900);
  string month = to_string(datetime->tm_mon + 1);
  if (datetime->tm_mon + 1 < 10) {
    month = "0" + month;
  }
  string day = to_string(datetime->tm_mday);
  if (datetime->tm_mday < 10) {
    day = "0" + day;
  }

  string result = year + "-" + month + "-" + day;
  return result;
}

bool FromUnixTimeExpression::parse(const Expr& entryExp, const PbMessagePtr& key, const PbMessagePtr& value) {
  if (entryExp.expression_size() != 2 && entryExp.expression_size() != 1) {
    LOG(ERROR) << "Failed to parse expression. unixtime(bigint unixtime[, string format])";
    return false;
  }

  return parseSubExpression(entryExp, key, value);
}

PbVariant FromUnixTimeExpression::evaluate(ExpressionContext* ctx) const {
  auto value = children[0]->evaluate(ctx);
  if (value.type > 4) {
    LOG(ERROR) << "expression from_unixtime(bigint unixtime[, string format]), unixtime must be a number";
    throw std::invalid_argument("Invalid argument exception");
  }

  string pattern = "%Y-%m-%d %H:%M:%S";
  time_t t = (time_t) value;
  if (children.size() == 2) {
    value = children[1]->evaluate(ctx);
    pattern = (string) value;
  }

  struct tm datetime;
  localtime_r(&t, &datetime);
  string result = datetimeToString(&datetime, pattern);
  return PbVariant(result);
}

bool UnixTimestampExpression::parse(const Expr& entryExp, const PbMessagePtr& key, const PbMessagePtr& value) {
  if (entryExp.expression_size() == 0) {
    return true;
  }

  if (entryExp.expression_size() > 1) {
    LOG(ERROR) << "Failed to parse expression. unix_timestamp([string date]), invalid arguments size";
    return false;
  }

  return parseSubExpression(entryExp, key, value);
}

PbVariant UnixTimestampExpression::evaluate(ExpressionContext* ctx) const {
  time_t t(0);
  if (children.empty()) {
    t = time(NULL);
  } else if (children.size() == 1) {
    struct tm datetime;
    string sDatetime = (string) children[0]->evaluate(ctx);
    t = stringToDateTime(&datetime, sDatetime);
  }

  return PbVariant(t);
}

PbVariant ToDateExpression::evaluate(ExpressionContext* ctx) const {
  string sDatetime = (string) child->evaluate(ctx);
  struct tm datetime;
  stringToDateTime(&datetime, sDatetime);

  string result = formatToDate(&datetime);
  return PbVariant(result);
}

PbVariant YearExpression::evaluate(ExpressionContext* ctx) const {
  string sDatetime = (string) child->evaluate(ctx);
  struct tm datetime;
  stringToDateTime(&datetime, sDatetime);

  return PbVariant(datetime.tm_year + 1900);
}

PbVariant MonthExpression::evaluate(ExpressionContext* ctx) const {
  string sDatetime = (string) child->evaluate(ctx);
  struct tm datetime;
  stringToDateTime(&datetime, sDatetime);

  return PbVariant(datetime.tm_mon + 1);
}

PbVariant DayExpression::evaluate(ExpressionContext* ctx) const {
  string sDatetime = (string) child->evaluate(ctx);
  struct tm datetime;
  stringToDateTime(&datetime, sDatetime);

  return PbVariant(datetime.tm_mday);
}

PbVariant HourExpression::evaluate(ExpressionContext* ctx) const {
  string sDatetime = (string) child->evaluate(ctx);
  struct tm datetime;
  stringToDateTime(&datetime, sDatetime);

  return PbVariant(datetime.tm_hour);
}

PbVariant MinuteExpression::evaluate(ExpressionContext* ctx) const {
  string sDatetime = (string) child->evaluate(ctx);
  struct tm datetime;
  stringToDateTime(&datetime, sDatetime);

  return PbVariant(datetime.tm_min);
}

PbVariant SecondExpression::evaluate(ExpressionContext* ctx) const {
  string sDatetime = (string) child->evaluate(ctx);
  struct tm datetime;
  stringToDateTime(&datetime, sDatetime);

  return PbVariant(datetime.tm_sec);
}

PbVariant WeekOfYearExpression::evaluate(ExpressionContext* ctx) const {
  string sDatetime = (string) child->evaluate(ctx);
  struct tm datetime;
  stringToDateTime(&datetime, sDatetime);

  string fstDayOfyear = std::to_string(datetime.tm_year + 1900) + "-01-01";
  struct tm fstDatetime;
  stringToDateTime(&fstDatetime, fstDayOfyear);

  int dayofweek = datetime.tm_wday;
  int dayofyear = datetime.tm_yday;
  int fstdayofweek = fstDatetime.tm_wday;

  int inv = 0;
  if (fstdayofweek <= dayofweek) {
    inv = dayofweek - fstdayofweek;
  } else {
    inv = 7 - (fstdayofweek - dayofweek);
  }

  int weekofyear = (dayofyear - inv) / 7 + 1;
  if (dayofweek < fstdayofweek) {
    ++ weekofyear;
  }

  return PbVariant(weekofyear);
}

PbVariant DateDiffExpression::evaluate(ExpressionContext* ctx) const {
  string sEndtime = (string) leftChild->evaluate(ctx);
  string sStarttime = (string) rightChild->evaluate(ctx);
  struct tm endtime, starttime;
  time_t end = stringToDateTime(&endtime, sEndtime, "%Y-%m-%d");
  time_t start = stringToDateTime(&starttime, sStarttime, "%Y-%m-%d");

  auto inv = end - start;
  int32_t result = inv / 86400;
  return PbVariant(result);
}

PbVariant DateAddExpression::evaluate(ExpressionContext* ctx) const {
  auto value = rightChild->evaluate(ctx);
  if (value.type > 4) {
    LOG(ERROR) << "expression date_add(string startdate, int days), days must be a integer";
    throw std::invalid_argument("Invalid argument exception");
  }

  static string pattern = "%Y-%m-%d";
  string sStarttime = (string) leftChild->evaluate(ctx);
  size_t days = (size_t) value;

  struct tm starttime;
  time_t timestamp = stringToDateTime(&starttime, sStarttime, pattern);
  time_t addtimestamp = timestamp + days * 86400;

  struct tm datetime;
  localtime_r(&addtimestamp, &datetime);
  string result = formatToDate(&datetime);

  return PbVariant(result);
}

PbVariant DateSubExpression::evaluate(ExpressionContext* ctx) const {
  auto value = rightChild->evaluate(ctx);
  if (value.type > 4) {
    LOG(ERROR) << "expression date_sub(string startdate, int days), days must be a integer";
    throw std::invalid_argument("Invalid argument exception");
  }

  static string pattern = "%Y-%m-%d";
  string sStarttime = (string) leftChild->evaluate(ctx);
  size_t days = (size_t) value;

  struct tm starttime;
  time_t timestamp = stringToDateTime(&starttime, sStarttime, pattern);
  time_t subtimestamp = timestamp - days * 86400;

  struct tm datetime;
  localtime_r(&subtimestamp, &datetime);
  string result = formatToDate(&datetime);

  return PbVariant(result);
}

} // namespace expr
} // namespace idgs 
