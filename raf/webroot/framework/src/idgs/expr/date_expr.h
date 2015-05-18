
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "expression.h"

namespace idgs {
namespace expr {

inline time_t stringToDateTime(struct tm* datetime, const std::string& szTime, const std::string& pattern = "%Y-%m-%d %H:%M:%S");
inline std::string datetimeToString(const struct tm* datetime, const std::string& pattern = "%Y-%m-%d %H:%M:%S");
inline std::string formatToDate(const struct tm* datetime);

/// FromUnixTime expression
/// To handle expression from_unixtime(bigint unixtime[, string format])
class FromUnixTimeExpression: public NAryExpression, public idgs::util::CloneEnabler<FromUnixTimeExpression, Expression> {
public:

  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual bool parse(const idgs::pb::Expr& entryExp, const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) override;

  virtual const std::string& name() override {
    static std::string name("FROM_UNIXTIME");
    return name;
  }
};

/// UnixTime expression
/// To handle expression unix_timestamp([string date])
class UnixTimestampExpression: public NAryExpression, public idgs::util::CloneEnabler<UnixTimestampExpression, Expression> {
public:

  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual bool parse(const idgs::pb::Expr& entryExp, const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) override;

  virtual const std::string& name() override {
    static std::string name("UNIX_TIMESTAMP");
    return name;
  }

  virtual const std::string* alias() override {
    static std::string alias("TIMESTAMP");
    return &alias;
  }
};

/// ToDate expression
/// To handle expression to_date(string date)
class ToDateExpression: public UnaryExpression, public idgs::util::CloneEnabler<ToDateExpression, Expression> {
public:

  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("TO_DATE");
    return name;
  }
};

/// Year expression
/// To handle expression year(string date)
class YearExpression: public UnaryExpression, public idgs::util::CloneEnabler<YearExpression, Expression> {
public:

  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("YEAR");
    return name;
  }
};

/// Month expression
/// To handle expression month(string date)
class MonthExpression: public UnaryExpression, public idgs::util::CloneEnabler<MonthExpression, Expression> {
public:

  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("MONTH");
    return name;
  }
};

/// Day expression
/// To handle expression day(string date), dayofmonth(string date)
class DayExpression: public UnaryExpression, public idgs::util::CloneEnabler<DayExpression, Expression> {
public:

  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("DAY");
    return name;
  }

  virtual const std::string* alias() override {
    static std::string alias("DAYOFMONTH");
    return &alias;
  }
};

/// Hour expression
/// To handle expression hour(string date)
class HourExpression: public UnaryExpression, public idgs::util::CloneEnabler<HourExpression, Expression> {
public:

  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("HOUR");
    return name;
  }
};

/// Minute expression
/// To handle expression minute(string date)
class MinuteExpression: public UnaryExpression, public idgs::util::CloneEnabler<MinuteExpression, Expression> {
public:

  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("MINUTE");
    return name;
  }
};

/// Second expression
/// To handle expression second(string date)
class SecondExpression: public UnaryExpression, public idgs::util::CloneEnabler<SecondExpression, Expression> {
public:

  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("SECOND");
    return name;
  }
};

/// WeekOfYear expression
/// To handle expression weekofyear(string date)
class WeekOfYearExpression: public UnaryExpression, public idgs::util::CloneEnabler<WeekOfYearExpression, Expression> {
public:

  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("WEEKOFYEAR");
    return name;
  }
};

/// DateDiff expression
/// To handle expression datediff(string enddate, string startdate)
class DateDiffExpression: public BinaryExpression, public idgs::util::CloneEnabler<DateDiffExpression, Expression> {
public:

  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("DATEDIFF");
    return name;
  }
};

/// DateAdd expression
/// To handle expression date_add(string startdate, int days)
class DateAddExpression: public BinaryExpression, public idgs::util::CloneEnabler<DateAddExpression, Expression> {
public:

  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("DATE_ADD");
    return name;
  }
};

/// DateSub expression
/// To handle expression date_sub(string startdate, int days)
class DateSubExpression: public BinaryExpression, public idgs::util::CloneEnabler<DateSubExpression, Expression> {
public:

  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("DATE_SUB");
    return name;
  }
};

} // namespace expr
} // namespace idgs 
