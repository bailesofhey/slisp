#include <iostream>
#include <memory>
#include <sstream>

#include "StdLib.h"
#include "../Interpreter.h"

//=============================================================================

void StdLib::Load(Interpreter &interpreter) {
  // Constants

  auto &symbols = interpreter.GetDynamicSymbols();
  auto &settings = interpreter.GetSettings();

  int major = 0,
      minor = 1;
  symbols.PutSymbolString("system.version", "Slip " + std::to_string(major) + "." + std::to_string(minor));
  symbols.PutSymbolNumber("system.versionNumber.major", major);
  symbols.PutSymbolNumber("system.versionNumber.minor", minor);

  // Default

  settings.PutDefaultFunction(CompiledFunction {
    FuncDef { FuncDef::AnyArgs(), FuncDef::NoArgs() },
    &StdLib::DefaultFunction
  }); 

  // Interpreter

  symbols.PutSymbolFunction("print", &StdLib::Print, FuncDef { FuncDef::AnyArgs(Literal::TypeInstance), FuncDef::NoArgs() });
  symbols.PutSymbolFunction("quit", &StdLib::Quit, FuncDef { FuncDef::NoArgs(), FuncDef::NoArgs() });
  symbols.PutSymbolFunction("help", &StdLib::Help, FuncDef { FuncDef::AnyArgs(Symbol::TypeInstance), FuncDef::NoArgs() });


  symbols.PutSymbolFunction("infix-register", &StdLib::InfixRegister, FuncDef { FuncDef::OneArg(Symbol::TypeInstance), FuncDef::NoArgs() });
  symbols.PutSymbolFunction("infix-unregister", &StdLib::InfixUnregister, FuncDef { FuncDef::OneArg(Symbol::TypeInstance), FuncDef::NoArgs() });

  // Assignment Operators
  FuncDef setDef { FuncDef::Args({&Symbol::TypeInstance, &Literal::TypeInstance}), FuncDef::OneArg(Literal::TypeInstance) };
  symbols.PutSymbolFunction("set", &StdLib::Set, setDef.Clone()); 
  symbols.PutSymbolFunction("=", &StdLib::Set, setDef.Clone()); 
  symbols.PutSymbolFunction("+=", &StdLib::Set, setDef.Clone()); 
  symbols.PutSymbolFunction("-=", &StdLib::Set, setDef.Clone()); 
  symbols.PutSymbolFunction("*=", &StdLib::Set, setDef.Clone()); 
  symbols.PutSymbolFunction("/=", &StdLib::Set, setDef.Clone()); 
  symbols.PutSymbolFunction("%=", &StdLib::Set, setDef.Clone()); 
  symbols.PutSymbolFunction("<<=", &StdLib::Set, setDef.Clone()); 
  symbols.PutSymbolFunction(">>=", &StdLib::Set, setDef.Clone()); 
  symbols.PutSymbolFunction("&=", &StdLib::Set, setDef.Clone()); 
  symbols.PutSymbolFunction("^=", &StdLib::Set, setDef.Clone()); 
  symbols.PutSymbolFunction("|=", &StdLib::Set, setDef.Clone()); 

  symbols.PutSymbolFunction("++", &StdLib::PreIncrement, FuncDef { FuncDef::OneArg(Number::TypeInstance), FuncDef::OneArg(Number::TypeInstance) });
  symbols.PutSymbolFunction("--", &StdLib::PreDecrement, FuncDef { FuncDef::OneArg(Number::TypeInstance), FuncDef::OneArg(Number::TypeInstance) });

  symbols.PutSymbolFunction("unset", &StdLib::UnSet, FuncDef { FuncDef::OneArg(Symbol::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });

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
  settings.PutListFunction(CompiledFunction {
    FuncDef { FuncDef::AnyArgs(), FuncDef::OneArg(Quote::TypeInstance) },
    &StdLib::List
  });
  symbols.PutSymbolFunction("list", CompiledFunction {
    FuncDef { FuncDef::AnyArgs(), FuncDef::OneArg(Quote::TypeInstance) },
    &StdLib::List
  });
  symbols.PutSymbolFunction("map", &StdLib::Map, FuncDef { FuncDef::Args({&Function::TypeInstance, &Quote::TypeInstance}), FuncDef::OneArg(Quote::TypeInstance) });
  symbols.PutSymbolFunction("head", &StdLib::Head, FuncDef { FuncDef::OneArg(Quote::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });
  symbols.PutSymbolFunction("tail", &StdLib::Tail, FuncDef { FuncDef::OneArg(Quote::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });

  // Logical

  symbols.PutSymbolFunction("and", &StdLib::And, FuncDef { FuncDef::ManyArgs(Sexp::TypeInstance, 2, ArgDef::ANY_ARGS), FuncDef::OneArg(Bool::TypeInstance) });
  symbols.PutSymbolFunction("&&", &StdLib::And, FuncDef { FuncDef::ManyArgs(Sexp::TypeInstance, 2, ArgDef::ANY_ARGS), FuncDef::OneArg(Bool::TypeInstance) });

  symbols.PutSymbolFunction("or", &StdLib::Or, FuncDef { FuncDef::ManyArgs(Sexp::TypeInstance, 2, ArgDef::ANY_ARGS), FuncDef::OneArg(Bool::TypeInstance) });
  symbols.PutSymbolFunction("||", &StdLib::Or, FuncDef { FuncDef::ManyArgs(Sexp::TypeInstance, 2, ArgDef::ANY_ARGS), FuncDef::OneArg(Bool::TypeInstance) });

  symbols.PutSymbolFunction("not", &StdLib::Not, FuncDef { FuncDef::OneArg(Sexp::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) });
  symbols.PutSymbolFunction("!", &StdLib::Not, FuncDef { FuncDef::OneArg(Sexp::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) });

  // Comparison

  RegisterComparator(symbols, "==", &StdLib::Eq);
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
  symbols.PutSymbolFunction("let", &StdLib::Let, FuncDef { FuncDef::ManyArgs(Sexp::TypeInstance, 2, ArgDef::ANY_ARGS), FuncDef::OneArg(Literal::TypeInstance) });
  symbols.PutSymbolFunction("begin", &StdLib::Begin, FuncDef { FuncDef::AtleastOneArg(Sexp::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });
  symbols.PutSymbolFunction("lambda", &StdLib::Lambda, FuncDef { FuncDef::ManyArgs(Sexp::TypeInstance, 2), FuncDef::OneArg(Function::TypeInstance) });
  symbols.PutSymbolFunction("fn", &StdLib::Lambda, FuncDef { FuncDef::ManyArgs(Sexp::TypeInstance, 2), FuncDef::OneArg(Function::TypeInstance) });
  symbols.PutSymbolFunction("def", &StdLib::Def, 
    FuncDef { FuncDef::Args({&Symbol::TypeInstance, &Sexp::TypeInstance, &Sexp::TypeInstance}),
    FuncDef::OneArg(Function::TypeInstance)
  });
  symbols.PutSymbolFunction("apply", &StdLib::Apply, FuncDef { FuncDef::Args({ &Function::TypeInstance, &Sexp::TypeInstance }), FuncDef::OneArg(Literal::TypeInstance) });

  // Register infix operators by precedence (using C++ rules, where appropriate)
  settings.RegisterInfixSymbol("++");
  settings.RegisterInfixSymbol("--");

  settings.RegisterInfixSymbol("not");
  settings.RegisterInfixSymbol("!");
  settings.RegisterInfixSymbol("~");

  settings.RegisterInfixSymbol("*");
  settings.RegisterInfixSymbol("/");
  settings.RegisterInfixSymbol("%");

  settings.RegisterInfixSymbol("+");
  settings.RegisterInfixSymbol("-");

  settings.RegisterInfixSymbol("<<");
  settings.RegisterInfixSymbol(">>");

  settings.RegisterInfixSymbol("<");
  settings.RegisterInfixSymbol("<=");
  settings.RegisterInfixSymbol(">");
  settings.RegisterInfixSymbol(">=");

  settings.RegisterInfixSymbol("==");
  settings.RegisterInfixSymbol("!=");

  settings.RegisterInfixSymbol("&");

  settings.RegisterInfixSymbol("^");

  settings.RegisterInfixSymbol("|");

  settings.RegisterInfixSymbol("and");
  settings.RegisterInfixSymbol("&&");

  settings.RegisterInfixSymbol("or");
  settings.RegisterInfixSymbol("||");

  settings.RegisterInfixSymbol("=");
  settings.RegisterInfixSymbol("+=");
  settings.RegisterInfixSymbol("-=");
  settings.RegisterInfixSymbol("*=");
  settings.RegisterInfixSymbol("/=");
  settings.RegisterInfixSymbol("%=");
  settings.RegisterInfixSymbol("<<=");
  settings.RegisterInfixSymbol(">>=");
  settings.RegisterInfixSymbol("&=");
  settings.RegisterInfixSymbol("^=");
  settings.RegisterInfixSymbol("|=");
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

bool StdLib::EvaluateListSexp(EvaluationContext &ctx) {
  ExpressionPtr listExpr { new Sexp() };
  Sexp *sexp = static_cast<Sexp*>(listExpr.get());
  ArgListHelper::CopyTo(ctx.Args, sexp->Args);
  if (ctx.Interp.EvaluatePartial(listExpr)) {
    ctx.Args.clear();
    ctx.Args.push_back(std::move(listExpr));
    return Print(ctx);
  }
  else
    return ctx.Interp.PushError(EvalError { "<default function>", "Failed to evaluate list" });
}

bool StdLib::DefaultFunction(EvaluationContext &ctx) {
  if (ctx.Args.size() > 1)
    return EvaluateListSexp(ctx);
  return Print(ctx);
}

bool StdLib::Print(EvaluationContext &ctx) {
  ExpressionPtr curr;
  auto &cmdInterface = ctx.Interp.GetCommandInterface();
  int argNum = 1;
  while (!ctx.Args.empty()) {
    std::stringstream out;
    curr = std::move(ctx.Args.front());
    ctx.Args.pop_front();
    if (ctx.Interp.EvaluatePartial(curr)) {
      bool result = PrintExpression(ctx.Interp, curr, out);
      out << std::endl;
      if (!result)
        return false;
      cmdInterface.WriteOutputLine(out.str());
    }
    else
      return ctx.Interp.PushError(EvalError { "print", "Failed to evaluate arg " + std::to_string(argNum) });
    ++argNum;
  }

  ctx.Expr = GetNil();
  return true;
}

bool StdLib::Quit(EvaluationContext &ctx) {
  ctx.Interp.Stop();
  ctx.Expr = ExpressionPtr { new Number { 0 } };
  return true;
}

bool StdLib::Help(EvaluationContext &ctx) {
  std::string defaultSexp = ctx.Interp.GetSettings().GetDefaultSexp();
  std::stringstream ss;
  Interpreter::SymbolFunctor functor = [&ss, &defaultSexp](const std::string &symbolName, ExpressionPtr &expr) {
    if (&expr->Type() == &Function::TypeInstance && symbolName != defaultSexp) {
      auto fn = static_cast<Function*>(expr.get());
      ss << "(" << symbolName << fn->Def.ToString() << std::endl;
    }
  };

  auto &symbols = ctx.Interp.GetDynamicSymbols();
  if (ctx.Args.empty())
    symbols.ForEach(functor);
  else {
    while (!ctx.Args.empty()) {
      ExpressionPtr currArg = std::move(ctx.Args.front());
      ctx.Args.pop_front();

      if (&currArg->Type() == &Symbol::TypeInstance) {
        auto sym = static_cast<Symbol*>(currArg.get());
        ExpressionPtr currValue;
        if (symbols.GetSymbol(sym->Value, currValue)) {
          functor(sym->Value, currValue);
        }
        else
          return UnknownSymbol(ctx.Interp, "help", sym->Value);
      }
    }
  }

  ctx.Expr = GetNil();
  ctx.Interp.GetCommandInterface().WriteOutputLine(ss.str());
  return true;
}

bool StdLib::Set(EvaluationContext &ctx) {
  ExpressionPtr sym = std::move(ctx.Args.front());
  ctx.Args.pop_front();
    
  ExpressionPtr value = std::move(ctx.Args.front());
  ctx.Args.pop_front();

  std::string symName = "";
  auto symE = static_cast<Symbol*>(sym.get());
  symName = symE->Value;

  ExpressionPtr temp;
  auto &currStackFrame = ctx.Interp.GetCurrentStackFrame();
  if (currStackFrame.GetLocalSymbols().GetSymbol(symName, temp))
    currStackFrame.PutLocalSymbol(symName, value->Clone());
  else
    currStackFrame.PutDynamicSymbol(symName, value->Clone());

  ctx.Expr = std::move(value);
  return true;
}

bool StdLib::UnSet(EvaluationContext &ctx) {
  ExpressionPtr sym = std::move(ctx.Args.front());
  ctx.Args.pop_front();

  std::string symName = "";
  auto symE = static_cast<Symbol*>(sym.get());
  symName = symE->Value;

  ExpressionPtr value;
  auto &currFrame = ctx.Interp.GetCurrentStackFrame();
  if (currFrame.GetSymbol(symName, value)) {
    currFrame.DeleteSymbol(symName);
    ctx.Expr = std::move(value);
    return true;
  }
  else
    return UnknownSymbol(ctx.Interp, "unset", symName);
}

bool StdLib::InfixRegistrationFunction(EvaluationContext &ctx, const std::string &name, bool unregister) {
  auto sym = static_cast<Symbol&>(*ctx.Args.front());
  ExpressionPtr fnExpr;
  if (ctx.Interp.GetDynamicSymbols().GetSymbol(sym.Value, fnExpr)) {
    if (TypeHelper::TypeMatches(Function::TypeInstance, fnExpr->Type())) {
      auto &settings = ctx.Interp.GetSettings();
      if (unregister)
        settings.UnregisterInfixSymbol(sym.Value);
      else
        settings.RegisterInfixSymbol(sym.Value);
      ctx.Expr = GetNil(); 
      return true;
    }
    else
      return TypeError(ctx.Interp, name, "function", fnExpr);
  }
  else
    return ctx.Interp.PushError(EvalError { name, "Symbol not found" });
}

bool StdLib::InfixRegister(EvaluationContext &ctx) {
  return InfixRegistrationFunction(ctx, "infix-register", false);
}

bool StdLib::InfixUnregister(EvaluationContext &ctx) {
  return InfixRegistrationFunction(ctx, "infix-unregister", true);
}

// Generic Functions

bool StdLib::Add(EvaluationContext &ctx) {
  auto currArg = ctx.Args.begin();
  if (currArg != ctx.Args.end()) {
    if (ctx.Interp.EvaluatePartial(*currArg)) {
      auto &type = (*currArg)->Type();
      if (TypeHelper::IsConvertableToNumber(type))
        return AddNum(ctx);
      else if (&type == &String::TypeInstance)
        return AddString(ctx);
      else if (&type == &Quote::TypeInstance)
        return AddList(ctx);
      else
        return TypeError(ctx.Interp, "+", "string, number or list", *currArg);
    }
    else
      return ctx.Interp.PushError(EvalError { "+", "Failed to evaluate first arg" });
  }
  else
    return ctx.Interp.PushError(EvalError { "+", "Expected at least one argument" });
}

// Number Functions

bool StdLib::Inc(EvaluationContext &ctx) {
  return UnaryNumberFn(ctx, "inc", [](int64_t num) { return num + 1; });
}

bool StdLib::Dec(EvaluationContext &ctx) {
  return UnaryNumberFn(ctx, "dec", [](int64_t num) { return num - 1; });
}

bool StdLib::PreIncrement(EvaluationContext &ctx) {
  return false;
}

bool StdLib::PreDecrement(EvaluationContext &ctx) {
  return false;
}

template <class F>
bool StdLib::UnaryNumberFn(EvaluationContext &ctx, const std::string &name, F fn) {
  ExpressionPtr numExpr { TypeHelper::GetNumber(ctx.Args.front()) };
  auto num = dynamic_cast<Number*>(numExpr.get());
  if (num) {
    ctx.Expr = ExpressionPtr { new Number { fn(num->Value) } };
    return true;
  }
  return TypeError(ctx.Interp, name, "number", ctx.Args.front() );
}

bool StdLib::AddNum(EvaluationContext &ctx) {
  auto fn = [](int64_t a, int64_t b) { return a + b; };
  return BinaryFunction(ctx, fn, "+");
}

bool StdLib::Sub(EvaluationContext &ctx) {
  auto fn = [](int64_t a, int64_t b) { return a - b; };
  return BinaryFunction(ctx, fn, "-");
}

bool StdLib::Mult(EvaluationContext &ctx) {
  auto fn = [](int64_t a, int64_t b) { return a * b; };
  return BinaryFunction(ctx, fn, "*");
}

bool StdLib::CheckDivideByZero(EvaluationContext &ctx, const std::string &name) {
  int argNum = 0;
  for (auto &arg : ctx.Args) {
    if (argNum) {
      ExpressionPtr numExpr = TypeHelper::GetNumber(arg);
      if (numExpr) {
        auto *num = dynamic_cast<Number*>(numExpr.get());
        if (num && num->Value == 0)
          return ctx.Interp.PushError(EvalError { name, "Divide by zero" });
      }
      else
        return TypeError(ctx.Interp, name, "number", arg);
    }
    ++argNum;
  }
  return true;
}

bool StdLib::Div(EvaluationContext &ctx) {
  if (CheckDivideByZero(ctx, "/")) {
    auto fn = [](int64_t a, int64_t b) { return a / b; };
    return BinaryFunction(ctx, fn, "/");
  }
  return false;
}

bool StdLib::Mod(EvaluationContext &ctx) {
  if (CheckDivideByZero(ctx, "%")) {
    auto fn = [](int64_t a, int64_t b) { return a % b; };
    return BinaryFunction(ctx, fn, "%");
  }
  return false;
}

// Bitwise Functions

bool StdLib::LeftShift(EvaluationContext &ctx) {
  auto fn = [](int64_t a, int64_t b) { return a << b; };
  return BinaryFunction(ctx, fn, "<<");
}

bool StdLib::RightShift(EvaluationContext &ctx) {
  auto fn = [](int64_t a, int64_t b) { return a >> b; };
  return BinaryFunction(ctx, fn, ">>");
}

bool StdLib::BitAnd(EvaluationContext &ctx) {
  auto fn = [](int64_t a, int64_t b) { return a & b; };
  return BinaryFunction(ctx, fn, "&");
}

bool StdLib::BitOr(EvaluationContext &ctx) {
  auto fn = [](int64_t a, int64_t b) { return a | b; };
  return BinaryFunction(ctx, fn, "|");
}

bool StdLib::BitXor(EvaluationContext &ctx) {
  auto fn = [](int64_t a, int64_t b) { return a ^ b; };
  return BinaryFunction(ctx, fn, "^");
}

bool StdLib::BitNot(EvaluationContext &ctx) {
  ExpressionPtr numExpr = TypeHelper::GetNumber(ctx.Args.front());
  if (numExpr) {
    auto *num = dynamic_cast<Number*>(numExpr.get());
    if (num) {
      ctx.Expr = ExpressionPtr { new Number(~num->Value) };
      return true;
    }
  }
  return TypeError(ctx.Interp, "~", "number", ctx.Args.front());
}

// String functions

bool StdLib::AddString(EvaluationContext &ctx) {
  std::stringstream ss;
  while (!ctx.Args.empty()) {
    auto str = dynamic_cast<String*>(ctx.Args.front().get());
    if (str)
      ss << str->Value;
    else
      return TypeError(ctx.Interp, "+", "string", ctx.Args.front());

    ctx.Args.pop_front();
  }

  ctx.Expr = ExpressionPtr { new String { ss.str() } };
  return true;
}

bool StdLib::Reverse(EvaluationContext &ctx) {
  ExpressionPtr arg = std::move(ctx.Args.front());
  ctx.Args.pop_front();
  auto argString = static_cast<String*>(arg.get());
  std::reverse(argString->Value.begin(), argString->Value.end());

  ctx.Expr = std::move(arg);
  return true;
}

// Lists

bool StdLib::List(EvaluationContext &ctx) {
  ExpressionPtr listExpr { new Sexp {} };
  auto list = static_cast<Sexp*>(listExpr.get());
  int argNum = 1;
  while (!ctx.Args.empty()) {
    ExpressionPtr arg = std::move(ctx.Args.front());
    ctx.Args.pop_front();
    if (ctx.Interp.EvaluatePartial(arg))
      list->Args.push_back(std::move(arg));
    else
      return ctx.Interp.PushError(EvalError { "list", "failed to evaluate argument " + std::to_string(argNum) });
    ++argNum;
  }

  ctx.Expr = ExpressionPtr { new Quote { std::move(listExpr) } };
  return true;
}

bool StdLib::Map(EvaluationContext &ctx) {
  ExpressionPtr fnExpr { std::move(ctx.Args.front()) };
  ctx.Args.pop_front();

  ExpressionPtr quoteExpr { std::move(ctx.Args.front()) };
  ctx.Args.pop_front();

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
      if (!ctx.Interp.EvaluatePartial(evalExpr))
        return ctx.Interp.PushError(EvalError { 
          "map",
          "Failed to call " +  fn->ToString() + " on item " + std::to_string(i)
        });
      resultList->Args.push_back(std::move(evalExpr));
      ++i;
    }
  }
  else 
    return TypeError(ctx.Interp, "map", "list", quote->Value);

  ctx.Expr = ExpressionPtr { new Quote { std::move(resultExpr) } };
  return true;
}

bool StdLib::AddList(EvaluationContext &ctx) {
  ExpressionPtr resultExpr { new Sexp {} };
  auto resultList = static_cast<Sexp*>(resultExpr.get());
  while (!ctx.Args.empty()) {
    auto quote = dynamic_cast<Quote*>(ctx.Args.front().get());
    if (quote) {
      auto list = dynamic_cast<Sexp*>(quote->Value.get());
      if (list) {
        while (!list->Args.empty()) {
          resultList->Args.push_back(std::move(list->Args.front()));
          list->Args.pop_front();
        }
      }
      else
        return TypeError(ctx.Interp, "+", "list", quote->Value);
    }
    else
      return TypeError(ctx.Interp, "+", "list", ctx.Args.front());

    ctx.Args.pop_front();
  }

  ctx.Expr = ExpressionPtr { new Quote { std::move(resultExpr) } };
  return true;
}

bool StdLib::Head(EvaluationContext &ctx) {
  ExpressionPtr quoteExpr { std::move(ctx.Args.front()) };
  auto quote = static_cast<Quote*>(quoteExpr.get());
  auto list = dynamic_cast<Sexp*>(quote->Value.get());
  if (list) {
    if (list->Args.empty())
      ctx.Expr = StdLib::GetNil();
    else {
      ctx.Expr = std::move(list->Args.front());
      list->Args.pop_front();
    }
    return true;
  }
  else
    return TypeError(ctx.Interp, "head", "list", quote->Value);
}

bool StdLib::Tail(EvaluationContext &ctx) {
  ExpressionPtr quoteExpr { std::move(ctx.Args.front()) };
  auto quote = static_cast<Quote*>(quoteExpr.get());
  auto list = dynamic_cast<Sexp*>(quote->Value.get());
  if (list) {
    if (list->Args.empty())
      ctx.Expr = StdLib::GetNil();
    else {
      list->Args.pop_front();
      ExpressionPtr tail { new Sexp {} };
      auto newList = dynamic_cast<Sexp*>(tail.get());
      while (!list->Args.empty()) {
        newList->Args.push_back(std::move(list->Args.front()));
        list->Args.pop_front();
      }
      ctx.Expr = ExpressionPtr { new Quote { std::move(tail) } };
    }
    return true;
  }
  else
    return TypeError(ctx.Interp, "tail", "list", quote->Value);
}

// Logical

bool StdLib::BinaryLogicalFunc(EvaluationContext &ctx, bool isAnd) {
  int argNum = 1;
  while (!ctx.Args.empty()) {
    ExpressionPtr currArg = std::move(ctx.Args.front());
    ctx.Args.pop_front();
    if (ctx.Interp.EvaluatePartial(currArg)) {
      Bool *argValue = dynamic_cast<Bool*>(currArg.get());
      if (argValue) {
        bool value = argValue->Value;
        if (isAnd ? !value : value) {
          ctx.Expr = ExpressionPtr { new Bool(isAnd ? false : true) };
          return true;
        }
      }
      else
        return TypeError(ctx.Interp, isAnd ? "and" : "or", "bool", currArg);
    }
    else
      return ctx.Interp.PushError(EvalError { isAnd ? "and" : "or", "Failed to evaluate arg " + std::to_string(argNum) });

    ++argNum;
  }
  ctx.Expr = ExpressionPtr { new Bool(isAnd ? true : false) };
  return true;
}

bool StdLib::And(EvaluationContext &ctx) {
  return BinaryLogicalFunc(ctx, true);
}

bool StdLib::Or(EvaluationContext &ctx) {
  return BinaryLogicalFunc(ctx, false);
}

bool StdLib::Not(EvaluationContext &ctx) {
  if (!ctx.Args.empty()) {
    ExpressionPtr arg = std::move(ctx.Args.front());
    ctx.Args.pop_front();
    if (ctx.Interp.EvaluatePartial(arg)) {
      Bool *boolArg = dynamic_cast<Bool*>(arg.get());
      if (boolArg) {
        ctx.Expr = ExpressionPtr { new Bool(!boolArg->Value) };
        return true;
      }
      else
        return TypeError(ctx.Interp, "not", "bool", arg);
    }
    else
      return ctx.Interp.PushError(EvalError { "not", "Failed to evaluate arg 1" });
  }
  else
    return ctx.Interp.PushError(EvalError { "not", "Expected 1 arg" });
}

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

using ExpressionPredicate = std::function<bool(const Expression &, const Expression &)>;
bool ExpressionPredicateFn(EvaluationContext &ctx, const std::string &name, ExpressionPredicate fn) {
  int argNum = 0;
  if (!ctx.Args.empty()) {
    ExpressionPtr firstArg = std::move(ctx.Args.front());
    ctx.Args.pop_front();
    ++argNum;
    if (ctx.Interp.EvaluatePartial(firstArg)) {
      ExpressionPtr prevArg = std::move(firstArg);
      bool result = false;
      while (!ctx.Args.empty()) {
        ExpressionPtr currArg = std::move(ctx.Args.front());
        ctx.Args.pop_front();
        ++argNum;
        if (ctx.Interp.EvaluatePartial(currArg)) {
          if (!fn(*prevArg, *currArg)) {
            ctx.Expr = ExpressionPtr { new Bool(false) };
            return true;
          }
        }
        else
          return ctx.Interp.PushError(EvalError { name, "Failed to evaluate arg number " + std::to_string(argNum) });

        prevArg = std::move(currArg);
      }
      ctx.Expr = ExpressionPtr { new Bool(true) };
      return true;
    }
    else
      return ctx.Interp.PushError(EvalError { name, "Failed to evaluate arg number " + std::to_string(argNum) });
  }
  else
    return ctx.Interp.PushError(EvalError { name, "Expected at least one arg" });
}

bool StdLib::Eq(EvaluationContext &ctx) {
  return ExpressionPredicateFn(ctx, "==", [](const Expression &lhs, const Expression &rhs) { return lhs == rhs; });
}

bool StdLib::Ne(EvaluationContext &ctx) {
  return ExpressionPredicateFn(ctx, "!=", [](const Expression &lhs, const Expression &rhs) { return lhs != rhs; });
}

bool StdLib::Lt(EvaluationContext &ctx) {
  return BinaryPredicate(ctx, "<", LtT<Bool>, LtT<Number>, LtT<String>);
}

bool StdLib::Gt(EvaluationContext &ctx) {
  return BinaryPredicate(ctx, ">", GtT<Bool>, GtT<Number>, GtT<String>);
}

bool StdLib::Lte(EvaluationContext &ctx) {
  return BinaryPredicate(ctx, "<=", LteT<Bool>, LteT<Number>, LteT<String>);
}

bool StdLib::Gte(EvaluationContext &ctx) {
  return BinaryPredicate(ctx, ">=", GteT<Bool>, GteT<Number>, GteT<String>);
}

// Branching, scoping and evaluation

bool StdLib::QuoteFn(EvaluationContext &ctx) {
  ExpressionPtr quote { new Quote { std::move(ctx.Args.front()) } };
  ctx.Args.pop_front();
  ctx.Expr = std::move(quote);
  return true;
}

bool StdLib::Unquote(EvaluationContext &ctx) {
  ExpressionPtr quoteExpr { std::move(ctx.Args.front()) };
  ExpressionPtr toEvaluate;
  if (auto quote = dynamic_cast<Quote*>(quoteExpr.get()))
    toEvaluate = std::move(quote->Value);
  else
    toEvaluate = std::move(quoteExpr);

  if (ctx.Interp.EvaluatePartial(toEvaluate)) {
    ctx.Expr = std::move(toEvaluate);
    return true;
  }
  else
    return ctx.Interp.PushError(EvalError { "unquote", "Failed to evaluate expression" });
}

bool StdLib::If(EvaluationContext &ctx) {
  ExpressionPtr condExpr = std::move(ctx.Args.front());
  ctx.Args.pop_front();

  ExpressionPtr trueExpr = std::move(ctx.Args.front());
  ctx.Args.pop_front();
   
  ExpressionPtr falseExpr = std::move(ctx.Args.front());
  ctx.Args.pop_front();

  auto cond = static_cast<Bool*>(condExpr.get());
  ExpressionPtr *branchExpr = cond->Value ? &trueExpr : &falseExpr;
  if (ctx.Interp.EvaluatePartial(*branchExpr)) {
    ctx.Expr = std::move(*branchExpr);
    return true;
  }
  else
    return ctx.Interp.PushError(EvalError { "if", "Failed to evaluate branch" } );
}

// Go through all the code and harden, perform additional argument checking

bool StdLib::Let(EvaluationContext &ctx) {
  Scope scope(ctx.Interp.GetCurrentStackFrame().GetLocalSymbols());
  ExpressionPtr varsExpr = std::move(ctx.Args.front());
  ctx.Args.pop_front();

  auto vars = dynamic_cast<Sexp*>(varsExpr.get());
  if (!vars)
    return TypeError(ctx.Interp, "let", "sexp", varsExpr);

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
          if (ctx.Interp.EvaluatePartial(varValueExpr)) {
            if (!scope.IsScopedSymbol(varName->Value))
              scope.PutSymbol(varName->Value, varValueExpr);
            else
              return ctx.Interp.PushError(EvalError { "let", "Duplicate binding for " + varName->Value });
          }
          else
            return ctx.Interp.PushError(EvalError { "let", "Failed to evaluate value for " + varName->Value } );
        }
        else
          return TypeError(ctx.Interp, "let", "symbol", varNameExpr);
      }
      else
        return ctx.Interp.PushError(EvalError {
          "let",
          "Expected 2 args: (name1 value1). Got " + std::to_string(nVarArgs) + " args"
        });
    }
    else
      return TypeError(ctx.Interp, "let", "(name1 value1)", varExpr);
  }

  return Begin(ctx);
}

