
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__) 
#include "idgs_gch.h" 
#endif // GNUC_ $
#include "utillity.h"
#include <sys/time.h>
#include <fstream>

using namespace std;

namespace idgs {

bool sys::isPrime(size_t num) {
  if (num <= 1)
    return false;
  else if (num == 2)
    return true;
  else if (num % 2 == 0)
    return false;
  else {
    bool prime = true;
    int divisor = 3;
    int upperLimit = static_cast<int>(sqrt(static_cast<double>(num)) + 1);
    while (divisor <= upperLimit) {
      if (num % divisor == 0) {
        prime = false;
        break;
      }
      divisor += 2;
    }
    return prime;
  }
}

unsigned long sys::getCurrentTime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

std::string sys::formatTime(double spent_time) {
  stringstream s;
  if (spent_time < 1000) {
    s << spent_time << "ms";
  } else if (spent_time / 1000 < 60) {
    s << spent_time / 1000 << "s";
  } else if (spent_time / 1000 / 60 < 60) {
    s << spent_time / 1000 / 60 << "min";
  } else if (spent_time / 1000 / 60 / 60 < 60) {
    s << spent_time / 1000 / 60 / 60 << "h";
  }
  return s.str();
}

void sys::saveFile(const std::string& fileName, const std::string& content) {
  auto pos = fileName.find_last_of("/");
  string::size_type npos = -1;
  if (pos != npos) {
    string dir = fileName.substr(0, pos);
    string cmd = "mkdir -p " + dir;
    system(cmd.c_str());
  }

  ofstream file(fileName);
  file << content;
  file.close();
}

std::string str::trim(const std::string& s) {
  if (s.length() == 0) {
    return s;
  }
  size_t beg = s.find_first_not_of(" \a\b\f\n\r\t\v");
  size_t end = s.find_last_not_of(" \a\b\f\n\r\t\v");
  if (beg == std::string::npos)
    return "";
  return std::string(s, beg, end - beg + 1);
}

void str::split(const std::string& str, const std::string& delimiter, std::vector<std::string>& tokens) {
  size_t start = 0;
  size_t end = str.find(delimiter);
  const size_t& del_len = delimiter.length();
  while(end != std::string::npos)
  {
    std::string sub_str = str.substr(start, end - start);
    tokens.push_back(sub_str);
    start = end + del_len;
    end = str.find(delimiter, start);
  }
  // append the last one, if exists
  std::string last_sub_str = str.substr(start);
  if(!last_sub_str.empty()) {
    tokens.push_back(last_sub_str);
  }
}

string str::toUpper(const string& str) {
  string value = str;
  transform(value.begin(), value.end(), value.begin(), ::toupper);
  return value;
}

string str::toLower(const string& str) {
  string value = str;
  transform(value.begin(), value.end(), value.begin(), ::tolower);
  return value;
}

}

