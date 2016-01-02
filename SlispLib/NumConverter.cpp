#include "NumConverter.h"

int NumConverter::GetNumberBase(const std::string &str) {
  if (str.length() > 2) {
    if (str[0] == '0') {
      if (str[1] == 'x')
        return 16;
      else if (str[1] == 'b')
        return 2;
    }
  }
  return 10;
}

void NumConverter::Convert(const std::string &str, int64_t &value) {
  int base = GetNumberBase(str);
  auto numStart = str.c_str();
  if (base != 10)
    numStart += 2;
  value = std::stoll(numStart, nullptr, base);
}
