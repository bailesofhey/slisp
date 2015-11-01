#pragma once

#include <string>
#include <iostream>
#include <sstream>

#include "Token.h"

class Tokenizer {
  public:
    void SetLine(const std::string& line);
    Tokenizer& operator++();
    Token& operator*();

    private:
      void SkipWhitespace(char &currChar);
      void TokenizeNumber(char &currChar);
      void TokenizeSymbol(char &currChar);
      void TokenizeString(char &currChar);
      void TokenizeParenOpen(char &currChar);
      void TokenizeParenClose(char &currChar);
      void TokenizeUnknown(char &currChar);
      void TokenizeNone();
      template <class F> void TokenizeSequence(TokenTypes tokenType, char &currChar, F pred);
      template <class F, class G> void TokenizeSequence(TokenTypes tokenType, char &currChar, F pred, G postSeqFn);

      Token CurrToken;
      std::stringstream Stream;
      static bool SymbolPredicate(char c);
};