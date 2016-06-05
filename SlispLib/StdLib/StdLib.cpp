#include <iostream>
#include <memory>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <iomanip>
#include <cctype>

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
  symbols.PutSymbolStr("system.version", "Slip " + std::to_string(major) + "." + std::to_string(minor));
  symbols.PutSymbolInt("system.versionNumber.major", major);
  symbols.PutSymbolInt("system.versionNumber.minor", minor);

  symbols.PutSymbolFloat("PI", 3.14159265358979323846);
  symbols.PutSymbolFloat("E", 2.71828182845904523536);

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

  FuncDef setDef { FuncDef::Args({&Symbol::TypeInstance, &Sexp::TypeInstance}), FuncDef::OneArg(Literal::TypeInstance) };  
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

  symbols.PutSymbolFunction("length", &StdLib::Length, FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Int::TypeInstance) });
  symbols.PutSymbolFunction("+", &StdLib::Add, FuncDef { FuncDef::AtleastOneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });
  symbols.PutSymbolFunction("empty?", &StdLib::EmptyQ, FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) });
  symbols.PutSymbolFunction("-", &StdLib::Sub, FuncDef { FuncDef::AtleastOneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });
  symbols.PutSymbolFunction("*", &StdLib::Mult, FuncDef { FuncDef::AtleastOneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });
  symbols.PutSymbolFunction("/", &StdLib::Div, FuncDef { FuncDef::AtleastOneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });

  FuncDef powDef { FuncDef::ManyArgs(Literal::TypeInstance, 2), FuncDef::OneArg(Literal::TypeInstance) };
  symbols.PutSymbolFunction("pow", &StdLib::Pow, powDef.Clone());
  symbols.PutSymbolFunction("**", &StdLib::Pow, powDef.Clone());
  symbols.PutSymbolFunction("abs", &StdLib::Abs, FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });
  symbols.PutSymbolFunction("max", &StdLib::Max, FuncDef { FuncDef::AtleastOneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });
  symbols.PutSymbolFunction("min", &StdLib::Min, FuncDef { FuncDef::AtleastOneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });

  FuncDef foreachDef { FuncDef::ManyArgs(Sexp::TypeInstance, 2, ArgDef::ANY_ARGS), FuncDef::OneArg(Literal::TypeInstance) };
  symbols.PutSymbolFunction("foreach", &StdLib::Foreach, foreachDef.Clone());
  symbols.PutSymbolFunction("for", &StdLib::Foreach, foreachDef.Clone());

  symbols.PutSymbolFunction("reverse", &StdLib::Reverse, FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });

  FuncDef headDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) };
  symbols.PutSymbolFunction("head", &StdLib::Head, headDef.Clone());
  symbols.PutSymbolFunction("car", &StdLib::Head, headDef.Clone());
  symbols.PutSymbolFunction("first", &StdLib::Head, headDef.Clone());
  symbols.PutSymbolFunction("front", &StdLib::Head, headDef.Clone());

  symbols.PutSymbolFunction("tail", &StdLib::Tail, headDef.Clone());
  symbols.PutSymbolFunction("cdr", &StdLib::Tail, headDef.Clone());
  symbols.PutSymbolFunction("rest", &StdLib::Tail, headDef.Clone());

  symbols.PutSymbolFunction("last", &StdLib::Last, headDef.Clone());
  symbols.PutSymbolFunction("back", &StdLib::Last, headDef.Clone());

  FuncDef atDef { FuncDef::Args({&Literal::TypeInstance, &Int::TypeInstance}), FuncDef::OneArg(Literal::TypeInstance)};
  symbols.PutSymbolFunction("at", &StdLib::At, atDef.Clone());
  symbols.PutSymbolFunction("nth", &StdLib::At, atDef.Clone());
  //TODO: Lists
  //TODO: ("abc" 1) => "b" ??????????
  //TODO: "abc"[1] => b
  //TODO: ([] "abc" 1) => b ????
  //TODO: ([1] "abc") => b ????


  // Float

  symbols.PutSymbolFunction("exp", &StdLib::Exp, FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) });
  symbols.PutSymbolFunction("log", &StdLib::Log, FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) });
  symbols.PutSymbolFunction("sqrt", &StdLib::Sqrt, FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) });
  symbols.PutSymbolFunction("ceil", &StdLib::Ceil, FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) });
  symbols.PutSymbolFunction("floor", &StdLib::Floor, FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) });
  symbols.PutSymbolFunction("round", &StdLib::Round, FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) });
  symbols.PutSymbolFunction("cos", &StdLib::Cos, FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) });
  symbols.PutSymbolFunction("sin", &StdLib::Sin, FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) });
  symbols.PutSymbolFunction("tan", &StdLib::Tan, FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) });
  symbols.PutSymbolFunction("acos", &StdLib::ACos, FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) });
  symbols.PutSymbolFunction("asin", &StdLib::ASin, FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) });
  symbols.PutSymbolFunction("atan", &StdLib::ATan, FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) });
  symbols.PutSymbolFunction("atan2", &StdLib::ATan2, FuncDef { FuncDef::ManyArgs(Float::TypeInstance, 2), FuncDef::OneArg(Float::TypeInstance) });
  symbols.PutSymbolFunction("cosh", &StdLib::Cosh, FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) });
  symbols.PutSymbolFunction("sinh", &StdLib::Sinh, FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) });
  symbols.PutSymbolFunction("tanh", &StdLib::Tanh, FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) });
  symbols.PutSymbolFunction("acosh", &StdLib::ACosh, FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) });
  symbols.PutSymbolFunction("asinh", &StdLib::ASinh, FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) });
  symbols.PutSymbolFunction("atanh", &StdLib::ATanh, FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) });

  // Numerical

  symbols.PutSymbolFunction("incr", &StdLib::Incr, FuncDef { FuncDef::OneArg(Int::TypeInstance), FuncDef::OneArg(Int::TypeInstance) });
  symbols.PutSymbolFunction("decr", &StdLib::Decr, FuncDef { FuncDef::OneArg(Int::TypeInstance), FuncDef::OneArg(Int::TypeInstance) });
  RegisterBinaryFunction(symbols, "%", &StdLib::Mod);

  FuncDef baseFn { FuncDef::OneArg(Int::TypeInstance), FuncDef::OneArg(Str::TypeInstance) };
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

  // Str 

  FuncDef trimDef { FuncDef::OneArg(Str::TypeInstance), FuncDef::OneArg(Str::TypeInstance) };
  symbols.PutSymbolFunction("trim", StdLib::Trim, trimDef.Clone());
  symbols.PutSymbolFunction("upper", StdLib::Upper, trimDef.Clone());
  symbols.PutSymbolFunction("lower", StdLib::Lower, trimDef.Clone());

  symbols.PutSymbolFunction("substr", StdLib::SubStr, FuncDef { FuncDef::ManyArgs(Literal::TypeInstance, 2, 3), FuncDef::OneArg(Str::TypeInstance) });

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


  symbols.PutSymbolFunction("cons", &StdLib::Cons, FuncDef { FuncDef::ManyArgs(Literal::TypeInstance, 2), FuncDef::OneArg(Quote::TypeInstance) });
  symbols.PutSymbolFunction("range", &StdLib::Range, FuncDef { FuncDef::ManyArgs(Int::TypeInstance, 2, 3), FuncDef::OneArg(Quote::TypeInstance) });
  symbols.PutSymbolFunction("..", &StdLib::Range, FuncDef { FuncDef::ManyArgs(Int::TypeInstance, 2), FuncDef::OneArg(Quote::TypeInstance) });

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
  symbols.PutSymbolFunction("'", &StdLib::QuoteFn, FuncDef { FuncDef::OneArg(Sexp::TypeInstance), FuncDef::OneArg(Quote::TypeInstance) });
  symbols.PutSymbolFunction("unquote", &StdLib::Unquote, FuncDef { FuncDef::OneArg(Sexp::TypeInstance), FuncDef::OneArg(Quote::TypeInstance) });

  symbols.PutSymbolFunction("if", &StdLib::If, FuncDef { 
    FuncDef::Args({&Bool::TypeInstance, &Sexp::TypeInstance, &Sexp::TypeInstance}),
    FuncDef::OneArg(Literal::TypeInstance)
  });

  symbols.PutSymbolFunction("cond", &StdLib::Cond, FuncDef { FuncDef::ManyArgs(Sexp::TypeInstance, 1, ArgDef::ANY_ARGS), FuncDef::OneArg(Literal::TypeInstance) });
  symbols.PutSymbolFunction("switch", &StdLib::Switch, FuncDef { FuncDef::ManyArgs(Sexp::TypeInstance, 3, ArgDef::ANY_ARGS), FuncDef::OneArg(Literal::TypeInstance) });
  symbols.PutSymbolFunction("while", &StdLib::While, FuncDef { FuncDef::ManyArgs(Sexp::TypeInstance, 2, ArgDef::ANY_ARGS), FuncDef::OneArg(Literal::TypeInstance) });
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

  symbols.PutSymbolFunction("type", &StdLib::TypeFunc, FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Function::TypeInstance) });

  symbols.PutSymbolFunction("atom?", &StdLib::TypeQFunc, FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) });
  symbols.PutSymbolFunction("bool?", &StdLib::TypeQFunc, FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) });
  symbols.PutSymbolFunction("int?", &StdLib::TypeQFunc, FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) });
  symbols.PutSymbolFunction("float?", &StdLib::TypeQFunc, FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) });
  symbols.PutSymbolFunction("str?", &StdLib::TypeQFunc, FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) });
  symbols.PutSymbolFunction("list?", &StdLib::TypeQFunc, FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) });
  symbols.PutSymbolFunction("fn?", &StdLib::TypeQFunc, FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) });

  symbols.PutSymbolFunction("bool", &StdLib::BoolFunc, FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) });
  symbols.PutSymbolFunction("int", &StdLib::IntFunc, FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Int::TypeInstance) });
  symbols.PutSymbolFunction("float", &StdLib::FloatFunc, FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Float::TypeInstance) });
  symbols.PutSymbolFunction("str", &StdLib::StrFunc, FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Str::TypeInstance) });

  // Register infix operators by precedence (using C++ rules, where appropriate)
  settings.RegisterInfixSymbol("..");

  settings.RegisterInfixSymbol("++");
  settings.RegisterInfixSymbol("--");

  settings.RegisterInfixSymbol("not");
  settings.RegisterInfixSymbol("!");
  settings.RegisterInfixSymbol("~");

  settings.RegisterInfixSymbol("**");
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
  ctx.Expr = GetNil();
  return true;
}

