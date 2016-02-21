#include <iostream>
#include <memory>
#include <sstream>
#include <algorithm>
#include <iomanip>

#include "StdLib.h"
#include "../Interpreter.h"

#include "../NumConverter.h"

//=============================================================================

void StdLib::Load(Interpreter &interpreter) {
  // Constants

  auto &symbols = interpreter.GetDynamicSymbols();
  auto &settings = interpreter.GetSettings();

  int major = 0,
      minor = 1;
  symbols.PutSymbolString("system.version", "Slip " + std::to_string(major) + "." + std::to_string(minor));
  symbols.PutSymbolInt("system.versionNumber.major", major);
  symbols.PutSymbolInt("system.versionNumber.minor", minor);

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
  symbols.PutSymbolFunction("=", &StdLib::Set,   setDef.Clone()); 
  symbols.PutSymbolFunction("+=", &StdLib::Set,  setDef.Clone()); 
  symbols.PutSymbolFunction("-=", &StdLib::Set,  setDef.Clone()); 
  symbols.PutSymbolFunction("*=", &StdLib::Set,  setDef.Clone()); 
  symbols.PutSymbolFunction("/=", &StdLib::Set,  setDef.Clone()); 
  symbols.PutSymbolFunction("%=", &StdLib::Set,  setDef.Clone()); 
  symbols.PutSymbolFunction("<<=", &StdLib::Set, setDef.Clone()); 
  symbols.PutSymbolFunction(">>=", &StdLib::Set, setDef.Clone()); 
  symbols.PutSymbolFunction("&=", &StdLib::Set,  setDef.Clone()); 
  symbols.PutSymbolFunction("^=", &StdLib::Set,  setDef.Clone()); 
  symbols.PutSymbolFunction("|=", &StdLib::Set,  setDef.Clone()); 
  
  FuncDef incrDef { FuncDef::OneArg(Symbol::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) };
  symbols.PutSymbolFunction("++", &StdLib::Set, incrDef.Clone()); 
  symbols.PutSymbolFunction("--", &StdLib::Set, incrDef.Clone()); 

  symbols.PutSymbolFunction("unset", &StdLib::UnSet, FuncDef { FuncDef::OneArg(Symbol::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });

  // Generic

  symbols.PutSymbolFunction("+", &StdLib::Add, FuncDef { FuncDef::AtleastOneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });
  symbols.PutSymbolFunction("-", &StdLib::Sub, FuncDef { FuncDef::AtleastOneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });
  symbols.PutSymbolFunction("*", &StdLib::Mult, FuncDef { FuncDef::AtleastOneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });
  symbols.PutSymbolFunction("/", &StdLib::Div, FuncDef { FuncDef::AtleastOneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });

  FuncDef powDef { FuncDef::ManyArgs(Literal::TypeInstance, 2), FuncDef::OneArg(Literal::TypeInstance) };
  symbols.PutSymbolFunction("pow", &StdLib::Pow, powDef.Clone());
  symbols.PutSymbolFunction("**", &StdLib::Pow, powDef.Clone());
  symbols.PutSymbolFunction("abs", &StdLib::Abs, FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });
  symbols.PutSymbolFunction("max", &StdLib::Max, FuncDef { FuncDef::AtleastOneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });
  symbols.PutSymbolFunction("min", &StdLib::Min, FuncDef { FuncDef::AtleastOneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });

  // Numerical

  symbols.PutSymbolFunction("incr", &StdLib::Incr, FuncDef { FuncDef::OneArg(Int::TypeInstance), FuncDef::OneArg(Int::TypeInstance) });
  symbols.PutSymbolFunction("decr", &StdLib::Decr, FuncDef { FuncDef::OneArg(Int::TypeInstance), FuncDef::OneArg(Int::TypeInstance) });
  RegisterBinaryFunction(symbols, "%", &StdLib::Mod);

  FuncDef baseFn { FuncDef::OneArg(Int::TypeInstance), FuncDef::OneArg(String::TypeInstance) };
  symbols.PutSymbolFunction("hex", StdLib::Hex, baseFn.Clone());
  symbols.PutSymbolFunction("bin", StdLib::Bin, baseFn.Clone());
  symbols.PutSymbolFunction("dec", StdLib::Dec, baseFn.Clone());

  // Bitwise

  RegisterBinaryFunction(symbols, "<<", &StdLib::LeftShift);
  RegisterBinaryFunction(symbols, ">>", &StdLib::RightShift);
  RegisterBinaryFunction(symbols, "&", &StdLib::BitAnd);
  RegisterBinaryFunction(symbols, "|", &StdLib::BitOr);
  RegisterBinaryFunction(symbols, "^", &StdLib::BitXor);
  symbols.PutSymbolFunction("~", &StdLib::BitNot, FuncDef { FuncDef::OneArg(Int::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });

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

  // Conversion operators

  symbols.PutSymbolFunction("bool", &StdLib::BoolFunc, FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) });
  symbols.PutSymbolFunction("int", &StdLib::IntFunc, FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Int::TypeInstance) });
  symbols.PutSymbolFunction("float", &StdLib::FloatFunc, FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Int::TypeInstance) });
  symbols.PutSymbolFunction("str", &StdLib::StrFunc, FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(String::TypeInstance) });

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

