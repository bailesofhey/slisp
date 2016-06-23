#include "NumConverter.h"

using namespace std;

bool NumConverter::IsBase10NumberFloat(const string &str) {
  return str.find(".") != string::npos
      || str.find("e") != string::npos;
}

int NumConverter::GetNumberBase(const string &str) {
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

bool NumConverter::Convert(const string &str, int64_t &value) {
  int base = GetNumberBase(str);
  auto numStart = str.c_str();
  if (base != 10)
    numStart += 2;
  value = stoll(numStart, nullptr, base);
  return true;
}

bool NumConverter::Convert(const string &str, double &value) {
  try {
    value = stod(str);
    return true;
  }
  catch (...) {
    return false;
  }
}