bool StdLib::Help(EvaluationContext &ctx) {
  std::string defaultSexp = ctx.Interp.GetSettings().GetDefaultSexp();
  std::stringstream ss;
  Interpreter::SymbolFunctor functor = [&ss, &defaultSexp](const std::string &symbolName, ExpressionPtr &expr) {
    if (symbolName != defaultSexp) {
      if (auto fn = TypeHelper::GetValue<Function>(expr))
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

      if (auto sym = TypeHelper::GetValue<Symbol>(currArg)) {
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
  if (auto symToSet = ctx.GetRequiredValue<Symbol>(symToSetExpr)) { 
    std::string symToSetName = symToSet->Value;
    const std::string setOp = ctx.GetThisFunctionName();
    if (!setOp.empty()) {
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
        ctx.Expr = value->Clone();
        if (currStackFrame.GetLocalSymbols().GetSymbol(symToSetName, temp))
          currStackFrame.PutLocalSymbol(symToSetName, std::move(value));
        else
          currStackFrame.PutDynamicSymbol(symToSetName, std::move(value));
        return true;
      }
    }
    return false;
  }
  else
    return false;
}

bool StdLib::UnSet(EvaluationContext &ctx) {
  ExpressionPtr sym = std::move(ctx.Args.front());
  ctx.Args.pop_front();

  if (auto symE = ctx.GetRequiredValue<Symbol>(sym)) {
    std::string symName = symE->Value;

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
  else
    return false;
}

bool StdLib::InfixRegistrationFunction(EvaluationContext &ctx, const std::string &name, bool unregister) {
  auto sym = static_cast<Symbol&>(*ctx.Args.front());
  ExpressionPtr fnExpr;
  if (ctx.GetSymbol(sym.Value, fnExpr)) {
    if (auto fn = ctx.GetRequiredValue<Function>(fnExpr)) { 
      auto &settings = ctx.Interp.GetSettings();
      if (unregister)
        settings.UnregisterInfixSymbol(sym.Value);
      else
        settings.RegisterInfixSymbol(sym.Value);
      ctx.Expr = GetNil(); 
      return true;
    }
    else
      return false;
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

template<class BeginIt, class EndIt>
bool ReverseGeneric(EvaluationContext &ctx, ExpressionPtr &arg, BeginIt beginIt, EndIt endIt) {
  std::reverse(beginIt, endIt);
  ctx.Expr = std::move(arg);
  return true;
}

bool StdLib::Reverse(EvaluationContext &ctx) {
  ExpressionPtr arg = std::move(ctx.Args.front());
  ctx.Args.pop_front();
  if (auto str = TypeHelper::GetValue<Str>(arg))
    return ReverseGeneric(ctx, arg, begin(str->Value), end(str->Value));
  else if (auto list = ctx.GetRequiredListValue(arg))
    return ReverseGeneric(ctx, arg, begin(list->Args), end(list->Args));
  else
    return false;
}

template <class I, class F>
bool StdLib::GenericNumFunc(EvaluationContext &ctx, I iFn, F fFn) {
  auto currArg = ctx.Args.begin();
  if (currArg != ctx.Args.end()) {
    if (ctx.Evaluate(*currArg, 1)) {
      if (TypeHelper::IsA<Int>(*currArg))
        return iFn(ctx);
      else if (TypeHelper::IsA<Float>(*currArg))
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

bool StdLib::At(EvaluationContext &ctx) {
  ExpressionPtr itArg = std::move(ctx.Args.front());
  ctx.Args.pop_front();
  ExpressionPtr idxArg = std::move(ctx.Args.front());
  ctx.Args.pop_front();

  if (auto *idxInt = ctx.GetRequiredValue<Int>(idxArg)) {
    int64_t idx = idxInt->Value; 
    if (auto *str = dynamic_cast<Str*>(itArg.get())) {
      if (idx < 0)
        idx += str->Value.length();
      try {
        ctx.Expr = ExpressionPtr { new Str(std::string(1, str->Value.at(idx))) };
        return true;
      }
      catch (std::out_of_range) {
        return ctx.Error("index " + std::to_string(idx) + " is out of bounds");
      }
    }
    else if (auto *iterable = dynamic_cast<IIterable*>(itArg.get())) {
      if (IteratorPtr iterator = iterable->GetIterator()) {
        if (idx < 0) {
          int64_t length = iterator->GetLength();
          if (length == IIterator::LENGTH_UNKNOWN)
            return ctx.Error("Negative indexes require iterator to support GetLength");
          idx += length;
        }
        int64_t currIdx = 0;
        bool more = false;
        do {
          ExpressionPtr& curr = iterator->Next();
          more = curr.operator bool();
          if (more) {
            if (currIdx == idx) {
              ctx.Expr = curr->Clone();
              return true;
            }
          }
          ++currIdx;
        } while (more);
        return ctx.Error("index " + std::to_string(idx) + " is out of bounds");
      }
    }
    return ctx.TypeError("iterable", itArg);
  }
  else
    return false;
}

bool StdLib::Add(EvaluationContext &ctx) {
  auto currArg = ctx.Args.begin();
  if (currArg != ctx.Args.end()) {
    if (ctx.Evaluate(*currArg, 1)) {
      auto &type = (*currArg)->Type();
      if (TypeHelper::IsA<Int>(type))
        return AddInt(ctx);
      else if (TypeHelper::IsA<Float>(type))
        return AddFloat(ctx);
      else if (TypeHelper::IsA<Str>(type))
        return AddStr(ctx);
      else if (TypeHelper::IsA<Quote>(type))
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

bool StdLib::Length(EvaluationContext &ctx) {
  return SequenceFn(ctx, 
    [](std::string &value) { return new Int(value.size()); },
    [](ArgList &value)     { return new Int(value.size()); }
  );
}

bool StdLib::EmptyQ(EvaluationContext &ctx) {
  return SequenceFn(ctx, 
    [](std::string &value) { return new Bool(value.empty()); },
    [](ArgList &value)     { return new Bool(value.empty()); }
  );
}

template <class S, class L>
bool StdLib::SequenceFn(EvaluationContext &ctx, S strFn, L listFn) {
  ExpressionPtr arg = std::move(ctx.Args.front());
  ctx.Args.clear();
  if (ctx.Evaluate(arg, "1")) {
    if (auto str = TypeHelper::GetValue<Str>(arg)) {
      ctx.Expr = ExpressionPtr { strFn(str->Value) };
      return true;
    }
    else if (auto quote = TypeHelper::GetValue<Quote>(arg)) {
      if (IsQuoteAList(ctx, *quote)) {
        if (auto listSexp = TypeHelper::GetValue<Sexp>(quote->Value)) {
          ctx.Expr = ExpressionPtr { listFn(listSexp->Args) };
          return true;
        }
      }
    }
    return ctx.TypeError("str/list", arg);
  }
  else
    return false;
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

// (foreach e lst (print e))
// (foreach e in lst (print e))
// (foreach lst print)
bool StdLib::Foreach(EvaluationContext &ctx) {
  auto firstArg = std::move(ctx.Args.front());
  ctx.Args.pop_front();
  auto secondArg = std::move(ctx.Args.front());
  ctx.Args.pop_front();

  if (auto firstSym = ctx.GetRequiredValue<Symbol>(firstArg)) {
    Symbol* currElementSym = nullptr; 
    ExpressionPtr iterableValueOrSym;
    Function *fn = nullptr;
    auto nRemainingArgs = ctx.Args.size();
    if (nRemainingArgs == 0) {
      iterableValueOrSym = std::move(firstArg);
      if (ctx.Evaluate(secondArg, "fn")) {
        fn = ctx.GetRequiredValue<Function>(secondArg);
        if (!fn)
          return false;
      }
      else
        return false;
    }
    else {
      if (nRemainingArgs > 1) {
        if (auto optionalInSym = TypeHelper::GetValue<Symbol>(secondArg)) {
          if (optionalInSym->Value == "in" || optionalInSym->Value == ":") {
            secondArg = std::move(ctx.Args.front());
            ctx.Args.pop_front();
          }
        }
      }
      iterableValueOrSym = std::move(secondArg);
      currElementSym = firstSym;
    }

    Expression *iterableArg = nullptr;
    if (auto iterableSym = TypeHelper::GetValue<Symbol>(iterableValueOrSym)) {
      if (!ctx.GetSymbol(iterableSym->Value, iterableArg))
        return ctx.UnknownSymbolError(iterableSym->Value);
    }
    else if (auto sexp = TypeHelper::GetValue<Sexp>(iterableValueOrSym)) {
      if (ctx.Evaluate(iterableValueOrSym, "iterable expression")) 
        iterableArg = iterableValueOrSym.get();
      else
        return false;
    }
    else
      iterableArg = iterableValueOrSym.get();

    return ForeachIterate(ctx, iterableArg, currElementSym, fn);
  }
  else
    return false;
}

bool StdLib::ForeachIterate(EvaluationContext &ctx, Expression *iterableArg, Symbol *currElementSym, Function *fn) {
  if (auto *iterable = dynamic_cast<IIterable*>(iterableArg)) {
    if (IteratorPtr iterator = iterable->GetIterator()) {
      ArgList bodyCopy;
      ArgListHelper::CopyTo(ctx.Args, bodyCopy);
      bool more = false;
      do {
        ExpressionPtr& curr = iterator->Next();
        more = curr.operator bool();
        if (more) {
          if (currElementSym) {
            Scope scope(ctx.Interp.GetCurrentStackFrame().GetLocalSymbols());
            scope.PutSymbol(currElementSym->Value, ExpressionPtr { new Ref(curr) }); 
            ctx.Args.clear();
            ArgListHelper::CopyTo(bodyCopy, ctx.Args);
            if (!Begin(ctx))
              return false;
          }
          else {
            ExpressionPtr fnEvalExpr { new Sexp };
            Sexp &fnEvalSexp = static_cast<Sexp&>(*fnEvalExpr);
            fnEvalSexp.Args.push_back(fn->Clone());
            fnEvalSexp.Args.push_back(ExpressionPtr { new Ref(curr) });
            if (ctx.Evaluate(fnEvalExpr, "function evaluation"))
              ctx.Expr = std::move(fnEvalExpr);
            else
              return false;
          }
        }
      } while (more);
      return true;
    }
  }
  return ctx.Error("argument is not iterable");
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
      if (auto num = ctx.GetRequiredValue<T>(arg)) {
        if (num->Value == 0)
          return ctx.Error("Divide by zero");
      }
      else
        return false;
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
  ctx.Expr = ExpressionPtr { new Str(ss.str()) };
  return true;
}

bool StdLib::Bin(EvaluationContext &ctx) {
  std::stringstream ss;
  if (auto num = TypeHelper::GetValue<Int>(ctx.Args.front())) {
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

  ctx.Expr = ExpressionPtr { new Str(s) };
  return true;
}

bool StdLib::Dec(EvaluationContext &ctx) {
  std::stringstream ss;
  ss << *ctx.Args.front();
  ctx.Expr = ExpressionPtr { new Str(ss.str()) };
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

bool StdLib::Exp(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return std::exp(a); });
}

bool StdLib::Log(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return std::log(a); });
}

bool StdLib::Sqrt(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return std::sqrt(a); });
}

bool StdLib::Ceil(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return std::ceil(a); });
}

bool StdLib::Floor(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return std::floor(a); });
}

bool StdLib::Round(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return std::round(a); });
}

bool StdLib::Cos(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return std::cos(a); });
}

