#include <string>
#include <exception>

#include "Tokenizer.h"

using namespace std;

//=============================================================================
Token::Token():
  Token(TokenTypes::NONE, "")
{
}

Token::Token(TokenTypes type, const string &value):
  Type(type),
  Value(value)
{
}

Token::operator string() const {
  string str = "Token { Type: ";

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
  else if (Type == TokenTypes::QUOTE)
    str += "QUOTE";
  else if (Type == TokenTypes::UNKNOWN)
    str += "UNKNOWN";
  else
    throw runtime_error("invalid token type");

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

ostream& operator<<(ostream& os, const Token& token) {
  return os << token.operator string();
}
