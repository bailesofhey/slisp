#include <string>
#include <exception>

#include "Tokenizer.h"

//=============================================================================

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