bool StdLib::Sin(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return std::sin(a); });
}

bool StdLib::Tan(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return std::tan(a); });
}

bool StdLib::ACos(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return std::acos(a); });
}

bool StdLib::ASin(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return std::asin(a); });
}

bool StdLib::ATan(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return std::atan(a); });
}

bool StdLib::ATan2(EvaluationContext &ctx) {
  return BinaryFunction<Float>(ctx, [](double a, double b) { return std::atan2(a, b); });
}

bool StdLib::Cosh(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return std::cosh(a); });
}

bool StdLib::Sinh(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return std::sinh(a); });
}

bool StdLib::Tanh(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return std::tanh(a); });
}

bool StdLib::ACosh(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return std::acosh(a); });
}

bool StdLib::ASinh(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return std::asinh(a); });
}

bool StdLib::ATanh(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return std::atanh(a); });
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
  if (auto num = ctx.GetRequiredValue<Int>(ctx.Args.front())) {
    ctx.Expr = ExpressionPtr { new Int(~num->Value) };
    return true;
  }
  return false;
}

// Str functions

bool StdLib::Trim(EvaluationContext &ctx) {
  ExpressionPtr firstArg = std::move(ctx.Args.front());
  ctx.Args.pop_front();
  if (auto str = ctx.GetRequiredValue<Str>(firstArg)) {
    ExpressionPtr result { new Str() };
    std::string &resultValue = static_cast<Str*>(result.get())->Value;
    std::string &val = str->Value;
    size_t firstNonSpace = val.find_first_not_of(' ');
    size_t lastNonSpace = val.find_last_not_of(' ');

    if (firstNonSpace != std::string::npos) 
      resultValue.assign(val.begin() + firstNonSpace, val.begin() + lastNonSpace + 1);

    ctx.Expr = std::move(result);
    return true;
  }
  else
    return false;
}


