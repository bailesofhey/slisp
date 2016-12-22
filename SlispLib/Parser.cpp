#include <iostream>
#include <memory>
#include <algorithm>

#include "NumConverter.h"
#include "Tokenizer.h"
#include "Parser.h"
#include "FunctionDef.h"
#include "InterpreterUtils.h"

using namespace std;

//=============================================================================

Parser::Parser(CommandInterface &commandInterface, ITokenizer &tokenizer, InterpreterSettings &settings, bool debug):
  CommandInterface_ { commandInterface },
  Tokenizer_ { tokenizer },
  Settings { settings },
  SourceContext_ { },
  Debug { debug }
{
}

bool Parser::Parse() {
  Reset();

  string line;
  ++SourceContext_.LineNum;
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

bool Parser::HasInfixArgCount(InterpreterSettings &settings, Sexp &sexp, ArgList::iterator &currArg, ArgList::iterator &firstPos) const {
  size_t nArgs = sexp.Args.size();
  if (nArgs < 3) // (3 + 4)
    return false;

  ExpressionPtr defaultSexpArg { new Symbol(SourceContext_, settings.GetDefaultSexp()) };
  if (Expression::AreEqual(defaultSexpArg, *currArg)) {
    firstPos = ++currArg;
    if ((nArgs % 2) == 1)
      return false;
  }
  else if ((nArgs % 2) == 0)
    return false;

  return true;
}

using InfixOp = pair<string, int>;
bool PopulateInfixOperators(InterpreterSettings &settings, ArgList::const_iterator firstPosArg, ArgList::const_iterator endArg, vector<InfixOp> &infixOperators) {
  auto currArg = firstPosArg;
  if (auto firstArgSym = dynamic_cast<Symbol*>((*currArg).get())) {
    if (settings.IsSymbolFunction(firstArgSym->Value))
      return false;
  }

  size_t argNum = 1;
  while (currArg != endArg) {
    if ((argNum % 2) == 0) {
      if (auto fnSym = dynamic_cast<Symbol*>((*currArg).get())) {
        string op = fnSym->Value;
        int precedence = settings.GetInfixSymbolPrecedence(op);
        if (precedence == InterpreterSettings::NO_PRECEDENCE)
          return false;
        infixOperators.push_back({ op, precedence });
      }
      else
        return false;
    }
    ++currArg;
    ++argNum;
  }
  return true;
}

void Parser::TransformInfixSexp(Sexp &sexp, bool isImplicit) const {
  auto firstArg = begin(sexp.Args);
  auto currArg = firstArg; 
  auto endArg = end(sexp.Args);
  auto firstPos = currArg;

  if (!HasInfixArgCount(Settings, sexp, currArg, firstPos))
    return;

  vector<InfixOp> infixOperators;
  if (!PopulateInfixOperators(Settings, firstPos, endArg, infixOperators))
    return;

  sort(begin(infixOperators), end(infixOperators), [](const InfixOp &lhs, const InfixOp &rhs) { 
    return lhs.second < rhs.second;
  });

  ArgList newArgs;
  ArgListHelper::CopyTo(sexp.Args, newArgs);

  if (firstArg != firstPos)
    newArgs.pop_front();

  for (auto &infixOp : infixOperators) {
    auto fnFirst = begin(newArgs);
    auto fnCurr = fnFirst;
    auto fnArg = end(newArgs);
    ExpressionPtr opSym { new Symbol(SourceContext_, infixOp.first) };
    ExpressionPtr opExpr { new Sexp(SourceContext_) };
    Sexp &opSexp = static_cast<Sexp&>(*opExpr);
    size_t fnArgNum = 1;
    size_t totalArgNum = 1;
    bool insideOp = false;
    while (fnCurr != end(newArgs)) {
      bool consumedCurrArg = false;

      if ((totalArgNum % 2) == 0) {
        if (Expression::AreEqual(*fnCurr, opSym)) {
          if (fnArgNum == 1) {
            opSexp.Args.push_front((*fnCurr)->Clone());
            insideOp = true;
            --fnCurr;
            --totalArgNum;
            continue;
          }
          consumedCurrArg = true;
        }
        else {
          if (insideOp) {
            newArgs.insert(fnCurr, opExpr->Clone());

            insideOp = false;
            fnArgNum = 1;
            opSexp.Args.clear();
            continue;
          }
        }
      }
      else {
        if (insideOp) {
          opSexp.Args.push_back((*fnCurr)->Clone());
          ++fnArgNum;
          consumedCurrArg = true;
        }
      }

      auto oldCurr = fnCurr;
      ++fnCurr;
      ++totalArgNum;
      if (consumedCurrArg)
        newArgs.erase(oldCurr);
    }

    if (!opSexp.Args.empty())
      newArgs.insert(fnCurr, opExpr->Clone());
  }

  sexp.Args.erase(firstPos, endArg);
  if (auto unwrappedNewArgsSexp = dynamic_cast<Sexp*>(newArgs.front().get())) {
    if (isImplicit)
      sexp.Args.push_back(unwrappedNewArgsSexp->Clone());
    else
      ArgListHelper::CopyTo(unwrappedNewArgsSexp->Args, sexp.Args);
  }
}

const string& Parser::Error() const {
  return Error_;
}

unique_ptr<Sexp> Parser::ExpressionTree() const {
  return unique_ptr<Sexp>(static_cast<Sexp*>(ExprTree->Clone().release()));
}

SourceContext Parser::GetSourceContext() const {
  return SourceContext_;
}

void Parser::SetSourceContext(const SourceContext &sourceContext) {
  SourceContext_ = sourceContext;
}

void Parser::Reset() {
  ExprTree = unique_ptr<Sexp>(new Sexp(SourceContext_));
  ExprTree->Args.push_back(ExpressionPtr { new Symbol { SourceContext_, Settings.GetDefaultSexp() } });
  Error_ = "";
  Depth = 0;
}

bool Parser::ParseToken(Sexp &root) {
  if (Debug)
    cout << static_cast<string>(*Tokenizer_) << endl;

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
  else if (tokenType == TokenTypes::QUOTE)
    return ParseQuote(root);
  else if (tokenType == TokenTypes::UNKNOWN) 
    return ParseUnknown(root);
  else if (tokenType == TokenTypes::NONE)
    return ParseNone(root);
  else {
    Error_ = "Unexpected token: " + static_cast<string>(*Tokenizer_);
    return false;
  }
}

template <class N>
bool ParseNum(const string &val, ExpressionPtr &numExpr, Sexp &root, string &error) {
  auto &num = static_cast<N&>(*numExpr);
  if (NumConverter::Convert(val, num.Value)) {
    root.Args.push_back(move(numExpr));
    return true;
  }
  else {
    error = "Value could not be converted to " + N::TypeInstance.Name();
    return false;
  }
}

bool Parser::ParseNumber(Sexp &root) {
  auto &val = (*Tokenizer_).Value;
  if (!val.empty()) {
    int base = NumConverter::GetNumberBase(val);
    if (base == 10 && NumConverter::IsBase10NumberFloat(val)) {
      ExpressionPtr numExpr { new Float(SourceContext_) };
      return ParseNum<Float>(val, numExpr, root, Error_);
    }
    else {
      ExpressionPtr numExpr { new Int(SourceContext_) };
      return ParseNum<Int>(val, numExpr, root, Error_);
    }
  }
  else {
    Error_ = "Number has no value";
    return false;
  }
}

bool Parser::ParseSymbol(Sexp &root) {
  auto &val = (*Tokenizer_).Value;
  if (!val.empty()) {
    root.Args.push_back(ExpressionPtr { new Symbol { SourceContext_, val } });
    return true;
  }
  else {
    Error_ = "Symbol has no value";
    return false;
  }
}

bool Parser::ParseString(Sexp &root) {
  root.Args.push_back(ExpressionPtr { new Str { SourceContext_, (*Tokenizer_).Value } });
  return true;
}

bool Parser::ParseParenOpen(Sexp &root) {
  ++Depth;
  ++Tokenizer_;
  auto currSexpExpr = ExpressionPtr { new Sexp(SourceContext_) };
  auto currSexp = static_cast<Sexp*>(currSexpExpr.get());
  return ParseSexpArgs(root, *currSexp);
}

bool Parser::ParseParenClose(Sexp &root) {
  --Depth;
  return true;
}

bool Parser::ParseSexpArgs(Sexp &root, Sexp &curr) {
  bool parseResult = true;
  Sexp currLineSexp { SourceContext_ };
  bool isMultiline = false;
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
    TransformInfixSexp(currLineSexp, isMultiline ? true : false);
    ArgListHelper::CopyTo(currLineSexp.Args, curr.Args);
    if (isMultiline)
      TransformInfixSexp(curr, false); 
    root.Args.emplace_back(curr.Clone());
    (*Tokenizer_).Type = TokenTypes::UNKNOWN;
    return true;
  }
  else  {
    if (Depth) {
      string line;
      if (CommandInterface_.HasMore()) {
        ++SourceContext_.LineNum;
        CommandInterface_.ReadContinuedInputLine(line);
        Tokenizer_.SetLine(line);
        ++Tokenizer_;

        TransformInfixSexp(currLineSexp, true);
        isMultiline = true;

        ArgListHelper::CopyTo(currLineSexp.Args, curr.Args);
        goto begin;
      }
      else {
        Error_ = "Unterminated Sexp";
        return false;
      }
    }
    else
      throw logic_error("Logic bug: NONE should only be reached with Depth > 0");
  }
}

bool Parser::ParseQuote(Sexp &root) {
  ExpressionPtr newExpr { new Sexp(SourceContext_) };
  Sexp &newSexp = static_cast<Sexp&>(*newExpr);
  ++Tokenizer_;
  if (ParseToken(newSexp)) {
    if (!newSexp.Args.empty()) {
      root.Args.emplace_back(new Quote(SourceContext_, move(newSexp.Args.front())));
      newSexp.Args.pop_front();
      return true;
    }
    else {
      Error_ = "Invalid quote expression";
      return false;
    }
  }
  return false;
}

bool Parser::ParseUnknown(Sexp &root) {
  Error_ = "Unknown token: " + (*Tokenizer_).Value;
  return false;
}

bool Parser::ParseNone(Sexp &root) {
  return true;
}
