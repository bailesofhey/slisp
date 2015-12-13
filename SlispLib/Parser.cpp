#include <iostream>
#include <memory>

#include "Tokenizer.h"
#include "Parser.h"
#include "FunctionDef.h"
#include "InterpreterUtils.h"

using std::cout;
using std::endl;

//=============================================================================

Parser::Parser(CommandInterface &commandInterface, ITokenizer &tokenizer, InterpreterSettings &settings, bool debug):
  CommandInterface_ { commandInterface },
  Tokenizer_ { tokenizer },
  Settings { settings },
  Debug { debug }
{
}

bool Parser::Parse() {
  Reset();

  std::string line;
  if (CommandInterface_.ReadInputLine(line)) {
    Tokenizer_.SetLine(line);
    
    ++Tokenizer_;
    bool parseResult = true;
    while (parseResult && (*Tokenizer_).Type != TokenTypes::NONE) {
      parseResult = ParseToken(*ExprTree);
      ++Tokenizer_;
    }

    if (parseResult)
      TransformInfixSexp(*ExprTree, true);

    return parseResult;
  }

  return false;
}

bool TransformInfixFunc(InterpreterSettings &settings, ExpressionPtr &currArg, Sexp &wrappedSexp, std::string &fnName) {
  if (auto fnSym = dynamic_cast<Symbol*>(currArg.get())) {
    if (fnName.empty()) {
      if (!settings.IsInfixSymbol(fnSym->Value))
        return false;
      fnName = fnSym->Value;
      wrappedSexp.Args.push_front(currArg->Clone());
      return true;
    }
    else if (fnSym->Value != fnName)
      return false; // Later: x = 3 + 4
    else
      return true;
  }
  else
    return false;
}

bool TransformInfixArg(InterpreterSettings &settings, ExpressionPtr &currArg, Sexp &wrappedSexp, size_t argNum) {
  if (argNum == 1) {
    if (auto symArg = dynamic_cast<Symbol*>(currArg.get())) {
      if (settings.IsSymbolFunction(symArg->Value))
        return false;
    }
  } 
  wrappedSexp.Args.push_back(currArg->Clone());
  return true;
}

bool IsInfixSexp(InterpreterSettings &settings, Sexp &sexp, ArgList::iterator &currArg, ArgList::iterator &firstPos) {
  size_t nArgs = sexp.Args.size();
  if (nArgs < 3) // (3 + 4)
    return false;

  ExpressionPtr defaultSexpArg { new Symbol(settings.GetDefaultSexp()) };
  if (Expression::AreEqual(defaultSexpArg, *currArg)) {
    firstPos = ++currArg;
    if ((nArgs % 2) == 1)
      return false;
  }
  else if ((nArgs % 2) == 0)
    return false;

  return true;
}

void Parser::TransformInfixSexp(Sexp &sexp, bool isImplicit) const {
  auto currArg = begin(sexp.Args);
  auto endArg = end(sexp.Args);
  auto firstPos = currArg;

  if (!IsInfixSexp(Settings, sexp, currArg, firstPos))
    return;

  ExpressionPtr wrappedExpr { new Sexp() };
  auto wrappedSexp = static_cast<Sexp*>(wrappedExpr.get());
  std::string fnName;
  size_t argNum = 1;
  while (currArg != endArg) {
    if ((argNum % 2) == 0) {
      if (!TransformInfixFunc(Settings, *currArg, *wrappedSexp, fnName))
        return;
    }
    else {
      if (!TransformInfixArg(Settings, *currArg, *wrappedSexp, argNum))
        return;
    }

    ++argNum;
    ++currArg;
  }

  sexp.Args.erase(firstPos, endArg);
  if (isImplicit)
    sexp.Args.push_back(std::move(wrappedExpr));
  else
    ArgListHelper::CopyTo(wrappedSexp->Args, sexp.Args);
}

const std::string& Parser::Error() const {
  return Error_;
}

std::unique_ptr<Sexp> Parser::ExpressionTree() const {
  return std::unique_ptr<Sexp>(static_cast<Sexp*>(ExprTree->Clone().release()));
}