bool StdLib::Begin(EvaluationContext &ctx) {
  ExpressionPtr currCodeExpr;
  while (!ctx.Args.empty()) {
    currCodeExpr = std::move(ctx.Args.front());
    ctx.Args.pop_front();

    if (!ctx.Interp.EvaluatePartial(currCodeExpr))
      return ctx.Interp.PushError(EvalError { "let", "Failed to evaluate let body" });
  }

  ctx.Expr = std::move(currCodeExpr);
  return true;
}

bool StdLib::Lambda(EvaluationContext &ctx) {
  ExpressionPtr formalsExpr = std::move(ctx.Args.front());
  ctx.Args.pop_front();

  ExpressionPtr codeExpr = std::move(ctx.Args.front());
  ctx.Args.pop_front();
  
  ArgList anonFuncArgs;
  int nArgs = 0;
  if (LambdaPrepareFormals(ctx.Interp, formalsExpr, anonFuncArgs, nArgs)) {
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
    auto &locals = ctx.Interp.GetCurrentStackFrame().GetLocalSymbols();
    locals.ForEach([interpFunc](const std::string &name, ExpressionPtr &value) {
      interpFunc->Closure.emplace(name, value->Clone());
    });
    ctx.Expr = std::move(func);
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

bool StdLib::Def(EvaluationContext &ctx) {
  ExpressionPtr symbolExpr { std::move(ctx.Args.front()) };
  ctx.Args.pop_front();

  ExpressionPtr lambdaExpr { new Sexp { } };
  auto lambda = static_cast<Sexp*>(lambdaExpr.get());
  lambda->Args.push_back(ExpressionPtr { new Symbol { "lambda" } });
  lambda->Args.push_back(std::move(ctx.Args.front()));
  ctx.Args.pop_front();
  lambda->Args.push_back(std::move(ctx.Args.front()));
  ctx.Args.pop_front();

  if (ctx.Interp.EvaluatePartial(lambdaExpr)) {
    ExpressionPtr setExpr { new Sexp {} };
    auto set = static_cast<Sexp*>(setExpr.get());
    set->Args.push_back(ExpressionPtr { new Symbol { "set" } });
    set->Args.push_back(std::move(symbolExpr));
    set->Args.push_back(std::move(lambdaExpr)); 
    if (ctx.Interp.EvaluatePartial(setExpr)) {
      ctx.Expr = std::move(setExpr);
      return true;
    }
    else
      return ctx.Interp.PushError(EvalError { "def", "evaluating set expression failed" });
  }
  else
    return ctx.Interp.PushError(EvalError { "def", "evaluating lambda expression failed" });
}

bool StdLib::Apply(EvaluationContext &ctx) {
  ExpressionPtr applicationExpr { new Sexp { } };
  auto application = static_cast<Sexp*>(applicationExpr.get());
  application->Args.push_back(std::move(ctx.Args.front()));
  ctx.Args.pop_front();

  ExpressionPtr appArgsExpr { std::move(ctx.Args.front()) };
  ctx.Args.pop_front();

  if (ctx.Interp.EvaluatePartial(appArgsExpr)) {
    ExpressionPtr newArgs;
    if (auto quote = dynamic_cast<Quote*>(appArgsExpr.get())) {
      newArgs = std::move(quote->Value);
    }
    else
      newArgs = std::move(appArgsExpr);

    auto appArgsSexp = dynamic_cast<Sexp*>(newArgs.get());
    if (appArgsSexp) {
      while (!appArgsSexp->Args.empty()) {
        application->Args.push_back(std::move(appArgsSexp->Args.front()));
        appArgsSexp->Args.pop_front();
      }

      if (ctx.Interp.EvaluatePartial(applicationExpr)) {
        ctx.Expr = std::move(applicationExpr);
        return true;
      }
      else
        return ctx.Interp.PushError(EvalError { "apply", "failed to evaluate function application" });
    }
    else
      return TypeError(ctx.Interp, "apply", "list", newArgs);
  }
  else
    return ctx.Interp.PushError(EvalError { "apply", "failed to evaluate argument list" });
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
static bool StdLib::BinaryFunction(EvaluationContext &ctx, F fn, const std::string &name) {
  bool first = true;
  Number result { 0 };
  while (!ctx.Args.empty()) {
    bool ok = false;
    ExpressionPtr numExpr = TypeHelper::GetNumber(ctx.Args.front());
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
      return TypeError(ctx.Interp, name, "number", ctx.Args.front());

    first = false;
    ctx.Args.pop_front();
  }

  ctx.Expr = ExpressionPtr { new Number { result } };
  return true;
}

template <class B, class N, class S>
bool StdLib::BinaryPredicate(EvaluationContext &ctx, const std::string &name, B bFn, N nFn, S sFn) {
  auto currArg = ctx.Args.begin();
  if (currArg != ctx.Args.end()) {
    if (ctx.Interp.EvaluatePartial(*currArg)) {
      auto &type = (*currArg)->Type();

      Bool defaultValue { true };
      if (bFn && (&type == &Bool::TypeInstance))
        return PredicateHelper<Bool>(ctx, name, bFn, defaultValue);
      else if (nFn && (&type == &Number::TypeInstance))
        return PredicateHelper<Number>(ctx, name, nFn, defaultValue);
      else if (sFn && (&type == &String::TypeInstance))
        return PredicateHelper<String>(ctx, name, sFn, defaultValue);
      else
        return TypeError(ctx.Interp, name, "literal", *currArg);
    }
    else
      return ctx.Interp.PushError(EvalError { name, "Failed to evaluate arg 1" });
  }
  else
    return ctx.Interp.PushError(EvalError { name, "Expected at least one argument" });
}

template <class T, class F, class R>
bool StdLib::PredicateHelper(EvaluationContext &ctx, const std::string &name, F fn, R defaultResult) {
  R result { defaultResult };
  ExpressionPtr firstArg = std::move(ctx.Args.front());
  ctx.Args.pop_front();
  auto last = dynamic_cast<T*>(firstArg.get());
  if (last) {
    int argNum = 1;
    while (!ctx.Args.empty()) {
      if (ctx.Interp.EvaluatePartial(ctx.Args.front())) {
        auto curr = dynamic_cast<T*>(ctx.Args.front().get());
        if (curr) {
          R tmp { fn(result, *last, *curr) };
          result = tmp;
        }
        else
          return TypeError(ctx.Interp, name, T::TypeInstance.TypeName, ctx.Args.front());

        *last = *curr;
        ctx.Args.pop_front();
      }
      else
        return ctx.Interp.PushError(EvalError { name, "Failed to evaluate arg " + std::to_string(argNum) });

      ++argNum;
    }
  }
  else
    return TypeError(ctx.Interp, name, T::TypeInstance.TypeName, firstArg);

  ctx.Expr = ExpressionPtr { result.Clone() };
  return true;
}

void StdLib::RegisterBinaryFunction(SymbolTable &symbolTable, const std::string& name, SlipFunction fn) {
  symbolTable.PutSymbolFunction(name, fn, FuncDef { FuncDef::AtleastOneArg(Number::TypeInstance), FuncDef::OneArg(Number::TypeInstance) });
}

void StdLib::RegisterComparator(SymbolTable &symbolTable, const std::string& name, SlipFunction fn) {
  symbolTable.PutSymbolFunction(name, fn, FuncDef { FuncDef::AtleastOneArg(), FuncDef::OneArg(Bool::TypeInstance) });
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
