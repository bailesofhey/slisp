#include <string>
#include <exception>

#include "Tokenizer.h"

//=============================================================================
Token::Token():
  Token(TokenTypes::NONE, "")
{
}

Token::Token(TokenTypes type, const std::string &value):
  Type(type),
  Value(value)
{
}

Token::operator std::string() const {
  std::string str = "Token { Type: ";

  if (Type == TokenTypes::NONE)
    str += "NONE";
  else if (Type == TokenTypes::NUMBER)
    str += "NUMBER";
  else if (Type == TokenTypes::SYMBOL)
    str += "SYMBOL";
  else if (Type == TokenTypes::STRING)
    str += "STRING";
  else if (Type == TokenTypes::PARENOPEN)
    str += "PARENOPEN";
  else if (Type == TokenTypes::PARENCLOSE)
    str += "PARENCLOSE";
  else if (Type == TokenTypes::UNKNOWN)
    str += "UNKNOWN";
  else
    throw std::exception("invalid token type");

  str += ", Value: ";
  if (Type == TokenTypes::STRING)
    str += "\"";
  str += Value;
  if (Type == TokenTypes::STRING)
    str += "\"";
  str += " }";

  return str;
}

bool Token::operator==(const Token& rhs) const {
  return Type == rhs.Type
      && Value == rhs.Value;
}

std::ostream& operator<<(std::ostream& os, const Token& token) {
  return os << token.operator std::string();
}