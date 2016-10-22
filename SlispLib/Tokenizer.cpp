#include <string>
#include <iostream>
#include <cctype>
#include <functional>

#include "Token.h"
#include "NumConverter.h"
#include "Tokenizer.h"

using namespace std;

//=============================================================================

void Tokenizer::SetLine(const string& line) {
  Stream.str(line);
  Stream.clear();
}

ITokenizer& Tokenizer::operator++() {
  if (!Stream.eof()) {
    char currChar = '\0';
    TokenizeNone();
    SkipWhitespace(currChar);
    if (!Stream.eof() && currChar != '\0') {
      if (currChar == '#') {
        Stream.str("");
        Stream.clear();
      }
      else {
        if (isdigit(currChar))
          TokenizeNumber(currChar);
        else if (SymbolPredicate(currChar))
          TokenizeSymbol(currChar);
        else if (currChar == '"')
          TokenizeString(currChar);
        else if (currChar == '(')
          TokenizeParenOpen(currChar);
        else if (currChar == ')')
          TokenizeParenClose(currChar);
        else if (currChar == '\'')
          TokenizeQuote(currChar);
        else
          TokenizeUnknown(currChar);
      }
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
  while (!Stream.eof() && isspace(currChar));
}

bool IsBinaryDigit(char c) {
  return c == '0' || c == '1';
}

bool IsFloatDigit(char c) {
  return isdigit(c) || c == '.' || c == 'e' || c == '-';
}

bool IsNumber(char c) {
  return isxdigit(c) || c == 'x' || c == 'b' || c == 'e' || c == '.' || c == '-';
}

bool IsDigit(char c) {
  return isdigit(c);
}

bool IsHexDigit(char c) {
  return isxdigit(c);
}

void Tokenizer::TokenizeNumber(char &currChar) {
  TokenizeSequence(TokenTypes::NUMBER, currChar, IsNumber);
  int base = NumConverter::GetNumberBase(CurrToken.Value);
  auto curr = std::begin(CurrToken.Value);
  auto end = std::end(CurrToken.Value);
  function<bool(int)> pred = IsDigit; 
  if (base != 10) {
    curr += 2;
    pred = base == 16 ? IsHexDigit : IsBinaryDigit;
  }
  else if (NumConverter::IsBase10NumberFloat(CurrToken.Value)) {
    pred = IsFloatDigit; 
  }

  while (curr != end) {
    if (!pred(*curr))
      break;
    *curr = tolower(*curr);
    ++curr;
  }

  if (curr != end) 
    CurrToken.Type = TokenTypes::UNKNOWN;
}

void Tokenizer::TokenizeSymbol(char &currChar) {
  TokenizeSequence(TokenTypes::SYMBOL, currChar, SymbolPredicate);

  size_t len = CurrToken.Value.length();
  if (len > 1 && CurrToken.Value[0] == '-') {
    if (!isdigit(CurrToken.Value[1]))
      return;
    for (size_t i = 2; i < len; ++i) {
      if (!IsFloatDigit(CurrToken.Value[i]))
        return;
    }
    CurrToken.Type = TokenTypes::NUMBER;
  }

  for (size_t i = 1; i < len; ++i) {
    if (CurrToken.Value[i] == '\'') {
      CurrToken.Type = TokenTypes::UNKNOWN;
      return;
    }
  }
}

      
const string EscapeChars = "ntvbrfa\\\"'0x";
const size_t HexEscapePos = 11;

enum StrState {
  Character,
  EscapeStart,
  HexFirstOctet,
  HexSecondOctet,
};

void Tokenizer::TokenizeString(char &currChar) {
  Stream.get(currChar);
  StrState state = StrState::Character;
  bool invalid = false;
  auto pred = [&state, &invalid](char c) { 
    if (state == StrState::Character) {
      if (c == '\\')
        state = StrState::EscapeStart;
    }
    else if (state == StrState::EscapeStart) {
      size_t pos = EscapeChars.find(c);
      if (pos != string::npos) {
        if (pos == HexEscapePos)
          state = StrState::HexFirstOctet;
        else
          state = StrState::Character;
      }
      else {
        invalid = true;
        state = StrState::Character;
      }
      return true;
    }
    else if (state == StrState::HexFirstOctet || state == StrState::HexSecondOctet) {
      invalid = !isxdigit(c);
      if (invalid || state == StrState::HexSecondOctet)
        state = StrState::Character;
      else if (state == StrState::HexFirstOctet)
        state = StrState::HexSecondOctet;
    }
    return c != '"';
  };
  TokenizeSequence(TokenTypes::STRING, currChar, pred, [this, &invalid](char& c) { Stream.get(c); });
  if (invalid) 
    CurrToken.Type = TokenTypes::UNKNOWN;
}

void Tokenizer::TokenizeParenOpen(char &currChar) {
  CurrToken.Value = currChar;
  CurrToken.Type = TokenTypes::PARENOPEN;
}

void Tokenizer::TokenizeParenClose(char &currChar) {
  CurrToken.Value = currChar;
  CurrToken.Type = TokenTypes::PARENCLOSE;
}

void Tokenizer::TokenizeQuote(char &currChar) {
  CurrToken.Value = currChar;
  CurrToken.Type = TokenTypes::QUOTE;
}

void Tokenizer::TokenizeUnknown(char &currChar) {
  auto pred = [](char c) { return !isspace(c); };
  TokenizeSequence(TokenTypes::UNKNOWN, currChar, pred);
}

void Tokenizer::TokenizeNone() {
  CurrToken.Value = "";
  CurrToken.Type = TokenTypes::NONE;
}

template <class F>
void Tokenizer::TokenizeSequence(TokenTypes tokenType, char &currChar, F pred) {
  TokenizeSequence(tokenType, currChar, pred, [](char &c) { });
}

template <class F, class G>
void Tokenizer::TokenizeSequence(TokenTypes tokenType, char &currChar, F pred, G postSeqFn) {
  while (!Stream.eof() && pred(currChar)) {
    CurrToken.Value += currChar;
    Stream.get(currChar);
  }

  postSeqFn(currChar);

  if (!Stream.eof() && !isspace(currChar) && currChar != ')')
    TokenizeUnknown(currChar);
  else {
    Stream.unget();
    CurrToken.Type = tokenType;
  }
}

bool Tokenizer::SymbolPredicate(char c) {
  return isdigit(c) || isalpha(c) || c == '\\' 
      || c == '~' || c == '`' || c == '!' || c == '@' || c == '#' || c == '$'
      || c == '%' || c == '^' || c == '&' || c == '*' || c == '-' || c == '_'
      || c == '=' || c == '+' || c == '{' || c == '[' || c == '}' || c == ']'
      || c == '|' || c == ';' || c == ':' || c == '<' || c == '>' || c == ','
      || c == '.' || c == '/' || c == '?';
}