template<class F>
bool CharTransform(EvaluationContext &ctx, F fn) {
  if (auto str = ctx.GetRequiredValue<Str>(ctx.Args.front())) {
    std::for_each(str->Value.begin(), str->Value.end(), [&fn](char &ch) { ch = fn(ch); });
    ctx.Expr = str->Clone();
    return true;
  }
  else
    return false;
}

bool StdLib::Upper(EvaluationContext &ctx) {
  return CharTransform(ctx, std::toupper);
}

bool StdLib::Lower(EvaluationContext &ctx) {
  return CharTransform(ctx, std::tolower);
}

bool StdLib::SubStr(EvaluationContext &ctx) {
  auto strArg = std::move(ctx.Args.front());
  ctx.Args.pop_front();
  if (auto str = ctx.GetRequiredValue<Str>(strArg)) {
    auto startArg = std::move(ctx.Args.front());
    ctx.Args.pop_front();
    if (auto startIdx = ctx.GetRequiredValue<Int>(startArg)) {
      size_t count = std::string::npos;
      if (!ctx.Args.empty()) {
        auto countArg = std::move(ctx.Args.front());
        ctx.Args.pop_front();
        if (auto countValue = ctx.GetRequiredValue<Int>(countArg))
          count = countValue->Value;
        else
          return false;
      }

      try {
        ctx.Expr.reset(new Str(str->Value.substr(startIdx->Value, count)));
        return true;
      }
      catch (std::out_of_range) {
        return ctx.Error("index " + std::to_string(startIdx->Value) + " is out of range");
      }
    }
    else
      return false;
  }
  else
    return false;
}

