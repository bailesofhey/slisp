#pragma once

#include <string>

enum TokenTypes {
  NONE = 0,
  UNKNOWN,
  NUMBER,
  SYMBOL,
  STRING,
  PARENOPEN,
  PARENCLOSE
};

struct Token {
  TokenTypes Type;
  std::string Value;

  operator std::string() const;
};