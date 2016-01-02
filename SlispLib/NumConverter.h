#pragma once

#include <string>

class NumConverter {
public:
  static int GetNumberBase(const std::string &str);
  static void Convert(const std::string &str, int64_t &value);
};