void Parser::Reset() {
  ExprTree = std::unique_ptr<Sexp>(new Sexp);
  ExprTree->Args.push_back(ExpressionPtr { new Symbol { Settings.GetDefaultSexp() } });
  Error_ = "";
  Depth = 0;
}

bool Parser::ParseToken(Sexp &root) {
  if (Debug)
    cout << static_cast<std::string>(*Tokenizer_) << endl;

  auto &tokenType = (*Tokenizer_).Type;
  if (tokenType == TokenTypes::NUMBER)
    return ParseNumber(root);
  else if (tokenType == TokenTypes::STRING)
    return ParseString(root);
  else if (tokenType == TokenTypes::SYMBOL)
    return ParseSymbol(root);
  else if (tokenType == TokenTypes::PARENOPEN)
    return ParseParenOpen(root);
  else if (tokenType == TokenTypes::PARENCLOSE)
    return ParseParenClose(root);
  else if (tokenType == TokenTypes::UNKNOWN)
    return ParseUnknown(root);
  else if (tokenType == TokenTypes::NONE)
    return ParseNone(root);
  else {
    Error_ = "Unexpected token: " + static_cast<std::string>(*Tokenizer_);
    return false;
  }
}

bool Parser::ParseNumber(Sexp &root) {
  auto &val = (*Tokenizer_).Value;
  if (!val.empty()) {
    root.Args.push_back(ExpressionPtr { new Number { std::atoll(val.c_str()) } });
    return true;
  }
  else {
    Error_ = "Number has no value";
    return false;
  }
}

bool Parser::ParseSymbol(Sexp &root) {
  auto &val = (*Tokenizer_).Value;
  if (!val.empty()) {
    root.Args.push_back(ExpressionPtr { new Symbol { val } });
    return true;
  }
  else {
    Error_ = "Symbol has no value";
    return false;
  }
}

bool Parser::ParseString(Sexp &root) {
  root.Args.push_back(ExpressionPtr { new String { (*Tokenizer_).Value } });
  return true;
}

bool Parser::ParseParenOpen(Sexp &root) {
  ++Depth;
  ++Tokenizer_;
  auto currSexpExpr = ExpressionPtr { new Sexp };
  auto currSexp = static_cast<Sexp*>(currSexpExpr.get());
  return ParseSexpArgs(root, *currSexp);
}

bool Parser::ParseParenClose(Sexp &root) {
  --Depth;
  return true;
}

bool Parser::ParseSexpArgs(Sexp &root, Sexp &curr) {
  bool parseResult = true;
  Sexp currLineSexp;
begin:
  currLineSexp.Args.clear();
  while ((parseResult = ParseToken(currLineSexp)) &&
         (*Tokenizer_).Type != TokenTypes::NONE &&
         (*Tokenizer_).Type != TokenTypes::PARENCLOSE) {
    ++Tokenizer_;
  }

  if (!parseResult)
    return false;
  else if ((*Tokenizer_).Type == TokenTypes::PARENCLOSE) {
    ArgListHelper::CopyTo(currLineSexp.Args, curr.Args);
    TransformInfixSexp(curr, false);
    root.Args.push_back(ExpressionPtr { curr.Clone() });
    (*Tokenizer_).Type = TokenTypes::UNKNOWN;
    return true;
  }
  else  {
    if (Depth) {
      std::string line;
      if (CommandInterface_.HasMore()) {
        CommandInterface_.ReadContinuedInputLine(line);
        Tokenizer_.SetLine(line);
        ++Tokenizer_;

        TransformInfixSexp(currLineSexp, true);

        ArgListHelper::CopyTo(currLineSexp.Args, curr.Args);
        goto begin;
      }
      else {
        Error_ = "Unterminated Sexp";
        return false;
      }
    }
    else
      throw std::exception("Logic bug: NONE should only be reached with Depth > 0");
  }
}

bool Parser::ParseUnknown(Sexp &root) {
  Error_ = "Unknown token: " + (*Tokenizer_).Value;
  return false;
}

bool Parser::ParseNone(Sexp &root) {
  return true;
}