bool StdLib::EvaluateListSexp(EvaluationContext &ctx) {
  ExpressionPtr listExpr { new Sexp() };
  Sexp *sexp = static_cast<Sexp*>(listExpr.get());
  ArgListHelper::CopyTo(ctx.Args, sexp->Args);
  if (ctx.Evaluate(listExpr, "list")) {
    ctx.Args.clear();
    ctx.Args.push_back(std::move(listExpr));
    return Print(ctx);
  }
  else
    return false; 
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
    out << std::setprecision(17);
    curr = std::move(ctx.Args.front());
    ctx.Args.pop_front();
    if (ctx.Evaluate(curr, argNum)) {
      out << *curr << std::endl;
      cmdInterface.WriteOutputLine(out.str());
    }
    else
      return false; 
    ++argNum;
  }

  ctx.Expr = GetNil();
  return true;
}

bool StdLib::Quit(EvaluationContext &ctx) {
  ctx.Interp.Stop();
  ctx.Expr = ExpressionPtr { new Int { 0 } };
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
          return ctx.UnknownSymbolError(sym->Value);
      }
    }
  }

  ctx.Expr = GetNil();
  ctx.Interp.GetCommandInterface().WriteOutputLine(ss.str());
  return true;
}

void BuildOpSexp(EvaluationContext &ctx, const std::string &op, ExpressionPtr &symToSetExpr) {
  ExpressionPtr opExpr { new Sexp() };
  Sexp &opSexp = static_cast<Sexp&>(*opExpr);
  opSexp.Args.push_back(ExpressionPtr { new Symbol(op) });
  opSexp.Args.push_back(symToSetExpr->Clone());
  ArgListHelper::CopyTo(ctx.Args, opSexp.Args);
  
  if (!ctx.Args.empty())
    ctx.Args.pop_front();
  ctx.Args.push_front(std::move(opExpr));
}

bool StdLib::Set(EvaluationContext &ctx) {
  ExpressionPtr symToSetExpr = std::move(ctx.Args.front());
  ctx.Args.pop_front();
  if (auto *symToSet = dynamic_cast<Symbol*>(symToSetExpr.get())) {
    std::string symToSetName = symToSet->Value;
    if (auto *thisSexp = dynamic_cast<Sexp*>(ctx.Expr.get())) {
      if (auto *thisFn = dynamic_cast<Function*>(thisSexp->Args.front().get())) {
        if (thisFn->Symbol) {
          if (auto *thisFnSym = dynamic_cast<Symbol*>(thisFn->Symbol.get())) {
            std::string setOp = thisFnSym->Value;
            if (setOp.length() > 1 && setOp.back() == '=') { 
              std::string op = setOp.substr(0, setOp.length() - 1);
              BuildOpSexp(ctx, op, symToSetExpr);
            }
            else if (setOp == "++")
              BuildOpSexp(ctx, "incr", symToSetExpr);
            else if (setOp == "--")
              BuildOpSexp(ctx, "decr", symToSetExpr);

            ExpressionPtr value = std::move(ctx.Args.front());
            ctx.Args.pop_front();
            if (ctx.Evaluate(value, "value")) {
              auto &currStackFrame = ctx.Interp.GetCurrentStackFrame();
              ExpressionPtr temp;
              if (currStackFrame.GetLocalSymbols().GetSymbol(symToSetName, temp))
                currStackFrame.PutLocalSymbol(symToSetName, value->Clone());
              else
                currStackFrame.PutDynamicSymbol(symToSetName, value->Clone());

              ctx.Expr = std::move(value);
              return true;
            }
            else
              return false; 
          }
        }
      }
    }
    return ctx.Error("Internal error reading sexp");
  }
  else
    return ctx.TypeError(Symbol::TypeInstance, symToSetExpr);
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
    return ctx.UnknownSymbolError(symName);
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
      return ctx.TypeError(Function::TypeInstance, fnExpr);
  }
  else
    return ctx.UnknownSymbolError(sym.Value);
}

