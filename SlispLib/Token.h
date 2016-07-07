#pragma once

#include <string>
#include <iostream>


enum TokenTypes {
  NONE = 0,
  UNKNOWN,
  NUMBER,
  SYMBOL,
  STRING,
  PARENOPEN,
  PARENCLOSE,
  QUOTE,
};

struct Token {
  TokenTypes Type;
  std::string Value;

  explicit Token();
  explicit Token(TokenTypes type, const std::string &value);
  operator std::string() const;
  bool operator==(const Token& rhs) const;
};

std::ostream& operator<<(std::ostream& os, const Token& token);