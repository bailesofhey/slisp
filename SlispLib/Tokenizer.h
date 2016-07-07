#pragma once

#include <string>
#include <iostream>
#include <sstream>

#include "Token.h"

class ITokenizer {
  public:
    virtual ~ITokenizer() {}
    virtual void SetLine(const std::string& line) = 0;
    virtual ITokenizer& operator++() = 0;
    virtual Token& operator*() = 0;
};

class Tokenizer: public ITokenizer {
  public:
    virtual void SetLine(const std::string& line) override;
    virtual ITokenizer& operator++() override;
    virtual Token& operator*() override;

  private:
    void SkipWhitespace(char &currChar);
    void TokenizeNumber(char &currChar);
    void TokenizeSymbol(char &currChar);
    void TokenizeString(char &currChar);
    void TokenizeParenOpen(char &currChar);
    void TokenizeParenClose(char &currChar);
    void TokenizeQuote(char &currChar);
    void TokenizeUnknown(char &currChar);
    void TokenizeNone();
    template <class F> void TokenizeSequence(TokenTypes tokenType, char &currChar, F pred);
    template <class F, class G> void TokenizeSequence(TokenTypes tokenType, char &currChar, F pred, G postSeqFn);

    Token CurrToken;
    std::stringstream Stream;
    static bool SymbolPredicate(char c);
};