bool StdLib::InfixRegister(EvaluationContext &ctx) {
  return InfixRegistrationFunction(ctx, "infix-register", false);
}

bool StdLib::InfixUnregister(EvaluationContext &ctx) {
  return InfixRegistrationFunction(ctx, "infix-unregister", true);
}

// Generic Functions

template <class I, class F>
bool StdLib::GenericNumFunc(EvaluationContext &ctx, I iFn, F fFn) {
  auto currArg = ctx.Args.begin();
  if (currArg != ctx.Args.end()) {
    if (ctx.Evaluate(*currArg, 1)) {
      auto &type = (*currArg)->Type();
      if (TypeHelper::IsConvertableToInt(type))
        return iFn(ctx);
      else if (&type == &Float::TypeInstance)
        return fFn(ctx);
      else
        return ctx.TypeError("int/float", *currArg);
    }
    else
      return false; 
  }
  else
    return ctx.ArgumentExpectedError();
}

bool StdLib::Add(EvaluationContext &ctx) {
  auto currArg = ctx.Args.begin();
  if (currArg != ctx.Args.end()) {
    if (ctx.Evaluate(*currArg, 1)) {
      auto &type = (*currArg)->Type();
      if (TypeHelper::IsConvertableToInt(type))
        return AddInt(ctx);
      else if (&type == &Float::TypeInstance)
        return AddFloat(ctx);
      else if (&type == &String::TypeInstance)
        return AddString(ctx);
      else if (&type == &Quote::TypeInstance)
        return AddList(ctx);
      else
        return ctx.TypeError("string/int/float/list", *currArg);
    }
    else
      return false; 
  }
  else
    return ctx.ArgumentExpectedError();
}

bool StdLib::Sub(EvaluationContext &ctx) {
  return GenericNumFunc(ctx, StdLib::SubInt, StdLib::SubFloat);
}

bool StdLib::Mult(EvaluationContext &ctx) {
  return GenericNumFunc(ctx, StdLib::MultInt, StdLib::MultFloat);
}

bool StdLib::Div(EvaluationContext &ctx) {
  return GenericNumFunc(ctx, StdLib::DivInt, StdLib::DivFloat);
}

bool StdLib::Pow(EvaluationContext &ctx) {
  return GenericNumFunc(ctx, StdLib::PowInt, StdLib::PowFloat);
}

bool StdLib::Abs(EvaluationContext &ctx) {
  return GenericNumFunc(ctx, StdLib::AbsInt, StdLib::AbsFloat);
}

bool StdLib::Max(EvaluationContext &ctx) {
  return GenericNumFunc(ctx, StdLib::MaxInt, StdLib::MaxFloat);
}

bool StdLib::Min(EvaluationContext &ctx) {
  return GenericNumFunc(ctx, StdLib::MinInt, StdLib::MinFloat);
}

// Int Functions

bool StdLib::Incr(EvaluationContext &ctx) {
  return UnaryFunction<Int>(ctx, [](int64_t num) { return num + 1; });
}

bool StdLib::Decr(EvaluationContext &ctx) {
  return UnaryFunction<Int>(ctx, [](int64_t num) { return num - 1; });
}

bool StdLib::AddInt(EvaluationContext &ctx) {
  return BinaryFunction<Int>(ctx, [](int64_t a, int64_t b) { return a + b; });
}

bool StdLib::SubInt(EvaluationContext &ctx) {
  return BinaryFunction<Int>(ctx, [](int64_t a, int64_t b) { return a - b; });
}

