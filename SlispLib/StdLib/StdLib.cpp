#include <iostream>
#include <memory>
#include <sstream>

#include "StdLib.h"
#include "../Interpreter.h"

//=============================================================================

void StdLib::Load(Interpreter &interpreter) {
  // Constants

  auto &symbols = interpreter.GetDynamicSymbols();

  int major = 0,
      minor = 1;
  symbols.PutSymbolString("system.version", "Slip " + std::to_string(major) + "." + std::to_string(minor));
  symbols.PutSymbolNumber("system.versionNumber.major", major);
  symbols.PutSymbolNumber("system.versionNumber.minor", minor);

  // Default

  interpreter.PutDefaultFunction(CompiledFunction {
    FuncDef { FuncDef::AnyArgs(Literal::TypeInstance), FuncDef::NoArgs() },
    &StdLib::Print
  }); 

  // Interpreter

  symbols.PutSymbolFunction("print", &StdLib::Print, FuncDef { FuncDef::AnyArgs(Literal::TypeInstance), FuncDef::NoArgs() });
  symbols.PutSymbolFunction("quit", &StdLib::Quit, FuncDef { FuncDef::NoArgs(), FuncDef::NoArgs() });

  // Generic

  symbols.PutSymbolFunction("+", &StdLib::Add, FuncDef { FuncDef::AtleastOneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });

  // Numerical

  symbols.PutSymbolFunction("inc", &StdLib::Inc, FuncDef { FuncDef::OneArg(Number::TypeInstance), FuncDef::OneArg(Number::TypeInstance) });
  symbols.PutSymbolFunction("dec", &StdLib::Dec, FuncDef { FuncDef::OneArg(Number::TypeInstance), FuncDef::OneArg(Number::TypeInstance) });
  RegisterBinaryFunction(symbols, "-", &StdLib::Sub);
  RegisterBinaryFunction(symbols, "*", &StdLib::Mult);
  RegisterBinaryFunction(symbols, "/", &StdLib::Div);
  RegisterBinaryFunction(symbols, "%", &StdLib::Mod);

  // Bitwise

  RegisterBinaryFunction(symbols, "<<", &StdLib::LeftShift);
  RegisterBinaryFunction(symbols, ">>", &StdLib::RightShift);
  RegisterBinaryFunction(symbols, "&", &StdLib::BitAnd);
  RegisterBinaryFunction(symbols, "|", &StdLib::BitOr);
  RegisterBinaryFunction(symbols, "^", &StdLib::BitXor);
  symbols.PutSymbolFunction("~", &StdLib::BitNot, FuncDef { FuncDef::OneArg(Number::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });

  // String 

  symbols.PutSymbolFunction("reverse", &StdLib::Reverse, FuncDef { FuncDef::OneArg(String::TypeInstance), FuncDef::OneArg(String::TypeInstance) });

  // Logical

  symbols.PutSymbolBool("true", true);
  symbols.PutSymbolBool("false", false);

  // Lists

  symbols.PutSymbolQuote("nil", ExpressionPtr { new Sexp { } });
  interpreter.PutListFunction(CompiledFunction {
    FuncDef { FuncDef::AnyArgs(), FuncDef::OneArg(Quote::TypeInstance) },
    &StdLib::List
  });
  symbols.PutSymbolFunction("map", &StdLib::Map, FuncDef { FuncDef::Args({&Function::TypeInstance, &Quote::TypeInstance}), FuncDef::OneArg(Quote::TypeInstance) });
  symbols.PutSymbolFunction("head", &StdLib::Head, FuncDef { FuncDef::OneArg(Quote::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });
  symbols.PutSymbolFunction("tail", &StdLib::Tail, FuncDef { FuncDef::OneArg(Quote::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });

  // Comparison

  RegisterComparator(symbols, "=", &StdLib::Eq);
  RegisterComparator(symbols, "!=", &StdLib::Ne);
  RegisterComparator(symbols, "<", &StdLib::Lt);
  RegisterComparator(symbols, ">", &StdLib::Gt);
  RegisterComparator(symbols, "<=", &StdLib::Lte);
  RegisterComparator(symbols, ">=", &StdLib::Gte);

  // Branching, scoping, evaluation

  symbols.PutSymbolFunction("quote", &StdLib::QuoteFn, FuncDef { FuncDef::OneArg(Sexp::TypeInstance), FuncDef::OneArg(Quote::TypeInstance) });
  symbols.PutSymbolFunction("unquote", &StdLib::Unquote, FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });
  symbols.PutSymbolFunction("if", &StdLib::If, FuncDef { 
    FuncDef::Args({&Bool::TypeInstance, &Sexp::TypeInstance, &Sexp::TypeInstance}),
    FuncDef::OneArg(Literal::TypeInstance)
  });
  symbols.PutSymbolFunction("let", &StdLib::Let, FuncDef { FuncDef::AnyArgs(Sexp::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });
  symbols.PutSymbolFunction("begin", &StdLib::Begin, FuncDef { FuncDef::AnyArgs(), FuncDef::OneArg(Literal::TypeInstance) });
  symbols.PutSymbolFunction("lambda", &StdLib::Lambda, FuncDef { FuncDef::ManyArgs(Sexp::TypeInstance, 2), FuncDef::OneArg(Function::TypeInstance) });
  symbols.PutSymbolFunction("fn", &StdLib::Lambda, FuncDef { FuncDef::ManyArgs(Sexp::TypeInstance, 2), FuncDef::OneArg(Function::TypeInstance) });
  symbols.PutSymbolFunction("def", &StdLib::Def, 
    FuncDef { FuncDef::Args({&Symbol::TypeInstance, &Sexp::TypeInstance, &Sexp::TypeInstance}),
    FuncDef::OneArg(Function::TypeInstance)
  });
  symbols.PutSymbolFunction("apply", &StdLib::Apply, FuncDef { FuncDef::Args({ &Function::TypeInstance, &Sexp::TypeInstance }), FuncDef::OneArg(Literal::TypeInstance) });

  // Symbol Table
  
  symbols.PutSymbolFunction("set", &StdLib::Set, FuncDef { FuncDef::Args({&Symbol::TypeInstance, &Literal::TypeInstance}), FuncDef::OneArg(Literal::TypeInstance) });
  symbols.PutSymbolFunction("unset", &StdLib::UnSet, FuncDef { FuncDef::OneArg(Symbol::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });
  symbols.PutSymbolFunction("help", &StdLib::Help, FuncDef { FuncDef::AnyArgs(Symbol::TypeInstance), FuncDef::NoArgs() });
}

void StdLib::UnLoad(Interpreter &interpreter) {
  //TODO
}

// Interpreter Functions

bool StdLib::PrintExpression(Interpreter &interpreter, ExpressionPtr &curr, std::ostream &out) {
  if (auto *currE = dynamic_cast<Bool*>(curr.get()))
    return PrintBool(currE->Value, out);
  else if (auto *currE = dynamic_cast<Number*>(curr.get()))
    return PrintLiteral(currE, out);
  else if (auto *currE = dynamic_cast<String*>(curr.get())) {
    char wrapper = '"';
    return PrintLiteral(currE, out, &wrapper);
  }
  else if (auto *currE = dynamic_cast<Symbol*>(curr.get())) {
    String temp { currE->Value };
    return PrintLiteral(&temp, out);
  }
  else if (auto *currE = dynamic_cast<Function*>(curr.get())) {
    String temp { "<Function>" };
    return PrintLiteral(&temp, out);
  }
  else if (auto *currE = dynamic_cast<Quote*>(curr.get())) {
    return PrintExpression(interpreter, currE->Value, out);
  }
  else if (auto *currE = dynamic_cast<Sexp*>(curr.get()))
    return PrintSexp(interpreter, *currE, out);
  else
    return interpreter.PushError(EvalError { "print", "Invalid expression type: " + curr->ToString() });
}

bool StdLib::Print(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  ExpressionPtr curr;
  auto &cmdInterface = interpreter.GetCommandInterface();
  while (!args.empty()) {
    std::stringstream out;
    curr = std::move(args.front());
    args.pop_front();
    bool result = PrintExpression(interpreter, curr, out);
    out << std::endl;
    if (!result)
      return false;
    cmdInterface.WriteOutputLine(out.str());
  }

  expr = GetNil();
  return true;
}

bool StdLib::Quit(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  interpreter.Stop();
  expr = ExpressionPtr { new Number { 0 } };
  return true;
}

// Generic Functions

bool StdLib::Add(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  auto currArg = args.begin();
  if (currArg != args.end()) {
    if (interpreter.EvaluatePartial(*currArg)) {
      auto &type = (*currArg)->Type();
      if (TypeHelper::IsConvertableToNumber(type))
        return AddNum(interpreter, expr, args);
      else if (&type == &String::TypeInstance)
        return AddString(interpreter, expr, args);
      else if (&type == &Quote::TypeInstance)
        return AddList(interpreter, expr, args);
      else
        return TypeError(interpreter, "+", "string, number or list", *currArg);
    }
    else
      return interpreter.PushError(EvalError { "+", "Failed to evaluate first arg" });
  }
  else
    return interpreter.PushError(EvalError { "+", "Expected at least one argument" });
}

// Number Functions

bool StdLib::Inc(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  return UnaryNumberFn("inc", interpreter, expr, args, [](int64_t num) { return num + 1; });
}

bool StdLib::Dec(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  return UnaryNumberFn("dec", interpreter, expr, args, [](int64_t num) { return num - 1; });
}

template <class F>
bool StdLib::UnaryNumberFn(const std::string &name, Interpreter &interpreter, ExpressionPtr &expr, ArgList &args, F fn) {
  ExpressionPtr numExpr { TypeHelper::GetNumber(std::move(args.front())) };
  auto num = dynamic_cast<Number*>(numExpr.get());
  if (num) {
    expr = ExpressionPtr { new Number { fn(num->Value) } };
    return true;
  }
  return TypeError(interpreter, name, "number", args.front() );
}

bool StdLib::AddNum(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  auto fn = [](int64_t a, int64_t b) { return a + b; };
  return BinaryFunction(interpreter, expr, args, fn, "+");
}

bool StdLib::Sub(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  auto fn = [](int64_t a, int64_t b) { return a - b; };
  return BinaryFunction(interpreter, expr, args, fn, "-");
}

bool StdLib::Mult(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  auto fn = [](int64_t a, int64_t b) { return a * b; };
  return BinaryFunction(interpreter, expr, args, fn, "*");
}

bool StdLib::CheckDivideByZero(const std::string &name, Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  int argNum = 0;
  for (auto &arg : args) {
    if (argNum) {
      ExpressionPtr numExpr = TypeHelper::GetNumber(arg);
      if (numExpr) {
        auto *num = dynamic_cast<Number*>(numExpr.get());
        if (num && num->Value == 0)
          return interpreter.PushError(EvalError { name, "Divide by zero" });
      }
      else
        return TypeError(interpreter, name, "number", arg);
    }
    ++argNum;
  }
  return true;
}

bool StdLib::Div(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  if (CheckDivideByZero("/", interpreter, expr, args)) {
    auto fn = [](int64_t a, int64_t b) { return a / b; };
    return BinaryFunction(interpreter, expr, args, fn, "/");
  }
  return false;
}

bool StdLib::Mod(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  if (CheckDivideByZero("%", interpreter, expr, args)) {
    auto fn = [](int64_t a, int64_t b) { return a % b; };
    return BinaryFunction(interpreter, expr, args, fn, "%");
  }
  return false;
}

// Bitwise Functions

bool StdLib::LeftShift(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  auto fn = [](int64_t a, int64_t b) { return a << b; };
  return BinaryFunction(interpreter, expr, args, fn, "<<");
}

bool StdLib::RightShift(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  auto fn = [](int64_t a, int64_t b) { return a >> b; };
  return BinaryFunction(interpreter, expr, args, fn, ">>");
}

bool StdLib::BitAnd(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  auto fn = [](int64_t a, int64_t b) { return a & b; };
  return BinaryFunction(interpreter, expr, args, fn, "&");
}

bool StdLib::BitOr(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  auto fn = [](int64_t a, int64_t b) { return a | b; };
  return BinaryFunction(interpreter, expr, args, fn, "|");
}

bool StdLib::BitXor(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  auto fn = [](int64_t a, int64_t b) { return a ^ b; };
  return BinaryFunction(interpreter, expr, args, fn, "^");
}

bool StdLib::BitNot(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  ExpressionPtr numExpr = TypeHelper::GetNumber(args.front());
  if (numExpr) {
    auto *num = dynamic_cast<Number*>(numExpr.get());
    if (num) {
      expr = ExpressionPtr { new Number(~num->Value) };
      return true;
    }
  }
  return TypeError(interpreter, "~", "number", args.front());
}

// String functions

bool StdLib::AddString(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  std::stringstream ss;
  while (!args.empty()) {
    auto str = dynamic_cast<String*>(args.front().get());
    if (str)
      ss << str->Value;
    else
      return TypeError(interpreter, "+", "string", args.front());

    args.pop_front();
  }

  expr = ExpressionPtr { new String { ss.str() } };
  return true;
}

bool StdLib::Reverse(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  ExpressionPtr arg = std::move(args.front());
  args.pop_front();
  auto argString = static_cast<String*>(arg.get());
  std::reverse(argString->Value.begin(), argString->Value.end());

  expr = std::move(arg);
  return true;
}

// Lists

bool StdLib::List(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  ExpressionPtr listExpr { new Sexp {} };
  auto list = static_cast<Sexp*>(listExpr.get());
  while (!args.empty()) {
    list->Args.push_back(std::move(args.front()));
    args.pop_front();
  }

  expr = ExpressionPtr { new Quote { std::move(listExpr) } };
  return true;
}

bool StdLib::Map(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  ExpressionPtr fnExpr { std::move(args.front()) };
  args.pop_front();

  ExpressionPtr quoteExpr { std::move(args.front()) };
  args.pop_front();

  ExpressionPtr resultExpr { new Sexp {} };

  auto fn = static_cast<Function*>(fnExpr.get());
  auto quote = static_cast<Quote*>(quoteExpr.get());
  auto list = dynamic_cast<Sexp*>(quote->Value.get());
  auto resultList = static_cast<Sexp*>(resultExpr.get());
  int i = 0;
  if (list) {
    for (auto &item : list->Args) {
      ExpressionPtr evalExpr { new Sexp { } };
      auto evalSexp = static_cast<Sexp*>(evalExpr.get());
      evalSexp->Args.push_back(fn->Clone());
      evalSexp->Args.push_back(item->Clone());
      if (!interpreter.EvaluatePartial(evalExpr))
        return interpreter.PushError(EvalError { 
          "map",
          "Failed to call " +  fn->ToString() + " on item " + std::to_string(i)
        });
      resultList->Args.push_back(std::move(evalExpr));
      ++i;
    }
  }
  else 
    return TypeError(interpreter, "map", "list", quote->Value);

  expr = ExpressionPtr { new Quote { std::move(resultExpr) } };
  return true;
}

bool StdLib::AddList(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  ExpressionPtr resultExpr { new Sexp {} };
  auto resultList = static_cast<Sexp*>(resultExpr.get());
  while (!args.empty()) {
    auto quote = dynamic_cast<Quote*>(args.front().get());
    if (quote) {
      auto list = dynamic_cast<Sexp*>(quote->Value.get());
      if (list) {
        while (!list->Args.empty()) {
          resultList->Args.push_back(std::move(list->Args.front()));
          list->Args.pop_front();
        }
      }
      else
        return TypeError(interpreter, "+", "list", quote->Value);
    }
    else
      return TypeError(interpreter, "+", "list", args.front());

    args.pop_front();
  }

  expr = ExpressionPtr { new Quote { std::move(resultExpr) } };
  return true;
}

bool StdLib::Head(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  ExpressionPtr quoteExpr { std::move(args.front()) };
  auto quote = static_cast<Quote*>(quoteExpr.get());
  auto list = dynamic_cast<Sexp*>(quote->Value.get());
  if (list) {
    if (list->Args.empty())
      expr = StdLib::GetNil();
    else {
      expr = std::move(list->Args.front());
      list->Args.pop_front();
    }
    return true;
  }
  else
    return TypeError(interpreter, "head", "list", quote->Value);
}

bool StdLib::Tail(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  ExpressionPtr quoteExpr { std::move(args.front()) };
  auto quote = static_cast<Quote*>(quoteExpr.get());
  auto list = dynamic_cast<Sexp*>(quote->Value.get());
  if (list) {
    if (list->Args.empty())
      expr = StdLib::GetNil();
    else {
      list->Args.pop_front();
      ExpressionPtr tail { new Sexp {} };
      auto newList = dynamic_cast<Sexp*>(tail.get());
      while (!list->Args.empty()) {
        newList->Args.push_back(std::move(list->Args.front()));
        list->Args.pop_front();
      }
      expr = ExpressionPtr { new Quote { std::move(tail) } };
    }
    return true;
  }
  else
    return TypeError(interpreter, "tail", "list", quote->Value);
}

// Logical

bool StdLib::And(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  return false;
}

//bool StdLib::Or(Interpreter &interpreter, ExpressionPtr &expr) {
//  return false;
//}
//
//bool StdLib::Not(Interpreter &interpreter, ExpressionPtr &expr) {
//  return false;
//}

// Comparison

template <class T>
bool EqT(Bool &r, T &a, T &b) {
  return r.Value && a == b;
}

template <class T>
bool NeT(Bool &r, T &a, T &b) {
  return r.Value && !EqT(r, a, b);
}

template <class T>
bool LtT(Bool &r, T &a, T &b) {
  return r.Value && a < b;
}

template <class T>
bool GteT(Bool &r, T &a, T &b) {
  return r.Value && !LtT(r, a, b);
}

template <class T>
bool GtT(Bool &r, T &a, T &b) {
  return r.Value && (GteT(r, a, b) && !EqT(r, a, b));
}

template <class T>
bool LteT(Bool &r, T &a, T &b) {
  return r.Value && (LtT(r, a, b) || EqT(r, a, b));
}

bool StdLib::Eq(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  return BinaryPredicate("=", interpreter, expr, args, EqT<Bool>, EqT<Number>, EqT<String>);
}

bool StdLib::Ne(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  return BinaryPredicate("!=", interpreter, expr, args, NeT<Bool>, NeT<Number>, NeT<String>);
}

bool StdLib::Lt(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  return BinaryPredicate("<", interpreter, expr, args, LtT<Bool>, LtT<Number>, LtT<String>);
}

bool StdLib::Gt(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  return BinaryPredicate(">", interpreter, expr, args, GtT<Bool>, GtT<Number>, GtT<String>);
}

bool StdLib::Lte(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  return BinaryPredicate("<=", interpreter, expr, args, LteT<Bool>, LteT<Number>, LteT<String>);
}

bool StdLib::Gte(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  return BinaryPredicate(">=", interpreter, expr, args, GteT<Bool>, GteT<Number>, GteT<String>);
}

// Branching, scoping and evaluation

bool StdLib::QuoteFn(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  ExpressionPtr quote { new Quote { std::move(args.front()) } };
  args.pop_front();
  expr = std::move(quote);
  return true;
}

bool StdLib::Unquote(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  ExpressionPtr quoteExpr { std::move(args.front()) };
  ExpressionPtr toEvaluate;
  if (auto quote = dynamic_cast<Quote*>(quoteExpr.get()))
    toEvaluate = std::move(quote->Value);
  else
    toEvaluate = std::move(quoteExpr);

  if (interpreter.EvaluatePartial(toEvaluate)) {
    expr = std::move(toEvaluate);
    return true;
  }
  else
    return interpreter.PushError(EvalError { "unquote", "Failed to evaluate expression" });
}

bool StdLib::If(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  ExpressionPtr condExpr = std::move(args.front());
  args.pop_front();

  ExpressionPtr trueExpr = std::move(args.front());
  args.pop_front();
   
  ExpressionPtr falseExpr = std::move(args.front());
  args.pop_front();

  auto cond = static_cast<Bool*>(condExpr.get());
  ExpressionPtr *branchExpr = cond->Value ? &trueExpr : &falseExpr;
  if (interpreter.EvaluatePartial(*branchExpr)) {
    expr = std::move(*branchExpr);
    return true;
  }
  else
    return interpreter.PushError(EvalError { "if", "Failed to evaluate branch" } );
}

bool StdLib::Let(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  Scope scope(interpreter.GetCurrentStackFrame().GetLocalSymbols());
  ExpressionPtr varsExpr = std::move(args.front());
  args.pop_front();

  auto vars = static_cast<Sexp*>(varsExpr.get());
  for (auto &varExpr : vars->Args) {
    auto var = dynamic_cast<Sexp*>(varExpr.get());
    if (var) {
      size_t nVarArgs = var->Args.size();
      if (nVarArgs == 2) {
        ExpressionPtr varNameExpr = std::move(var->Args.front());
        var->Args.pop_front();

        ExpressionPtr varValueExpr = std::move(var->Args.front());
        var->Args.pop_front();

        auto varName = dynamic_cast<Symbol*>(varNameExpr.get());
        if (varName) {
          if (interpreter.EvaluatePartial(varValueExpr))
            scope.PutSymbol(varName->Value, varValueExpr);
          else
            return interpreter.PushError(EvalError { "let", "Failed to evaluate value for " + varName->Value } );
        }
        else
          return TypeError(interpreter, "let", "symbol", varNameExpr);
      }
      else
        return interpreter.PushError(EvalError {
          "let",
          "Expected 2 args: (name1 value1). Got " + std::to_string(nVarArgs) + " args"
        });
    }
    else
      return TypeError(interpreter, "let", "(name1 value1)", varExpr);
  }

  return Begin(interpreter, expr, args);
}

bool StdLib::Begin(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  ExpressionPtr currCodeExpr;
  while (!args.empty()) {
    currCodeExpr = std::move(args.front());
    args.pop_front();

    if (!interpreter.EvaluatePartial(currCodeExpr))
      return interpreter.PushError(EvalError { "let", "Failed to evaluate let body" });
  }

  expr = std::move(currCodeExpr);
  return true;
}

bool StdLib::Lambda(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  ExpressionPtr formalsExpr = std::move(args.front());
  args.pop_front();

  ExpressionPtr codeExpr = std::move(args.front());
  args.pop_front();
  
  ArgList anonFuncArgs;
  int nArgs = 0;
  if (LambdaPrepareFormals(interpreter, formalsExpr, anonFuncArgs, nArgs)) {
    ExpressionPtr func {
      new InterpretedFunction {
        FuncDef {
          FuncDef::ManyArgs(Literal::TypeInstance, nArgs),
          FuncDef::AnyArgs()
        },
        std::move(codeExpr),
        std::move(anonFuncArgs)
      } 
    };
    auto *interpFunc = static_cast<InterpretedFunction*>(func.get());
    auto &locals = interpreter.GetCurrentStackFrame().GetLocalSymbols();
    locals.ForEach([interpFunc](const std::string &name, ExpressionPtr &value) {
      interpFunc->Closure.emplace(name, value->Clone());
    });
    expr = std::move(func);
    return true;
  }
  else
    return false;
}

bool StdLib::LambdaPrepareFormals(Interpreter &interpreter, ExpressionPtr &formalsExpr, ArgList &anonFuncArgs, int &nArgs) {
  auto formalsList = dynamic_cast<Sexp*>(formalsExpr.get());
  if (formalsList) {
    for (auto &formal : formalsList->Args) {
      auto formalSym = dynamic_cast<Symbol*>(formal.get());
      if (formalSym) {
        anonFuncArgs.push_back(formalSym->Clone());
      }
      else
        return TypeError(interpreter, "lambda", "Symbol", formal);
      ++nArgs;
    }
  }
  else
    return TypeError(interpreter, "lambda", "formals list", formalsExpr);
  
  return true;
}

bool StdLib::Def(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  ExpressionPtr symbolExpr { std::move(args.front()) };
  args.pop_front();

  ExpressionPtr lambdaExpr { new Sexp { } };
  auto lambda = static_cast<Sexp*>(lambdaExpr.get());
  lambda->Args.push_back(ExpressionPtr { new Symbol { "lambda" } });
  lambda->Args.push_back(std::move(args.front()));
  args.pop_front();
  lambda->Args.push_back(std::move(args.front()));
  args.pop_front();

  if (interpreter.EvaluatePartial(lambdaExpr)) {
    ExpressionPtr setExpr { new Sexp {} };
    auto set = static_cast<Sexp*>(setExpr.get());
    set->Args.push_back(ExpressionPtr { new Symbol { "set" } });
    set->Args.push_back(std::move(symbolExpr));
    set->Args.push_back(std::move(lambdaExpr)); 
    if (interpreter.EvaluatePartial(setExpr)) {
      expr = std::move(setExpr);
      return true;
    }
    else
      return interpreter.PushError(EvalError { "def", "evaluating set expression failed" });
  }
  else
    return interpreter.PushError(EvalError { "def", "evaluating lambda expression failed" });
}

bool StdLib::Apply(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  ExpressionPtr applicationExpr { new Sexp { } };
  auto application = static_cast<Sexp*>(applicationExpr.get());
  application->Args.push_back(std::move(args.front()));
  args.pop_front();

  ExpressionPtr appArgsExpr { std::move(args.front()) };
  args.pop_front();

  if (interpreter.EvaluatePartial(appArgsExpr)) {
    ExpressionPtr args;
    if (auto quote = dynamic_cast<Quote*>(appArgsExpr.get())) {
      args = std::move(quote->Value);
    }
    else
      args = std::move(appArgsExpr);

    auto appArgsSexp = dynamic_cast<Sexp*>(args.get());
    if (appArgsSexp) {
      while (!appArgsSexp->Args.empty()) {
        application->Args.push_back(std::move(appArgsSexp->Args.front()));
        appArgsSexp->Args.pop_front();
      }

      if (interpreter.EvaluatePartial(applicationExpr)) {
        expr = std::move(applicationExpr);
        return true;
      }
      else
        return interpreter.PushError(EvalError { "apply", "failed to evaluate function application" });
    }
    else
      return TypeError(interpreter, "apply", "list", appArgsExpr);
  }
  else
    return interpreter.PushError(EvalError { "apply", "failed to evaluate argument list" });
}

// Symbol Table Functions

bool StdLib::Set(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  ExpressionPtr sym = std::move(args.front());
  args.pop_front();
    
  ExpressionPtr value = std::move(args.front());
  args.pop_front();

  std::string symName = "";
  auto symE = static_cast<Symbol*>(sym.get());
  symName = symE->Value;

  interpreter.GetCurrentStackFrame().PutDynamicSymbol(symName, value->Clone());
  expr = std::move(value);
  return true;
}

bool StdLib::UnSet(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  ExpressionPtr sym = std::move(args.front());
  args.pop_front();

  std::string symName = "";
  auto symE = static_cast<Symbol*>(sym.get());
  symName = symE->Value;

  ExpressionPtr value;
  auto &currFrame = interpreter.GetCurrentStackFrame();
  if (currFrame.GetSymbol(symName, value)) {
    currFrame.DeleteSymbol(symName);
    expr = std::move(value);
    return true;
  }
  else
    return UnknownSymbol(interpreter, "unset", symName);
}

bool StdLib::Help(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args) {
  std::string defaultSexp = interpreter.GetDefaultSexp();
  std::stringstream ss;
  Interpreter::SymbolFunctor functor = [&ss, &defaultSexp](const std::string &symbolName, ExpressionPtr &expr) {
    if (&expr->Type() == &Function::TypeInstance && symbolName != defaultSexp) {
      auto fn = static_cast<Function*>(expr.get());
      ss << "(" << symbolName << fn->Def.ToString() << std::endl;
    }
  };

  auto &symbols = interpreter.GetDynamicSymbols();
  if (args.empty())
    symbols.ForEach(functor);
  else {
    while (!args.empty()) {
      ExpressionPtr currArg = std::move(args.front());
      args.pop_front();

      if (&currArg->Type() == &Symbol::TypeInstance) {
        auto sym = static_cast<Symbol*>(currArg.get());
        ExpressionPtr currValue;
        if (symbols.GetSymbol(sym->Value, currValue)) {
          functor(sym->Value, currValue);
        }
        else
          return UnknownSymbol(interpreter, "help", sym->Value);
      }
    }
  }

  expr = GetNil();
  interpreter.GetCommandInterface().WriteOutputLine(ss.str());
  return true;
}

// Helpers

template<class T> bool StdLib::PrintLiteral(T *expr, std::ostream &out, char *wrapper) {
  if (wrapper)
    out << *wrapper;
  out << expr->Value;
  if (wrapper)
    out << *wrapper;
  return true;
}

bool StdLib::PrintSexp(Interpreter &interpreter, Sexp &sexp, std::ostream &out) {
  out << "(";
  bool first = true;
  for (auto &arg : sexp.Args) {
    if (!first)
      out << " ";
    bool result = PrintExpression(interpreter, arg, out);
    if (!result)
      return false;
    first = false;
  }
  out << ")";
  return true;
}

bool StdLib::PrintBool(bool expr, std::ostream &out) {
  if (expr)
    out << "true";
  else
    out << "false";
  return true;
}

template<class F>
static bool StdLib::BinaryFunction(Interpreter &interpreter, ExpressionPtr &expr, ArgList &args, F fn, const std::string &name) {
  bool first = true;
  Number result { 0 };
  while (!args.empty()) {
    bool ok = false;
    ExpressionPtr numExpr = TypeHelper::GetNumber(args.front());
    if (numExpr) {
      auto num = dynamic_cast<Number*>(numExpr.get());
      if (num) {
        if (first)
          result.Value = num->Value;
        else
          result.Value = fn(result.Value, num->Value);
        ok = true;
      }
    }
    if (!ok)
      return TypeError(interpreter, name, "number", args.front());

    first = false;
    args.pop_front();
  }

  expr = ExpressionPtr { new Number { result } };
  return true;
}

template <class B, class N, class S>
bool StdLib::BinaryPredicate(const std::string &name, Interpreter &interpreter, ExpressionPtr &expr, ArgList &args, B bFn, N nFn, S sFn) {
  auto currArg = args.begin();
  if (currArg != args.end()) {
    auto &type = (*currArg)->Type();

    Bool defaultValue { true };
    if (bFn && (&type == &Bool::TypeInstance))
      return PredicateHelper<Bool>(name, interpreter, expr, args, bFn, defaultValue);
    else if (nFn && (&type == &Number::TypeInstance))
      return PredicateHelper<Number>(name, interpreter, expr, args, nFn, defaultValue);
    else if (sFn && (&type == &String::TypeInstance))
      return PredicateHelper<String>(name, interpreter, expr, args, sFn, defaultValue);
    else
      return TypeError(interpreter, name, "literal", *currArg);
  }
  else
    return interpreter.PushError(EvalError { name, "Expected at least one argument" });
}

template <class T, class F, class R>
bool StdLib::PredicateHelper(const std::string &name, Interpreter &interpreter, ExpressionPtr &expr, ArgList &args, F fn, R defaultResult) {
  R result { defaultResult };
  ExpressionPtr firstArg = std::move(args.front());
  args.pop_front();
  auto last = dynamic_cast<T*>(firstArg.get());
  if (last) {
    while (!args.empty()) {
      auto curr = dynamic_cast<T*>(args.front().get());
      if (curr) {
        R tmp { fn(result, *last, *curr) };
        result = tmp;
      }
      else
        return TypeError(interpreter, name, T::TypeInstance.TypeName, args.front());

      *last = *curr;
      args.pop_front();
    }
  }
  else
    return TypeError(interpreter, name, T::TypeInstance.TypeName, firstArg);

  expr = ExpressionPtr { result.Clone() };
  return true;
}

void StdLib::RegisterBinaryFunction(SymbolTable &symbolTable, const std::string& name, SlipFunction fn) {
  symbolTable.PutSymbolFunction(name, fn, FuncDef { FuncDef::AtleastOneArg(Number::TypeInstance), FuncDef::OneArg(Number::TypeInstance) });
}

void StdLib::RegisterComparator(SymbolTable &symbolTable, const std::string& name, SlipFunction fn) {
  symbolTable.PutSymbolFunction(name, fn, FuncDef { FuncDef::AnyArgs(Literal::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) });
}

bool StdLib::UnknownSymbol(Interpreter &interpreter, const std::string &where, const std::string &symName) {
  return interpreter.PushError(EvalError { "unset", "Unknown symbol: " + symName });
}

bool StdLib::TypeError(Interpreter &interpreter, const std::string &where, const ExpressionPtr &expected, const ExpressionPtr &actual) {
  return TypeError(interpreter, where, expected->Type().TypeName, actual);
}

bool StdLib::TypeError(Interpreter &interpreter, const std::string &where, const std::string &expectedName, const ExpressionPtr &actual) {
  return interpreter.PushError(EvalError {
    where,
    expectedName + " expected. Got " + actual->Type().TypeName
  });
}

ExpressionPtr StdLib::GetNil() {
  return ExpressionPtr { new Quote { ExpressionPtr { new Sexp {} } } };
}
