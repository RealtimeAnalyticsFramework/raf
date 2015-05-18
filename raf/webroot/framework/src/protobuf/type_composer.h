
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#ifndef TYPE_COMPOSER_H_
#define TYPE_COMPOSER_H_
#include <vector>
#include <map>
#include <sstream>

namespace protobuf {

///
/// dynamic field
///
struct DynamicField {
public:
  DynamicField();
  DynamicField(const std::string& label_, const std::string& type_, const std::string& name);
  std::string toString() const;
  void toString(std::ostream& os) const;

public:
  /// optional. required or repeated
  std::string label;
  std::string type;
  std::string name;
  int number;
};

inline DynamicField::DynamicField() :
    label("optional"), number(0) {

}
inline DynamicField::DynamicField(const std::string& label_, const std::string& type_, const std::string& name_) :
    label(label_), type(type_), name(name_), number(0) {

}
inline void DynamicField::toString(std::ostream& os) const {
  os << "  " << label << " " << type << " " << name << " = " << number << ";" << std::endl;
}
inline std::string DynamicField::toString() const {
  std::ostringstream oss;
  toString(oss);
  return oss.str();
}

///
/// dynamic message
///
struct DynamicMessage {
public:
  DynamicMessage();
  DynamicMessage(std::string name_);
  std::string toString() const;
  void toString(std::ostream& os) const;

  void addField(DynamicField& field);

public:
  std::string name;
  std::vector<DynamicField> fields;
};

inline DynamicMessage::DynamicMessage() {
}

inline DynamicMessage::DynamicMessage(std::string name_) :
    name(name_) {
}

inline std::string DynamicMessage::toString() const {
  std::ostringstream oss;
  toString(oss);
  return oss.str();
}
inline void DynamicMessage::toString(std::ostream& os) const {
  os << "message " << name << " {" << std::endl;
  for (auto it = fields.begin(); it != fields.end(); ++it) {
    it->toString(os);
  }
  os << "}" << std::endl;
}

///
/// dynamic module
///
class DynamicTypeComposer {
public:
  DynamicTypeComposer();
  ~DynamicTypeComposer();
  std::string toString() const;
  void toString(std::ostream& os) const;

  const std::string& getName() const {
    return name;
  }
  ;
  void setName(const std::string& name) {
    this->name = name;
  }

  const std::string& getPackage() const {
    return package;
  }

  void setPackage(const std::string& p) {
    package = p;
  }

  void saveFile(const std::string& fileName);

  void addImport(std::string m);

  void addMessage(const DynamicMessage& msg);
private:
  std::string name;
  std::string package;
  std::vector<std::string> imports;
  std::map<std::string, DynamicMessage> messages;
};
inline std::string DynamicTypeComposer::toString() const {
  std::ostringstream oss;
  toString(oss);
  return oss.str();
}
inline void DynamicTypeComposer::toString(std::ostream& os) const {
  os << "package " << package << ";" << std::endl;
  for (std::string p : imports) {
    os << "import \"" << p << '\"' << std::endl;
  }
  for (auto it = messages.begin(); it != messages.end(); ++it) {
    it->second.toString(os);
  }
}
inline void DynamicTypeComposer::addImport(std::string m) {
  imports.push_back(m);
}

} // namespace protobuf 
#endif /* TYPE_COMPOSER_H_ */
