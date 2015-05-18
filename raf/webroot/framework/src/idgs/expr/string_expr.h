
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "expression.h"
extern "C" {
}

namespace idgs {
namespace expr {

/// SubStr expression
/// To handle expression substr(string, startpos[, length]), substring(string, startpos[, length].
class SubStrExpression: public NAryExpression, public idgs::util::CloneEnabler<SubStrExpression, Expression> {
public:

  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual bool parse(const idgs::pb::Expr& entryExp, const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) override;

  virtual const std::string& name() override {
    static std::string name("SUBSTR");
    return name;
  }

  virtual const std::string* alias() override {
    static std::string alias = {"SUBSTRING"};
    return &alias;
  }
};

/// Like expression
/// To handle operator 'like'.
class LikeExpression: public BinaryExpression, public idgs::util::CloneEnabler<LikeExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("LIKE");
    return name;
  }
};

/// Length of string expression
/// To handle expression length(string).
class LengthExpression: public UnaryExpression, public idgs::util::CloneEnabler<LengthExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("LENGTH");
    return name;
  }
};

/// Reverse expression
/// To handle expression reverse(string).
class ReverseExpression: public UnaryExpression, public idgs::util::CloneEnabler<ReverseExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("REVERSE");
    return name;
  }
};

/// Concat expression
/// To handle expression concat(string, string...).
/// concat one or more string to a new string
class ConcatExpression: public NAryExpression, public idgs::util::CloneEnabler<ConcatExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("CONCAT");
    return name;
  }
};

/// ConcatWS expression
/// concat string with separator
/// concat_ws(SEP, str1, str2...) e.g. concat_ws(",", "abc", "de", "f") -> "abc,de,f"
class ConcatWSExpression: public NAryExpression, public idgs::util::CloneEnabler<ConcatWSExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("CONCAT_WS");
    return name;
  }
};

/// Upper expression
/// to handle expression upper(string), ucase(string)
class UpperExpression: public UnaryExpression, public idgs::util::CloneEnabler<UpperExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("UPPER");
    return name;
  }

  virtual const std::string* alias() override {
    static std::string alias = {"UCASE"};
    return &alias;
  }
};

/// Lower expression
/// to handle expression lower(string), lcase(string)
class LowerExpression: public UnaryExpression, public idgs::util::CloneEnabler<LowerExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("LOWER");
    return name;
  }

  virtual const std::string* alias() override {
    static std::string alias = {"LCASE"};
    return &alias;
  }
};

/// Trim expression
/// to handle expression trim(string)
class TrimExpression: public UnaryExpression, public idgs::util::CloneEnabler<TrimExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() {
    static std::string name("TRIM");
    return name;
  }
};

/// LTrim expression
/// to handle expression ltrim(string)
class LTrimExpression: public UnaryExpression, public idgs::util::CloneEnabler<LTrimExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("LTRIM");
    return name;
  }
};

/// RTrim expression
/// to handle expression rtrim(string)
class RTrimExpression: public UnaryExpression, public idgs::util::CloneEnabler<RTrimExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("RTRIM");
    return name;
  }
};

/// Space expression
/// to handle expression space(int n)
/// return N spaces
class SpaceExpression: public UnaryExpression, public idgs::util::CloneEnabler<SpaceExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("SPACE");
    return name;
  }
};

/// Repeat expression
/// to handle expression repeat(string str, int n)
/// return n strs
class RepeatExpression: public BinaryExpression, public idgs::util::CloneEnabler<RepeatExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("REPEAT");
    return name;
  }
};

/// Ascii expression
/// to handle expression ascii(string)
/// return ascii of the first char of string
class AsciiExpression: public UnaryExpression, public idgs::util::CloneEnabler<AsciiExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("ASCII");
    return name;
  }
};

/// LPad expression
/// to handle expression lpad(string, int n, string pad))
/// return a string with length n, if length of string less then n, fill with pad in left
class LPadExpression: public TernaryExpression, public idgs::util::CloneEnabler<LPadExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("LPAD");
    return name;
  }
};

/// RPad expression
/// to handle expression rpad(string, int n, string pad))
/// return a string with length n, if length of string less then n, fill with pad in right
class RPadExpression: public TernaryExpression, public idgs::util::CloneEnabler<RPadExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("RPAD");
    return name;
  }
};

/// Locate expression
/// to handle expression locate(substr, str[, pos])
class LocateExpression: public NAryExpression, public idgs::util::CloneEnabler<LocateExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual bool parse(const idgs::pb::Expr& entryExp, const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) override;

  virtual const std::string& name() override {
    static std::string name("LOCATE");
    return name;
  }
};

/// Locate expression
/// to handle expression instr(str, substr)
class InStrExpression: public BinaryExpression, public idgs::util::CloneEnabler<InStrExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("INSTR");
    return name;
  }
};

/// FindInSet expression
/// to handle expression find_in_set(string str, string strlist)
/// strlist is a list of string use ',' to seperated, e.g. str1,str2,str3
/// return -1 if error, 0 if not find, others the index of str in strlist.
class FindInSetExpression: public BinaryExpression, public idgs::util::CloneEnabler<FindInSetExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("FIND_IN_SET");
    return name;
  }
};

/// parse_url(string a, string b) a: url b: one of HOST, PATH, QUERY, REF, PROTOCOL, FILE, AUTHORITY, USERINFO
/// e.g. http://username:password@hostname/path?arg=value#anchor
/// [PROTOCOL] => http [HOST] => hostname [USERINFO] => username [pass] => password [PATH] => /path [QUERY] => arg=value [REF] => anchor
class ParseURLExpression: public NAryExpression, public idgs::util::CloneEnabler<ParseURLExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual bool parse(const idgs::pb::Expr& entryExp, const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) override;

  virtual const std::string& name() override {
    static std::string name("PARSE_URL");
    return name;
  }
};

class GetJsonObjectExpression: public BinaryExpression, public idgs::util::CloneEnabler<GetJsonObjectExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("GET_JSON_OBJECT");
    return name;
  }
private:
  static yajl_val extract(const yajl_val& node, const std::string& path);
  static bool parsePath(std::string& path, int32_t& index);
  static std::string toJson(const yajl_val& node);
};

class RegExpExpression: public BinaryExpression, public idgs::util::CloneEnabler<RegExpExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("REGEXP");
    return name;
  }

  virtual const std::string* alias() override {
    static std::string alias("RLIKE");
    return &alias;
  }
};

class RegExpReplaceExpression: public TernaryExpression, public idgs::util::CloneEnabler<RegExpReplaceExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual const std::string& name() override {
    static std::string name("REGEXP_REPLACE");
    return name;
  }
};

class RegExpExtractExpression: public NAryExpression, public idgs::util::CloneEnabler<RegExpExtractExpression, Expression> {
public:

  /// @brief Evaluate the express, and the state of ctx may be changed by the express.
  /// @param ctx The context, provide addition parameter, e.g. KEY or VALUE of the entry.
  virtual protobuf::PbVariant evaluate(ExpressionContext* ctx) const override;

  virtual bool parse(const idgs::pb::Expr& entryExp, const idgs::actor::PbMessagePtr& key, const idgs::actor::PbMessagePtr& value) override;

  virtual const std::string& name() override {
    static std::string name("REGEXP_EXTRACT");
    return name;
  }
};

} // namespace expr
} // namespace idgs 
