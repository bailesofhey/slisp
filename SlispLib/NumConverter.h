#pragma once

#include <string>

class NumConverter {
public:
  static bool IsFloat(const std::string &str);
  static int GetNumberBase(const std::string &str);
  static bool Convert(const std::string &str, int64_t &value);
  static bool Convert(const std::string &str, double &value);
};