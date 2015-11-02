#pragma once

#include <iostream>
#include <memory>

#include "Tokenizer.h"
#include "Expression.h"
#include "CommandInterface.h"

class Parser {
  public:
    explicit Parser(CommandInterface &commandInterface, const std::string &defaultSexp, bool debug = false);
    Parser() = delete;
    Parser(const Parser&) = delete;
    Parser(Parser &&) = delete;
    Parser& operator=(const Parser&) = delete;
    Parser& operator=(Parser &&) = delete;

    bool Parse();
    const std::string& Error() const;
    std::unique_ptr<Sexp> ExpressionTree() const;

  private:
    CommandInterface      &CommandInterface_;
    Tokenizer             Tokenizer_;
    std::unique_ptr<Sexp> ExprTree;
    std::string           DefaultSexp;
    std::string           Error_;
    int                   Depth;
    bool                  Debug;

    void Reset();
    bool ParseToken(Sexp &root);
    bool ParseNumber(Sexp &root);
    bool ParseString(Sexp &root);
    bool ParseSymbol(Sexp &root);
    bool ParseParenOpen(Sexp &root);
    bool ParseParenClose(Sexp &root);
    bool ParseSexpArgs(Sexp &root, Sexp &curr);
    bool ParseUnknown(Sexp &root);
    bool ParseNone(Sexp &root);
};