bool StdLib::AddStr(EvaluationContext &ctx) {
  std::stringstream ss;
  while (!ctx.Args.empty()) {
    if (auto str = ctx.GetRequiredValue<Str>(ctx.Args.front()))
      ss << str->Value;
    else
      return false;
    ctx.Args.pop_front();
  }

  ctx.Expr = ExpressionPtr { new Str { ss.str() } };
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

  ExpressionPtr listExpr { std::move(ctx.Args.front()) };
  ctx.Args.pop_front();

  ExpressionPtr resultExpr { new Sexp {} };
  auto resultList = static_cast<Sexp*>(resultExpr.get());
  int i = 0;
  if (auto fn = ctx.GetRequiredValue<Function>(fnExpr)) {
    if (auto list = ctx.GetRequiredListValue(listExpr)) {
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
      return false;
  }
  else
    return false;

  ctx.Expr = ExpressionPtr { new Quote { std::move(resultExpr) } };
  return true;
}

bool StdLib::Cons(EvaluationContext &ctx) {
  auto arg1 = std::move(ctx.Args.front());
  ctx.Args.pop_front();
  auto arg2 = std::move(ctx.Args.front());
  ctx.Args.pop_front();

  ExpressionPtr qExpr1 { };
  if (TypeHelper::IsAtom(arg1->Type())) 
    ctx.Args.push_back(ExpressionPtr { new Quote(ExpressionPtr { new Sexp({std::move(arg1)}) }) });
  else
    ctx.Args.push_back(std::move(arg1));

  ExpressionPtr qExpr2 { };
  if (TypeHelper::IsAtom(arg2->Type())) 
    ctx.Args.push_back(ExpressionPtr { new Quote(ExpressionPtr { new Sexp({std::move(arg2)}) }) });
  else
    ctx.Args.push_back(std::move(arg2));

  return Add(ctx);
}

bool StdLib::AddList(EvaluationContext &ctx) {
  ExpressionPtr resultExpr { new Sexp {} };
  auto resultList = static_cast<Sexp*>(resultExpr.get());
  while (!ctx.Args.empty()) {
    if (auto list = ctx.GetRequiredListValue(ctx.Args.front())) {
      while (!list->Args.empty()) {
        resultList->Args.push_back(std::move(list->Args.front()));
        list->Args.pop_front();
      }
    }
    else
      return false;
    ctx.Args.pop_front();
  }

  ctx.Expr = ExpressionPtr { new Quote { std::move(resultExpr) } };
  return true;
}

bool StdLib::Head(EvaluationContext &ctx) {
  ExpressionPtr seqArg { std::move(ctx.Args.front()) };
  if (auto str = TypeHelper::GetValue<Str>(seqArg)) {
    if (str->Value.empty())
      ctx.Expr.reset(new Str());
    else 
      ctx.Expr.reset(new Str(std::string(1, str->Value[0])));
    return true;
  }
  else if (auto list = ctx.GetRequiredListValue(seqArg)) {
    if (list->Args.empty())
      ctx.Expr = StdLib::GetNil();
    else {
      ctx.Expr = std::move(list->Args.front());
      list->Args.pop_front();
    }
    return true;
  }
  else
    return false;
}

bool StdLib::Tail(EvaluationContext &ctx) {
  ExpressionPtr seqArg { std::move(ctx.Args.front()) };
  if (auto str = TypeHelper::GetValue<Str>(seqArg)) {
    if (str->Value.empty())
      ctx.Expr.reset(new Str());
    else
      ctx.Expr.reset(new Str(str->Value.substr(1)));
    return true;
  }
  else if (auto list = ctx.GetRequiredListValue(seqArg)) {
    if (list->Args.empty())
      ctx.Expr = StdLib::GetNil();
    else {
      list->Args.pop_front();
      ExpressionPtr tail { new Sexp {} };
      auto newList = static_cast<Sexp*>(tail.get());
      while (!list->Args.empty()) {
        newList->Args.push_back(std::move(list->Args.front()));
        list->Args.pop_front();
      }
      ctx.Expr = ExpressionPtr { new Quote { std::move(tail) } };
    }
    return true;
  }
  else
    return false;
}

bool StdLib::Last(EvaluationContext &ctx) {
  ExpressionPtr seqArg { std::move(ctx.Args.front()) };
  if (auto str = TypeHelper::GetValue<Str>(seqArg)) {
    if (str->Value.empty())
      ctx.Expr.reset(new Str());
    else
      ctx.Expr.reset(new Str(std::string(1, str->Value.back())));
    return true;
  }
  else if (auto list = ctx.GetRequiredListValue(seqArg)) {
    if (list->Args.empty())
      ctx.Expr = StdLib::GetNil();
    else
      ctx.Expr = list->Args.back()->Clone();
    return true;
  }
  else
    return false;
}

bool StdLib::Range(EvaluationContext &ctx) {
  ExpressionPtr startExpr = std::move(ctx.Args.front());
  ctx.Args.pop_front();
  ExpressionPtr endExpr = std::move(ctx.Args.front());
  ctx.Args.pop_front();

  int64_t step = 1;
  if (!ctx.Args.empty()) {
    ExpressionPtr stepExpr = std::move(ctx.Args.front());
    ctx.Args.pop_front();
    
    if (auto stepV = ctx.GetRequiredValue<Int>(stepExpr)) {
      step = stepV->Value; 
      if (step == 0)
        return ctx.Error("step cannot be zero");
    }
    else
      return false;
  }
  bool positiveStep = step > 0;
  if (auto startV = ctx.GetRequiredValue<Int>(startExpr)) {
    if (auto endV = ctx.GetRequiredValue<Int>(endExpr)) {
      int64_t start = startV->Value,
              end   = endV->Value;
      if (positiveStep) {
        if (start > end)
          return ctx.Error("start cannot be greater than end when using a positive step");
      }
      else {
        if (start < end)
          return ctx.Error("start cannot be less than end when using a negative step");
      }

      ExpressionPtr listSexp { new Sexp };
      auto list = static_cast<Sexp*>(listSexp.get());
      for (int64_t i = start; positiveStep ? (i <= end) : (i >= end); i += step)
        list->Args.push_back(ExpressionPtr { new Int(i) });
      ctx.Expr = ExpressionPtr { new Quote(std::move(listSexp)) };
      return true;
    }
    else
      return false;
  }
  else
    return false;
}

// Logical

bool StdLib::BinaryLogicalFunc(EvaluationContext &ctx, bool isAnd) {
  int argNum = 1;
  while (!ctx.Args.empty()) {
    ExpressionPtr currArg = std::move(ctx.Args.front());
    ctx.Args.pop_front();
    if (ctx.Evaluate(currArg, argNum)) {
      if (auto argValue = ctx.GetRequiredValue<Bool>(currArg)) {
        bool value = argValue->Value;
        if (isAnd ? !value : value) {
          ctx.Expr = ExpressionPtr { new Bool(isAnd ? false : true) };
          return true;
        }
      }
      else
        return false;
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
      if (auto boolArg = ctx.GetRequiredValue<Bool>(arg)) {
        ctx.Expr = ExpressionPtr { new Bool(!boolArg->Value) };
        return true;
      }
      else
        return false;
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
  return BinaryPredicate(ctx, LtT<Bool>, LtT<Int>, LtT<Float>, LtT<Str>);
}

bool StdLib::Gt(EvaluationContext &ctx) {
  return BinaryPredicate(ctx, GtT<Bool>, GtT<Int>, GtT<Float>, GtT<Str>);
}

bool StdLib::Lte(EvaluationContext &ctx) {
  return BinaryPredicate(ctx, LteT<Bool>, LteT<Int>, LteT<Float>, LteT<Str>);
}

bool StdLib::Gte(EvaluationContext &ctx) {
  return BinaryPredicate(ctx, GteT<Bool>, GteT<Int>, GteT<Float>, GteT<Str>);
}

// Branching, scoping and evaluation

bool StdLib::QuoteFn(EvaluationContext &ctx) {
  ExpressionPtr quote { new Quote { std::move(ctx.Args.front()) } };
  ctx.Args.pop_front();
  ctx.Expr = std::move(quote);
  return true;
}

bool StdLib::Unquote(EvaluationContext &ctx) {
  if (ctx.Evaluate(ctx.Args.front(), "1")) {
    ExpressionPtr quoteExpr { std::move(ctx.Args.front()) };
    ExpressionPtr toEvaluate;

    if (auto quote = TypeHelper::GetValue<Quote>(quoteExpr))
      toEvaluate = std::move(quote->Value);
    else 
      toEvaluate = std::move(quoteExpr);

    if (ctx.Evaluate(toEvaluate, "expression"))
      ctx.Expr = std::move(toEvaluate);
    else
      return false;

    return true;
  }
  return false;
}

// (cond 
//   ((n > 0) "greater than zero") 
//   ((n < 0) "less than zero") 
//   (true    "equal to zero")) 
bool StdLib::Cond(EvaluationContext &ctx) {
  int argNum = 1;
  while (!ctx.Args.empty()) {
    ExpressionPtr arg = std::move(ctx.Args.front());
    ctx.Args.pop_front();
    if (auto argSexp = ctx.GetRequiredValue<Sexp>(arg)) {
      if (argSexp->Args.size() == 2) {
        ExpressionPtr boolExpr = std::move(argSexp->Args.front());
        argSexp->Args.pop_front();
        ExpressionPtr statementExpr = std::move(argSexp->Args.front());
        argSexp->Args.pop_front();
        if (ctx.Evaluate(boolExpr, "condition")) {
          if (auto boolResult = ctx.GetRequiredValue<Bool>(boolExpr)) {
            if (boolResult->Value) {
              if (ctx.Evaluate(statementExpr, "statement")) {
                ctx.Expr = std::move(statementExpr);
                return true;
              }
              else
                return false;
            }
          }
          else
            return false;
        }
        else
          return false;
      }
      else
        return ctx.Error("arg " + std::to_string(argNum) + ": expected 2 args. got " + std::to_string(argSexp->Args.size()));
    }
    else
      return false;
    ++argNum;
  }
  ctx.Expr = GetNil();
  return true;
}

// (switch (type x)                  #  (switch (type x)
//   (case int "x is an int")        #    (int "x is an int")
//   (case float "x is a float")     #    (float "x is a float")
//   (default "x is not a number"))  #    ("x is not a number"))

// TODO: needs refactoring!
bool StdLib::Switch(EvaluationContext &ctx) {
  ArgList argCopy;
  ArgListHelper::CopyTo(ctx.Args, argCopy);
  ExpressionPtr varExpr = std::move(argCopy.front());
  argCopy.pop_front();
  int argNum = 1;
  if (ctx.Evaluate(varExpr, "variable")) {
    while (!argCopy.empty()) {
      ExpressionPtr arg = std::move(argCopy.front());
      argCopy.pop_front();
      bool isDefault = argCopy.empty();
      if (auto argSexp = ctx.GetRequiredValue<Sexp>(arg)) {
        const std::string optionalSymbolName = isDefault ? "default" : "case";
        if (argSexp->Args.size() == (isDefault ? 2 : 3)) {
          ExpressionPtr optionalCaseSymbolExpr = std::move(argSexp->Args.front());
          argSexp->Args.front();
          argSexp->Args.pop_front();
          if (auto optionalCaseSymbol = TypeHelper::GetValue<Symbol>(optionalCaseSymbolExpr)) {
            if (optionalCaseSymbol->Value != optionalSymbolName)
              return ctx.Error("Expected \"" + optionalSymbolName + "\"");
          }
          else
            return ctx.Error("Expected \"" + optionalSymbolName + "\" symbol");
        }

        if (isDefault) {
          if (argSexp->Args.size() == 1) {
            ExpressionPtr statementExpr = std::move(argSexp->Args.front());
            argSexp->Args.pop_front();
            if (ctx.Evaluate(statementExpr, "statement")) {
              ctx.Expr = std::move(statementExpr);
              return true;
            }
            else
              return false;
          }
          else
            return ctx.Error("arg " + std::to_string(argNum) + ": expected 1 args. got " + std::to_string(argSexp->Args.size()));
        }
        else {
          if (argSexp->Args.size() == 2) {
            ExpressionPtr valueExpr = std::move(argSexp->Args.front());
            argSexp->Args.pop_front();
            ExpressionPtr statementExpr = std::move(argSexp->Args.front());
            argSexp->Args.pop_front();
            ctx.Args.clear();
            ctx.Args.push_front(std::move(valueExpr));
            ctx.Args.push_front(varExpr->Clone());
            if (Eq(ctx)) {
              if (auto boolResult = ctx.GetRequiredValue<Bool>(ctx.Expr)) {
                if (boolResult->Value) {
                  if (ctx.Evaluate(statementExpr, "statement")) {
                    ctx.Expr = std::move(statementExpr);
                    return true;
                  }
                  else
                    return false;
                }
              }
              else
                return false;
            }
            else
              return false;
          }
          else
            return ctx.Error("arg " + std::to_string(argNum) + ": expected 2 args. got " + std::to_string(argSexp->Args.size()));
        }
      }
      else
        return false;
      ++argNum;
    }
  }
  else
    return false;
  ctx.Expr = GetNil();
  return true;
}

bool StdLib::While(EvaluationContext &ctx) {
  ExpressionPtr lastStatementResult = GetNil();
  ArgList loopArgs;
  while (true) {
    loopArgs.clear();
    ArgListHelper::CopyTo(ctx.Args, loopArgs);
    ExpressionPtr condExpr = std::move(loopArgs.front());
    loopArgs.pop_front();
    if (ctx.Evaluate(condExpr, "condition")) {
      if (auto condResult = ctx.GetRequiredValue<Bool>(condExpr)) {
        if (condResult->Value) {
          int bodyStatementNum = 1;
          while (!loopArgs.empty()) {
            ExpressionPtr currBodyStatement = std::move(loopArgs.front());
            loopArgs.pop_front();
            if (ctx.Evaluate(currBodyStatement, "body" + std::to_string(bodyStatementNum)))
              lastStatementResult = std::move(currBodyStatement);
            else
              return false;
            ++bodyStatementNum;
          }
        }
        else {
          ctx.Expr = std::move(lastStatementResult);
          return true;
        }
      }
      else
        return false;
    }
    else
      return false;
  }

  return false;
}

bool StdLib::If(EvaluationContext &ctx) {
  ExpressionPtr condExpr = std::move(ctx.Args.front());
  ctx.Args.pop_front();

  ExpressionPtr trueExpr = std::move(ctx.Args.front());
  ctx.Args.pop_front();
   
  ExpressionPtr falseExpr = std::move(ctx.Args.front());
  ctx.Args.pop_front();

  if (auto cond = ctx.GetRequiredValue<Bool>(condExpr)) {
    ExpressionPtr *branchExpr = cond->Value ? &trueExpr : &falseExpr;
    if (ctx.Evaluate(*branchExpr, "branch")) {
      ctx.Expr = std::move(*branchExpr);
      return true;
    }
    else
      return false; 
  }
  else
    return false;
}

// Go through all the code and harden, perform additional argument checking

bool StdLib::Let(EvaluationContext &ctx) {
  Scope scope(ctx.Interp.GetCurrentStackFrame().GetLocalSymbols());
  ExpressionPtr varsExpr = std::move(ctx.Args.front());
  ctx.Args.pop_front();
  if (auto vars = ctx.GetRequiredValue<Sexp>(varsExpr)) {
    for (auto &varExpr : vars->Args) {
      if (auto var = ctx.GetRequiredValue<Sexp>(varExpr, "(name1 value1)")) {
        size_t nVarArgs = var->Args.size();
        if (nVarArgs == 2) {
          ExpressionPtr varNameExpr = std::move(var->Args.front());
          var->Args.pop_front();

          ExpressionPtr varValueExpr = std::move(var->Args.front());
          var->Args.pop_front();

          if (auto varName = ctx.GetRequiredValue<Symbol>(varNameExpr)) {
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
            return false;
        }
        else
          return ctx.Error("Expected 2 args: (name1 value1). Got " + std::to_string(nVarArgs) + " args");
      }
      else
        return false;
    }
    return Begin(ctx);
  }
  else
    return false;
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
  if (auto formalsList = ctx.GetRequiredValue<Sexp>(formalsExpr, "formals list")) {
    for (auto &formal : formalsList->Args) {
      if (auto formalSym = ctx.GetRequiredValue<Symbol>(formal))
        anonFuncArgs.push_back(formalSym->Clone());
      else
        return false;
      ++nArgs;
    }
  }
  else
    return false;
  
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
    if (auto quote = TypeHelper::GetValue<Quote>(appArgsExpr)) {
      newArgs = std::move(quote->Value);
    }
    else
      newArgs = std::move(appArgsExpr);

    if (auto appArgsSexp = ctx.GetRequiredValue<Sexp>(newArgs, "list")) {
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
      return false;
  }
  else
    return false; 
}

// Conversion operators

bool StdLib::BoolFunc(EvaluationContext &ctx) {
  bool value = false;
  if (auto &expr = ctx.Args.front()) {
    if (ctx.Evaluate(expr, 1)) {
      if (auto boolValue = TypeHelper::GetValue<Bool>(expr))
        value = boolValue->Value;
      else if (auto intValue = TypeHelper::GetValue<Int>(expr))
        value = intValue->Value != 0;
      else if (auto floatValue = TypeHelper::GetValue<Float>(expr))
        value = floatValue->Value != 0.0;
      else if (auto strValue = TypeHelper::GetValue<Str>(expr))
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
      if (auto boolValue = TypeHelper::GetValue<Bool>(expr))
        value = boolValue->Value;
      else if (auto intValue = TypeHelper::GetValue<Int>(expr))
        value = intValue->Value;
      else if (auto floatValue = TypeHelper::GetValue<Float>(expr))
        value = std::llround(floatValue->Value);
      else if (auto strValue = TypeHelper::GetValue<Str>(expr)) {
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
      if (auto boolValue = TypeHelper::GetValue<Bool>(expr))
        value = boolValue->Value ? 1 : 0;
      else if (auto intValue = TypeHelper::GetValue<Int>(expr))
        value = static_cast<double>(intValue->Value);
      else if (auto floatValue = TypeHelper::GetValue<Float>(expr))
        value = floatValue->Value;
      else if (auto strValue = TypeHelper::GetValue<Str>(expr)) {
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
      ctx.Expr = ExpressionPtr { new Str(expr->ToString()) };
      return true;
    }
    else
      return false;
  }
  return false;
}

bool StdLib::TypeFunc(EvaluationContext &ctx) {
  std::string typeName;
  if (auto quote = TypeHelper::GetValue<Quote>(ctx.Args.front())) {
    if (IsQuoteAList(ctx, *quote)) 
      typeName = "list";
    else
      typeName = quote->Type().Name();
  }
  else
    typeName = ctx.Args.front()->Type().Name();

  ExpressionPtr typeSymbol;
  if (ctx.GetSymbol(typeName, typeSymbol)) {
    ctx.Expr = std::move(typeSymbol);
    return true;
  }
  else
    return ctx.Error("unknown type");
}

bool StdLib::TypeQFunc(EvaluationContext &ctx) {
  std::string thisFuncName  = ctx.GetThisFunctionName();
  if (!thisFuncName.empty()) {
    if (thisFuncName.back() == '?') {
      thisFuncName.erase(thisFuncName.end() - 1);
      bool isAtomFunc = thisFuncName == "atom";
      ExpressionPtr typeExpr { new Sexp };
      auto *typeSexp = static_cast<Sexp*>(typeExpr.get());
      typeSexp->Args.push_back(ExpressionPtr { new Symbol("type") });
      typeSexp->Args.push_back(ctx.Args.front()->Clone());
      ctx.Args.clear();
      ctx.Args.push_back(ExpressionPtr { new Symbol(isAtomFunc ? "list" : thisFuncName) });
      ctx.Args.push_back(std::move(typeExpr));
      return isAtomFunc ? Ne(ctx) : Eq(ctx);
    }
  }
  return false; 
}

// Helpers

bool StdLib::IsQuoteAList(EvaluationContext &ctx, Quote &quote) {
  auto &quoteValue = quote.Value;
  if (quoteValue) {
    if (auto sexp = TypeHelper::GetValue<Sexp>(quoteValue)) {
      if (sexp->Args.empty())
        return true;
      else {
        auto &firstSexpArg = sexp->Args.front();
        if (auto sym = TypeHelper::GetValue<Symbol>(firstSexpArg)) {
          ExpressionPtr symValue;
          if (ctx.GetSymbol(sym->Value, symValue)) {
            if (TypeHelper::IsA<Function>(symValue))
              return false;
            else
              return true;
          }
        }
        return true;
      }
    }
    else
      return false;
  }
  else
    return false;
}

template <class T, class F>
bool StdLib::UnaryFunction(EvaluationContext &ctx, F fn) {
  ExpressionPtr numExpr = ctx.Args.front()->Clone();
  if (auto num = ctx.GetRequiredValue<T>(numExpr)) {
    ctx.Expr = ExpressionPtr { new T { fn(num->Value) } };
    return true;
  }
  return false;
}

template<class T, class F>
bool StdLib::BinaryFunction(EvaluationContext &ctx, F fn) {
  bool first = true;
  T result { 0 };
  int argNum = 1;
  while (!ctx.Args.empty()) {
    bool ok = false;
    if (ctx.Evaluate(ctx.Args.front(), argNum)) {
      if (auto num = TypeHelper::GetValue<T>(ctx.Args.front())) {
        if (first)
          result.Value = num->Value;
        else
          result.Value = fn(result.Value, num->Value);
        ok = true;
      }
    }
    else
      return false;

    if (!ok)
      return ctx.TypeError<T>(ctx.Args.front());

    first = false;
    ctx.Args.pop_front();
    ++argNum;
  }

  ctx.Expr = ExpressionPtr { new T { result } };
  return true;
}

template <class B, class I, class F, class S>
bool StdLib::BinaryPredicate(EvaluationContext &ctx, B bFn, I iFn, F fFn, S sFn) {
  auto currArg = ctx.Args.begin();
  if (currArg != ctx.Args.end()) {
    if (ctx.Evaluate(*currArg, 1)) {
      Bool defaultValue { true };
      if (bFn && TypeHelper::IsA<Bool>(*currArg))
        return PredicateHelper<Bool>(ctx, bFn, defaultValue);
      else if (iFn && TypeHelper::IsA<Int>(*currArg))
        return PredicateHelper<Int>(ctx, iFn, defaultValue);
      else if (fFn && TypeHelper::IsA<Float>(*currArg))
        return PredicateHelper<Float>(ctx, fFn, defaultValue);
      else if (sFn && TypeHelper::IsA<Str>(*currArg))
        return PredicateHelper<Str>(ctx, sFn, defaultValue);
      else
        return ctx.TypeError<Literal>(*currArg);
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
  if (auto last = ctx.GetRequiredValue<T>(firstArg)) {
    int argNum = 1;
    while (!ctx.Args.empty()) {
      if (ctx.Evaluate(ctx.Args.front(), argNum)) {
        auto curr = TypeHelper::GetValue<T>(ctx.Args.front());
        if (curr) {
          R tmp { fn(result, *last, *curr) };
          result = tmp;
        }
        else
          return ctx.TypeError<T>(ctx.Args.front());

        *last = *curr;
        ctx.Args.pop_front();
      }
      else
        return false; 

      ++argNum;
    }
  }
  else
    return false;

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