bool StdLib::MultInt(EvaluationContext &ctx) {
  return BinaryFunction<Int>(ctx, [](int64_t a, int64_t b) { return a * b; });
}

template <class T>
bool StdLib::CheckDivideByZero(EvaluationContext &ctx) {
  int argNum = 0;
  for (auto &arg : ctx.Args) {
    if (argNum) {
      ExpressionPtr numExpr = arg->Clone();
      if (numExpr) {
        auto *num = dynamic_cast<T*>(numExpr.get());
        if (num && num->Value == 0)
          return ctx.Error("Divide by zero");
      }
      else
        return ctx.TypeError(T::TypeInstance, arg);
    }
    ++argNum;
  }
  return true;
}

bool StdLib::DivInt(EvaluationContext &ctx) {
  if (CheckDivideByZero<Int>(ctx))
    return BinaryFunction<Int>(ctx, [](int64_t a, int64_t b) { return a / b; });
  return false;
}

bool StdLib::Mod(EvaluationContext &ctx) {
  if (CheckDivideByZero<Int>(ctx))
    return BinaryFunction<Int>(ctx, [](int64_t a, int64_t b) { return a % b; });
  return false;
}

bool StdLib::Hex(EvaluationContext &ctx) {
  std::stringstream ss;
  ss << "0x" << std::hex << *ctx.Args.front();
  ctx.Expr = ExpressionPtr { new String(ss.str()) };
  return true;
}

bool StdLib::Bin(EvaluationContext &ctx) {
  std::stringstream ss;
  if (auto *num = dynamic_cast<Int*>(ctx.Args.front().get())) {
    int64_t value = num->Value;
    if (value) {
      while (value) {
        if (value % 2)
          ss << "1";
        else
          ss << "0";
        value >>= 1;
      }
    }
    else
      ss << "0";
  }
  ss << "b0";

  std::string s = ss.str();
  std::reverse(begin(s), end(s));

  ctx.Expr = ExpressionPtr { new String(s) };
  return true;
}

bool StdLib::Dec(EvaluationContext &ctx) {
  std::stringstream ss;
  ss << *ctx.Args.front();
  ctx.Expr = ExpressionPtr { new String(ss.str()) };
  return true;
}

bool StdLib::PowInt(EvaluationContext &ctx) {
  return BinaryFunction<Int>(ctx, [](int64_t a, int64_t b) { return static_cast<int64_t>(std::pow(a, b)); });
}

bool StdLib::AbsInt(EvaluationContext &ctx) {
  return UnaryFunction<Int>(ctx, [](int64_t n) { return std::abs(n); });
}

bool StdLib::MaxInt(EvaluationContext &ctx) {
  return BinaryFunction<Int>(ctx, [](int64_t a, int64_t b) { return std::max(a, b); });
}

bool StdLib::MinInt(EvaluationContext &ctx) {
  return BinaryFunction<Int>(ctx, [](int64_t a, int64_t b) { return std::min(a, b); });
}

// Float Functions

bool StdLib::AddFloat(EvaluationContext &ctx) {
  return BinaryFunction<Float>(ctx, [](double a, double b) { return a + b; });
}

bool StdLib::SubFloat(EvaluationContext &ctx) {
  return BinaryFunction<Float>(ctx, [](double a, double b) { return a - b; });
}

bool StdLib::MultFloat(EvaluationContext &ctx) {
  return BinaryFunction<Float>(ctx, [](double a, double b) { return a * b; });
}

bool StdLib::DivFloat(EvaluationContext &ctx) {
  if (CheckDivideByZero<Float>(ctx))
    return BinaryFunction<Float>(ctx, [](double a, double b) { return a / b; });
  return false;
}

bool StdLib::AbsFloat(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double n) { return std::abs(n); });
}

bool StdLib::MaxFloat(EvaluationContext &ctx) {
  return BinaryFunction<Float>(ctx, [](double a, double b) { return std::max(a, b); });
}

bool StdLib::MinFloat(EvaluationContext &ctx) {
  return BinaryFunction<Float>(ctx, [](double a, double b) { return std::min(a, b); });
}

