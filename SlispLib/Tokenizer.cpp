#include <string>
#include <iostream>
#include <cctype>

#include "Token.h"
#include "Tokenizer.h"

//=============================================================================

void Tokenizer::SetLine(const std::string& line) {
  Stream.str(line);
  Stream.clear();
}

ITokenizer& Tokenizer::operator++() {
  if (!Stream.eof()) {
    char currChar = '\0';
    TokenizeNone();
    SkipWhitespace(currChar);
    if (!Stream.eof() && currChar != '\0') {
      if (std::isdigit(currChar))
        TokenizeNumber(currChar);
      else if (SymbolPredicate(currChar))
        TokenizeSymbol(currChar);
      else if (currChar == '"') 
        TokenizeString(currChar);
      else if (currChar == '(')
        TokenizeParenOpen(currChar);
      else if (currChar == ')')
        TokenizeParenClose(currChar);
      else
        TokenizeUnknown(currChar);
    }
    else
      TokenizeNone();
  }
  else
    TokenizeNone();

  return *this;
}

Token& Tokenizer::operator*() {
  return CurrToken;
}

void Tokenizer::SkipWhitespace(char &currChar) {
  do {
    Stream.get(currChar);
  }
  while (!Stream.eof() && std::isspace(currChar));
}

void Tokenizer::TokenizeNumber(char &currChar) {
  bool isFirst = true;
  TokenizeSequence(TokenTypes::NUMBER, currChar, std::isdigit);
}

void Tokenizer::TokenizeSymbol(char &currChar) {
  TokenizeSequence(TokenTypes::SYMBOL, currChar, SymbolPredicate);

  size_t len = CurrToken.Value.length();
  if (len > 1 && CurrToken.Value[0] == '-') {
    for (size_t i = 1; i < len; ++i) {
      if (!std::isdigit(CurrToken.Value[i]))
        return;
    }
    CurrToken.Type = TokenTypes::NUMBER;
  }
}

void Tokenizer::TokenizeString(char &currChar) {
  Stream.get(currChar);
  auto pred = [](char c) { return c != '"'; };
  auto postSeqFn = [this](char& c) { Stream.get(c); };
  TokenizeSequence(TokenTypes::STRING, currChar, pred, postSeqFn);
}

void Tokenizer::TokenizeParenOpen(char &currChar) {
  CurrToken.Value = currChar;
  CurrToken.Type = TokenTypes::PARENOPEN;
}

void Tokenizer::TokenizeParenClose(char &currChar) {
  CurrToken.Value = currChar;
  CurrToken.Type = TokenTypes::PARENCLOSE;
}

void Tokenizer::TokenizeUnknown(char &currChar) {
  CurrToken.Value += currChar;
  CurrToken.Type = TokenTypes::UNKNOWN;
}

void Tokenizer::TokenizeNone() {
  CurrToken.Value = "";
  CurrToken.Type = TokenTypes::NONE;
}

template <class F>
void Tokenizer::TokenizeSequence(TokenTypes tokenType, char &currChar, F pred) {
  TokenizeSequence(tokenType, currChar, pred, [](char &c) {});
}

template <class F, class G>
void Tokenizer::TokenizeSequence(TokenTypes tokenType, char &currChar, F pred, G postSeqFn) {
  while (!Stream.eof() && pred(currChar)) {
    CurrToken.Value += currChar;
    currChar = Stream.get();
  }

  postSeqFn(currChar);

  if (!Stream.eof() && !std::isspace(currChar) && currChar != ')')
    TokenizeUnknown(currChar);
  else {
    Stream.unget();
    CurrToken.Type = tokenType;
  }
}

bool Tokenizer::SymbolPredicate(char c) {
  return std::isdigit(c) || std::isalpha(c) || c == '\\' || c == '\''
      || c == '~' || c == '`' || c == '!' || c == '@' || c == '#' || c == '$'
      || c == '%' || c == '^' || c == '&' || c == '*' || c == '-' || c == '_'
      || c == '=' || c == '+' || c == '{' || c == '[' || c == '}' || c == ']'
      || c == '|' || c == ';' || c == ':' || c == '<' || c == '>' || c == ','
      || c == '.' || c == '/' || c == '?';
}