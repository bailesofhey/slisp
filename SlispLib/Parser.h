#pragma once
#include <iostream>
#include <memory>

#include "Tokenizer.h"
#include "Expression.h"
#include "CommandInterface.h"
#include "InterpreterUtils.h"

class Parser {
  public:
    explicit Parser(CommandInterface &commandInterface, ITokenizer &tokenizer, InterpreterSettings &settings, bool debug = false);
    Parser() = delete;
    Parser(const Parser&) = delete;
    Parser(Parser &&) = delete;
    Parser& operator=(const Parser&) = delete;
    Parser& operator=(Parser &&) = delete;

    bool Parse();
    const std::string& Error() const;
    std::unique_ptr<Sexp> ExpressionTree() const;
    SourceContext         SourceContext_;

  private:
    CommandInterface      &CommandInterface_;
    ITokenizer            &Tokenizer_;
    InterpreterSettings   &Settings;
    std::unique_ptr<Sexp> ExprTree;
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
    bool ParseQuote(Sexp &root);
    bool ParseUnknown(Sexp &root);
    bool ParseNone(Sexp &root);

    void TransformInfixSexp(Sexp &sexp, bool isImplicit) const;
    bool HasInfixArgCount(InterpreterSettings &settings, Sexp &sexp, ArgList::iterator &currArg, ArgList::iterator &firstPos) const;
};