bool StdLib::PowFloat(EvaluationContext &ctx) {
  return BinaryFunction<Float>(ctx, [](double a, double b) { return std::pow(a, b); });
}

// Bitwise Functions

bool StdLib::LeftShift(EvaluationContext &ctx) {
  return BinaryFunction<Int>(ctx, [](int64_t a, int64_t b) { return a << b; });
}

bool StdLib::RightShift(EvaluationContext &ctx) {
  return BinaryFunction<Int>(ctx, [](int64_t a, int64_t b) { return a >> b; });
}

bool StdLib::BitAnd(EvaluationContext &ctx) {
  return BinaryFunction<Int>(ctx, [](int64_t a, int64_t b) { return a & b; });
}

bool StdLib::BitOr(EvaluationContext &ctx) {
  return BinaryFunction<Int>(ctx, [](int64_t a, int64_t b) { return a | b; });
}

bool StdLib::BitXor(EvaluationContext &ctx) {
  return BinaryFunction<Int>(ctx, [](int64_t a, int64_t b) { return a ^ b; });
}

bool StdLib::BitNot(EvaluationContext &ctx) {
  ExpressionPtr numExpr = TypeHelper::GetInt(ctx.Args.front());
  if (numExpr) {
    auto *num = dynamic_cast<Int*>(numExpr.get());
    if (num) {
      ctx.Expr = ExpressionPtr { new Int(~num->Value) };
      return true;
    }
  }
  return ctx.TypeError(Int::TypeInstance, ctx.Args.front());
}

// String functions

bool StdLib::AddString(EvaluationContext &ctx) {
  std::stringstream ss;
  while (!ctx.Args.empty()) {
    auto str = dynamic_cast<String*>(ctx.Args.front().get());
    if (str)
      ss << str->Value;
    else
      return ctx.TypeError(String::TypeInstance, ctx.Args.front());

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
    if (ctx.Evaluate(arg, argNum))
      list->Args.push_back(std::move(arg));
    else
      return false; 
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
      if (!ctx.EvaluateNoError(evalExpr))
        return ctx.Error("Failed to call " +  fn->ToString() + " on item " + std::to_string(i));
      resultList->Args.push_back(std::move(evalExpr));
      ++i;
    }
  }
  else 
    return ctx.TypeError("list", quote->Value);

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
        return ctx.TypeError("list", quote->Value);
    }
    else
      return ctx.TypeError("list", ctx.Args.front());

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
    return ctx.TypeError("list", quote->Value);
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
    return ctx.TypeError("list", quote->Value);
}

// Logical

bool StdLib::BinaryLogicalFunc(EvaluationContext &ctx, bool isAnd) {
  int argNum = 1;
  while (!ctx.Args.empty()) {
    ExpressionPtr currArg = std::move(ctx.Args.front());
    ctx.Args.pop_front();
    if (ctx.Evaluate(currArg, argNum)) {
      Bool *argValue = dynamic_cast<Bool*>(currArg.get());
      if (argValue) {
        bool value = argValue->Value;
        if (isAnd ? !value : value) {
          ctx.Expr = ExpressionPtr { new Bool(isAnd ? false : true) };
          return true;
        }
      }
      else
        return ctx.TypeError(Bool::TypeInstance, currArg);
    }
    else
      return false; 

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
    if (ctx.Evaluate(arg, 1)) {
      Bool *boolArg = dynamic_cast<Bool*>(arg.get());
      if (boolArg) {
        ctx.Expr = ExpressionPtr { new Bool(!boolArg->Value) };
        return true;
      }
      else
        return ctx.TypeError(Bool::TypeInstance, arg);
    }
    else
      return false; 
  }
  else
    return ctx.ArgumentExpectedError();
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
bool ExpressionPredicateFn(EvaluationContext &ctx, ExpressionPredicate fn) {
  int argNum = 0;
  if (!ctx.Args.empty()) {
    ExpressionPtr firstArg = std::move(ctx.Args.front());
    ctx.Args.pop_front();
    ++argNum;
    if (ctx.Evaluate(firstArg, argNum)) {
      ExpressionPtr prevArg = std::move(firstArg);
      bool result = false;
      while (!ctx.Args.empty()) {
        ExpressionPtr currArg = std::move(ctx.Args.front());
        ctx.Args.pop_front();
        ++argNum;
        if (ctx.Evaluate(currArg, argNum)) {
          if (!fn(*prevArg, *currArg)) {
            ctx.Expr = ExpressionPtr { new Bool(false) };
            return true;
          }
        }
        else
          return false; 

        prevArg = std::move(currArg);
      }
      ctx.Expr = ExpressionPtr { new Bool(true) };
      return true;
    }
    else
      return false; 
  }
  else
    return ctx.ArgumentExpectedError();
}

bool StdLib::Eq(EvaluationContext &ctx) {
  return ExpressionPredicateFn(ctx, [](const Expression &lhs, const Expression &rhs) { return lhs == rhs; });
}

bool StdLib::Ne(EvaluationContext &ctx) {
  return ExpressionPredicateFn(ctx, [](const Expression &lhs, const Expression &rhs) { return lhs != rhs; });
}

bool StdLib::Lt(EvaluationContext &ctx) {
  return BinaryPredicate(ctx, LtT<Bool>, LtT<Int>, LtT<Float>, LtT<String>);
}

bool StdLib::Gt(EvaluationContext &ctx) {
  return BinaryPredicate(ctx, GtT<Bool>, GtT<Int>, GtT<Float>, GtT<String>);
}

bool StdLib::Lte(EvaluationContext &ctx) {
  return BinaryPredicate(ctx, LteT<Bool>, LteT<Int>, LteT<Float>, LteT<String>);
}

bool StdLib::Gte(EvaluationContext &ctx) {
  return BinaryPredicate(ctx, GteT<Bool>, GteT<Int>, GteT<Float>, GteT<String>);
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

  if (ctx.Evaluate(toEvaluate, "expression")) {
    ctx.Expr = std::move(toEvaluate);
    return true;
  }
  else
    return false; 
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
  if (ctx.Evaluate(*branchExpr, "branch")) {
    ctx.Expr = std::move(*branchExpr);
    return true;
  }
  else
    return false; 
}

// Go through all the code and harden, perform additional argument checking

bool StdLib::Let(EvaluationContext &ctx) {
  Scope scope(ctx.Interp.GetCurrentStackFrame().GetLocalSymbols());
  ExpressionPtr varsExpr = std::move(ctx.Args.front());
  ctx.Args.pop_front();

  auto vars = dynamic_cast<Sexp*>(varsExpr.get());
  if (!vars)
    return ctx.TypeError(Sexp::TypeInstance, varsExpr);

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
          if (ctx.Evaluate(varValueExpr, varName->Value)) {
            if (!scope.IsScopedSymbol(varName->Value))
              scope.PutSymbol(varName->Value, varValueExpr);
            else
              return ctx.Error("Duplicate binding for " + varName->Value);
          }
          else
            return false; 
        }
        else
          return ctx.TypeError(Symbol::TypeInstance, varNameExpr);
      }
      else
        return ctx.Error("Expected 2 args: (name1 value1). Got " + std::to_string(nVarArgs) + " args");
    }
    else
      return ctx.TypeError("(name1 value1)", varExpr);
  }

  return Begin(ctx);
}

bool StdLib::Begin(EvaluationContext &ctx) {
  ExpressionPtr currCodeExpr;
  while (!ctx.Args.empty()) {
    currCodeExpr = std::move(ctx.Args.front());
    ctx.Args.pop_front();

    if (!ctx.Evaluate(currCodeExpr, "body"))
      return false;
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
  if (LambdaPrepareFormals(ctx, formalsExpr, anonFuncArgs, nArgs)) {
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

bool StdLib::LambdaPrepareFormals(EvaluationContext &ctx, ExpressionPtr &formalsExpr, ArgList &anonFuncArgs, int &nArgs) {
  auto formalsList = dynamic_cast<Sexp*>(formalsExpr.get());
  if (formalsList) {
    for (auto &formal : formalsList->Args) {
      auto formalSym = dynamic_cast<Symbol*>(formal.get());
      if (formalSym) {
        anonFuncArgs.push_back(formalSym->Clone());
      }
      else
        return ctx.TypeError(Symbol::TypeInstance, formal);
      ++nArgs;
    }
  }
  else
    return ctx.TypeError("formals list", formalsExpr);
  
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

  if (ctx.Evaluate(lambdaExpr, "lambdaExpr")) {
    ExpressionPtr setExpr { new Sexp {} };
    auto set = static_cast<Sexp*>(setExpr.get());
    set->Args.push_back(ExpressionPtr { new Symbol { "set" } });
    set->Args.push_back(std::move(symbolExpr));
    set->Args.push_back(std::move(lambdaExpr)); 
    if (ctx.Evaluate(setExpr, "setExpr")) {
      ctx.Expr = std::move(setExpr);
      return true;
    }
  }
  return false; 
}

bool StdLib::Apply(EvaluationContext &ctx) {
  ExpressionPtr applicationExpr { new Sexp { } };
  auto application = static_cast<Sexp*>(applicationExpr.get());
  application->Args.push_back(std::move(ctx.Args.front()));
  ctx.Args.pop_front();

  ExpressionPtr appArgsExpr { std::move(ctx.Args.front()) };
  ctx.Args.pop_front();

  if (ctx.Evaluate(appArgsExpr, "argsList")) {
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

      if (ctx.Evaluate(applicationExpr, "functionApplication")) {
        ctx.Expr = std::move(applicationExpr);
        return true;
      }
      else
        return false; 
    }
    else
      return ctx.TypeError("list", newArgs);
  }
  else
    return false; 
}
// Conversion operators

bool StdLib::BoolFunc(EvaluationContext &ctx) {
  bool value = false;
  if (auto &expr = ctx.Args.front()) {
    if (ctx.Evaluate(expr, 1)) {
      if (auto *boolValue = dynamic_cast<Bool*>(expr.get()))
        value = boolValue->Value;
      else if (auto *intValue = dynamic_cast<Int*>(expr.get()))
        value = intValue->Value != 0;
      else if (auto *floatValue = dynamic_cast<Float*>(expr.get()))
        value = floatValue->Value != 0.0;
      else if (auto *strValue = dynamic_cast<String*>(expr.get()))
        value = strValue->Value != "";
      ctx.Expr = ExpressionPtr { new Bool(value) };
      return true;
    }
    else
      return false;
  }
  return false;
}

bool StdLib::IntFunc(EvaluationContext &ctx) {
  int64_t value = 0;
  if (auto &expr = ctx.Args.front()) {
    if (ctx.Evaluate(expr, 1)) {
      if (auto *boolValue = dynamic_cast<Bool*>(expr.get()))
        value = boolValue->Value;
      else if (auto *intValue = dynamic_cast<Int*>(expr.get()))
        value = intValue->Value;
      else if (auto *floatValue = dynamic_cast<Float*>(expr.get()))
        value = std::llround(floatValue->Value);
      else if (auto *strValue = dynamic_cast<String*>(expr.get())) {
        if (!strValue->Value.empty()) {
          try {
            NumConverter::Convert(strValue->Value, value);
          }
          catch (...) {
          }
        }
      }
      ctx.Expr = ExpressionPtr { new Int(value) };
      return true;
    }
    else
      return false;
  }
  return false;
}

bool StdLib::FloatFunc(EvaluationContext &ctx) {
  double value = 0;
  if (auto &expr = ctx.Args.front()) {
    if (ctx.Evaluate(expr, 1)) {
      if (auto *boolValue = dynamic_cast<Bool*>(expr.get()))
        value = boolValue->Value ? 1 : 0;
      else if (auto *intValue = dynamic_cast<Int*>(expr.get()))
        value = static_cast<double>(intValue->Value);
      else if (auto *floatValue = dynamic_cast<Float*>(expr.get()))
        value = floatValue->Value;
      else if (auto *strValue = dynamic_cast<String*>(expr.get())) {
        if (!strValue->Value.empty()) {
          try {
            NumConverter::Convert(strValue->Value, value);
          }
          catch (...) {
          }
        }
      }
      ctx.Expr = ExpressionPtr { new Float(value) };
      return true;
    }
    else
      return false;
  }
  return false;
}

bool StdLib::StrFunc(EvaluationContext &ctx) {
  std::string value = "";
  if (auto &expr = ctx.Args.front()) {
    if (ctx.Evaluate(expr, 1)) {
      ctx.Expr = ExpressionPtr { new String(expr->ToString()) };
      return true;
    }
    else
      return false;
  }
  return false;
}

// Helpers

template <class T, class F>
bool StdLib::UnaryFunction(EvaluationContext &ctx, F fn) {
  ExpressionPtr numExpr = ctx.Args.front()->Clone();
  auto num = dynamic_cast<T*>(numExpr.get());
  if (num) {
    ctx.Expr = ExpressionPtr { new T { fn(num->Value) } };
    return true;
  }
  return ctx.TypeError(T::TypeInstance, ctx.Args.front());
}

template<class T, class F>
bool StdLib::BinaryFunction(EvaluationContext &ctx, F fn) {
  bool first = true;
  T result { 0 };
  while (!ctx.Args.empty()) {
    bool ok = false;
    ExpressionPtr numExpr = ctx.Args.front()->Clone();
    if (numExpr) {
      auto num = dynamic_cast<T*>(numExpr.get());
      if (num) {
        if (first)
          result.Value = num->Value;
        else
          result.Value = fn(result.Value, num->Value);
        ok = true;
      }
    }
    if (!ok)
      return ctx.TypeError(T::TypeInstance, ctx.Args.front());

    first = false;
    ctx.Args.pop_front();
  }

  ctx.Expr = ExpressionPtr { new T { result } };
  return true;
}

template <class B, class I, class F, class S>
bool StdLib::BinaryPredicate(EvaluationContext &ctx, B bFn, I iFn, F fFn, S sFn) {
  auto currArg = ctx.Args.begin();
  if (currArg != ctx.Args.end()) {
    if (ctx.Evaluate(*currArg, 1)) {
      auto &type = (*currArg)->Type();

      Bool defaultValue { true };
      if (bFn && (&type == &Bool::TypeInstance))
        return PredicateHelper<Bool>(ctx, bFn, defaultValue);
      else if (iFn && (&type == &Int::TypeInstance))
        return PredicateHelper<Int>(ctx, iFn, defaultValue);
      else if (fFn && (&type == &Float::TypeInstance))
        return PredicateHelper<Float>(ctx, fFn, defaultValue);
      else if (sFn && (&type == &String::TypeInstance))
        return PredicateHelper<String>(ctx, sFn, defaultValue);
      else
        return ctx.TypeError(Literal::TypeInstance, *currArg);
    }
    else
      return false; 
  }
  else
    return ctx.ArgumentExpectedError();
}

template <class T, class F, class R>
bool StdLib::PredicateHelper(EvaluationContext &ctx, F fn, R defaultResult) {
  R result { defaultResult };
  ExpressionPtr firstArg = std::move(ctx.Args.front());
  ctx.Args.pop_front();
  auto last = dynamic_cast<T*>(firstArg.get());
  if (last) {
    int argNum = 1;
    while (!ctx.Args.empty()) {
      if (ctx.Evaluate(ctx.Args.front(), argNum)) {
        auto curr = dynamic_cast<T*>(ctx.Args.front().get());
        if (curr) {
          R tmp { fn(result, *last, *curr) };
          result = tmp;
        }
        else
          return ctx.TypeError(T::TypeInstance, ctx.Args.front());

        *last = *curr;
        ctx.Args.pop_front();
      }
      else
        return false; 

      ++argNum;
    }
  }
  else
    return ctx.TypeError(T::TypeInstance, firstArg);

  ctx.Expr = ExpressionPtr { result.Clone() };
  return true;
}

void StdLib::RegisterBinaryFunction(SymbolTable &symbolTable, const std::string& name, SlipFunction fn) {
  symbolTable.PutSymbolFunction(name, fn, FuncDef { FuncDef::AtleastOneArg(Int::TypeInstance), FuncDef::OneArg(Int::TypeInstance) });
}

void StdLib::RegisterComparator(SymbolTable &symbolTable, const std::string& name, SlipFunction fn) {
  symbolTable.PutSymbolFunction(name, fn, FuncDef { FuncDef::AtleastOneArg(), FuncDef::OneArg(Bool::TypeInstance) });
}

ExpressionPtr StdLib::GetNil() {
  return ExpressionPtr { new Quote { ExpressionPtr { new Sexp {} } } };
}
