#include <iostream>
#include <memory>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <cctype>
#include <cstdio>
#include <cstdarg>
#include <csignal>

#include "StdLib.h"
#include "../Interpreter.h"

#include "../NumConverter.h"
#include "../FileSystem.h"

using namespace std;

//=============================================================================

bool StdLib::Load(Interpreter &interpreter) {
  auto* thisModule = interpreter.CreateModule("StdLib", "StdLib");
  if (!thisModule)
    return false;

  SourceContext_ = SourceContext(thisModule, 0);

  // Constants

  auto &symbols = interpreter.GetDynamicSymbols(SourceContext_);
  auto &settings = interpreter.GetSettings();

  LoadEnvironment(symbols, interpreter.GetEnvironment());

  symbols.PutSymbolFloat("PI", 3.14159265358979323846);
  symbols.PutSymbolFloat("E", 2.71828182845904523536);

  // Interpreter

  FuncDef dispDef { FuncDef::AnyArgs(Literal::TypeInstance), FuncDef::NoArgs() };
  symbols.PutSymbolFunction(
    "display",
    {"(display .. args) -> nil"},
    "dump each arg to console. similar to (print)",
    {{"(display \"hello world\")", "\"hello world\""}},
    StdLib::Display,
    dispDef.Clone()
  );
  symbols.PutSymbolFunction(
    "print",
    {"(print .. args) -> nil"}, 
    "print each arg (strings are not show with quotes). similar to (display)",
    {{"(print \"hello world\")", "hello world"}},
    StdLib::Print,
    dispDef.Clone()
  );
  symbols.PutSymbolFunction(
    "prompt",
    {"(prompt prefix) -> str"},
    "print prefix, then read a line from stdin",
    {{"(prompt \"name: \")", "name: John\n\"John\""}},
    StdLib::Prompt,
    FuncDef { FuncDef::ManyArgs(Str::TypeInstance, 0, 1), FuncDef::OneArg(Str::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "quit",
    {"(quit) -> nil"},
    "exit the REPL",
    {},
    StdLib::Quit,
    FuncDef { FuncDef::NoArgs(), FuncDef::NoArgs() }
  );
  symbols.PutSymbolFunction(
    "exit",
    {"(exit exitCode) -> nil"},
    "exit the REPL with exitCode",
    {},
    StdLib::Exit,
    FuncDef { FuncDef::OneArg(Int::TypeInstance), FuncDef::NoArgs() }
  );
  symbols.PutSymbolFunction(
    "symbols",
    {"(symbols) -> list"},
    "get list of all symbols", 
    {},
    StdLib::Symbols,
    FuncDef { FuncDef::NoArgs(), FuncDef::OneArg(Quote::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "help",
    {"(help symbol) -> nil", "(help str) -> nil"},
    "get help on a particular symbol. see also: help.signatures, help.doc, help.examples, symbols",
    {},
    StdLib::Help,
    FuncDef { FuncDef::AnyArgs(Sexp::TypeInstance), FuncDef::NoArgs() }
  );
  symbols.PutSymbolFunction(
    "help.signatures",
    {"(help.signatures symbol) -> str", "(help.signatures str) -> str"},
    "get signatures of a particular symbol",
    {},
    StdLib::HelpSignatures,
    FuncDef { FuncDef::OneArg(Sexp::TypeInstance), FuncDef::OneArg(Str::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "help.doc",
    {"(help.doc symbol) -> str", "(help.doc str) -> str"},
    "get doc of a particular symbol",
    {},
    StdLib::HelpDoc,
    FuncDef { FuncDef::OneArg(Sexp::TypeInstance), FuncDef::OneArg(Str::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "help.examples",
    {"(help.examples symbol) -> list", "(help.examples str) -> list"},
    "get examples of a particular symbol",
    {},
    StdLib::HelpExamples,
    FuncDef { FuncDef::OneArg(Sexp::TypeInstance), FuncDef::OneArg(Quote::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "infix-register",
    {"(infix-register symbol) -> nil"},
    "support calling symbol (a binary function) in infix form",
    {{"(set add +)", ""}, {"(infix-register add)", ""}, {"(3 add 4)", "7"}},
    StdLib::InfixRegister,
    FuncDef { FuncDef::OneArg(Symbol::TypeInstance), FuncDef::NoArgs() }
  );
  symbols.PutSymbolFunction(
    "infix-unregister",
    {"(infix-unregister symbol) -> nil"},
    "unregister previously defined infix symbol",
    {{"(infix-unregister add)",""}},
    StdLib::InfixUnregister,
    FuncDef { FuncDef::OneArg(Symbol::TypeInstance), FuncDef::NoArgs() }
  );

  // IO
  initializer_list<ExampleDef> ioExample {
    {"(file.writelines \"ioExample.txt\" (\"line1\" \"line2\"))", "true"},
    {"(file.readlines \"ioExample.txt\")", "(\"line1\" \"line2\")"},
    {"(file.delete \"ioExample.txt\")", "true"},
    {"(file.exists \"ioExample.txt\")", "false"}
  };
  
  symbols.PutSymbolFunction(
    "file.exists",
    {"(file.exists filepath) -> bool"},
    "does filepath exist",
    ioExample,
    StdLib::Exists,
    FuncDef { FuncDef::OneArg(Str::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "file.delete",
    {"(file.delete filepath) -> bool"},
    "delete filepath",
    ioExample,
    StdLib::Delete,
    FuncDef { FuncDef::OneArg(Str::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "file.readlines",
    {"(file.readlines filepath) -> list"},
    "returns a list of all lines in filepath",
    ioExample,
    StdLib::ReadLines,
    FuncDef { FuncDef::OneArg(Str::TypeInstance), FuncDef::OneArg(Quote::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "file.writelines",
    {"(file.writelines filepath lines) -> bool"},
    "write list of lines into filepath (overwrite what's already there)",
    ioExample,
    StdLib::WriteLines,
    FuncDef { FuncDef::Args({&Str::TypeInstance, &Quote::TypeInstance}), FuncDef::OneArg(Bool::TypeInstance) }
  );

  // Assignment Operators

  FuncDef setDef { FuncDef::Args({&Symbol::TypeInstance, &Sexp::TypeInstance}), FuncDef::OneArg(Literal::TypeInstance) };  
  symbols.PutSymbolFunction(
    "set", 
    {"(set symbol value) -> value"},
    "sets a symbol to value",
    {{"(set foo 42)", "42"}},
    StdLib::Set,
    setDef.Clone()
  ); 
  symbols.PutSymbolFunction(
    "=",
    {"(= symbol value) -> value"},
    "alias for (set)",
    {{"(= foo 42)", "42"}},
    StdLib::Set,   
    setDef.Clone()
  ); 

    
  string setWithOpDoc = "perform operation on current symbol and return new value. value can be: int, float, str, list";
  symbols.PutSymbolFunction(
    "+=",
    {"(+= symbol value) -> value"},
    setWithOpDoc,
    {{"(= foo 2)", "2"}, {"(+= foo 3)", "5"}},
    StdLib::Set, 
    setDef.Clone()
  ); 
  symbols.PutSymbolFunction(
    "-=",
    {"(-= symbol value) -> value"},
    setWithOpDoc,
    {{"(= foo 2)", "2"}, {"(-= foo 3)", "-1"}},
    StdLib::Set,
    setDef.Clone()
  ); 
  symbols.PutSymbolFunction(
    "*=",
    {"(*= symbol value) -> value"},
    setWithOpDoc,
    {{"(= foo 2)", "2"}, {"(*= foo 3)", "6"}},
    StdLib::Set,
    setDef.Clone()
  ); 
  symbols.PutSymbolFunction(
    "/=",
    {"(/= symbol value) -> value"},
    setWithOpDoc,
    {{"(= foo 2)", "2"}, {"(/= foo 3)", "0"}},
    StdLib::Set,
    setDef.Clone()
  ); 
  symbols.PutSymbolFunction(
    "%=",
    {"(%= symbol value) -> value"},
    setWithOpDoc,
    {{"(= foo 2)", "2"}, {"(%= foo 3)", "2"}},
    StdLib::Set,
    setDef.Clone()
  ); 
  symbols.PutSymbolFunction(
    "<<=",
    {"(<<= symbol value) -> value"},
    setWithOpDoc, 
    {{"(= foo 2)", "2"}, {"(<<= foo 3)", "16"}},
    StdLib::Set,
    setDef.Clone()
  ); 
  symbols.PutSymbolFunction(
    ">>=",
    {"(>>= symbol value) -> value"},
    setWithOpDoc,
    {{"(= foo 2)", "2"}, {"(>>= foo 3)", "0"}},
    StdLib::Set,
    setDef.Clone()
  ); 
  symbols.PutSymbolFunction(
    "&=",
    {"(&= symbol value) -> value"},
    setWithOpDoc,
    {{"(= foo 2)", "2"}, {"(&= foo 3)", "2"}},
    StdLib::Set,
    setDef.Clone()
  ); 
  symbols.PutSymbolFunction(
    "^=",
    {"(^= symbol value) -> value"},
    setWithOpDoc,
    {{"(= foo 2)", "2"}, {"(^= foo 3)", "1"}},
    StdLib::Set, 
    setDef.Clone()
  ); 
  symbols.PutSymbolFunction(
    "|=",
    {"(|= symbol value) -> value"},
    setWithOpDoc,
    {{"(= foo 2)", "2"}, {"(|= foo 3)", "3"}},
    StdLib::Set,  
    setDef.Clone()
  ); 
  
  FuncDef incrDef { FuncDef::OneArg(Symbol::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) };
  symbols.PutSymbolFunction(
    "++",
    {"(++ symbol) -> value"},
    "increment symbol and return new value",
    {{"(= foo 2)", "2"}, {"(++ foo)", "3"}},
    StdLib::Set,
    incrDef.Clone()
  ); 
  symbols.PutSymbolFunction(
    "--",
    {"(-- symbol) -> value"},
    "decrement symbol and return new value",
    {{"(= foo 2)", "2"}, {"(-- foo)", "1"}},
    StdLib::Set, 
    incrDef.Clone()
  ); 

  symbols.PutSymbolFunction(
    "unset",
    {"(unset symbol) -> value"},
    "make symbol undefined",
    {},
    StdLib::UnSet,
    FuncDef { FuncDef::OneArg(Symbol::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) }
  );

  // Generic

  symbols.PutSymbolFunction(
    "length",
    {"(length iterable) -> int"},
    "return the length of iterable (str, list)",
    {{"(length \"abc\")", "3"}, {"(length (42 53 64))", "3"}},
    StdLib::Length, 
    FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Int::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "+",
    {"(+ .. values) -> value"},
    "add each value and return result. all values must be same type which can be: int, float, str, list",
    {{"(+ 2 3)", "5"}, {"(+ \"a\" \"bc\")", "\"abc\""}},
    StdLib::Add,
    FuncDef { FuncDef::AtleastOneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "empty?",
    {"(empty? iterable) -> bool"},
    "return whether iterable (list, str) is empty",
    {{"(empty? \"abc\")", "false"}},
    StdLib::EmptyQ,
    FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "-",
    {"(- .. nums) -> num"},
    "subtract each num and return result. all values must be same type which can be: int, float",
    {{"(- 42 10)", "32"}},
    StdLib::Sub,
    FuncDef { FuncDef::AtleastOneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "*",
    {"(* .. nums) -> num"},
    "multiply each num and return result. all values must be same type which can be: int, float",
    {{"(* 2 3)", "6"}},
    StdLib::Mult, 
    FuncDef { FuncDef::AtleastOneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "/",
    {"(/ .. nums) -> num"},
    "divide each num and return result. all values must be same type which can be: int, float",
    {{"(/ 42 6)", "7"}},
    StdLib::Div, 
    FuncDef { FuncDef::AtleastOneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) });

  FuncDef powDef { FuncDef::ManyArgs(Literal::TypeInstance, 2), FuncDef::OneArg(Literal::TypeInstance) };
  symbols.PutSymbolFunction(
    "pow",
    {"(pow base exponent) -> num"},
    "base to the power of exponent. base and exponent must be same type which can be: int, float",
    {{"(pow 2 3)", "8"}},
    StdLib::Pow,
    powDef.Clone()
  );
  symbols.PutSymbolFunction(
    "**",
    {"(** base expontent) -> num"},
    "alias for (pow)",
    {},
    StdLib::Pow,
    powDef.Clone()
  );
  symbols.PutSymbolFunction(
    "abs",
    {"(abs num) -> num"},
    "absolute value of num which can be: int, float",
    {},
    StdLib::Abs,
    FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "max",
    {"(max .. nums) -> num"},
    "return maximum num",
    {{"(max 42 54 23)", "54"}},
    StdLib::Max,
    FuncDef { FuncDef::AtleastOneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "min",
    {"(min .. nums) -> num"},
    "return minimum num",
    {{"(min 42 54 23)", "23"}},
    StdLib::Min,
    FuncDef { FuncDef::AtleastOneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) }
  );

  FuncDef foreachDef { FuncDef::ManyArgs(Sexp::TypeInstance, 2, ArgDef::ANY_ARGS), FuncDef::OneArg(Literal::TypeInstance) };
  symbols.PutSymbolFunction(
    "foreach",
    {"(foreach item iterable .. expressions) -> value", "(foreach item in iterable .. expressions) -> value",
    "(foreach item : iterable .. expressions) -> value", "(foreach iterable fn) -> value"},
    "evaluate fn or expressions on each item in iterable. return last expression evaluated",
    {{"(foreach num (42 54 23) (print num))", "42\n54\n23\n23"}},
    StdLib::Foreach,
    foreachDef.Clone()
  );
  symbols.PutSymbolFunction(
    "for",
    {"(for item iterable .. expressions) -> value", "(for item in iterable .. expressions) -> value",
     "(for item : iterable .. expressions) -> value", "(for iterable fn) -> value"},
    "alias for (foreach)",
    {},
    StdLib::Foreach, 
    foreachDef.Clone()
  );

  symbols.PutSymbolFunction(
    "reverse",
    {"(reverse iterable) -> new-iterable"},
    "return the reverse of iterable which can be: str, list",
    {{"(reverse \"abc\")", "\"cba\""}},
    StdLib::Reverse,
    FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) }
  );

  FuncDef headDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) };
  symbols.PutSymbolFunction(
    "head",
    {"(head iterable) -> value"},
    "return first item in iterable which can be: str, list",
    {{"(reverse \"abc\")", "\"a\""}},
    StdLib::Head,
    headDef.Clone()
  );
  symbols.PutSymbolFunction(
    "car",
    {"(car iterable) -> value"},
    "alias for (head)",
    {},
    StdLib::Head,
    headDef.Clone()
  );
  symbols.PutSymbolFunction(
    "first",
    {"(first iterable) -> value"},
    "alias for (head)",
    {},
    StdLib::Head,
    headDef.Clone()
  );
  symbols.PutSymbolFunction(
    "front",
    {"(front iterable) -> value"},
    "alias for (head)",
    {},
    StdLib::Head,
    headDef.Clone()
  );

  symbols.PutSymbolFunction(
    "tail",
    {"(tail iterable) -> iterable"},
    "returns new iterable (str, list) without first element",
    {{"(tail \"abc\")", "bc"}},
    StdLib::Tail,
    headDef.Clone()
  );
  symbols.PutSymbolFunction(
    "cdr", 
    {"(cdr iterable) -> iterable"},
    "alias for (tail)",
    {},
    StdLib::Tail,
    headDef.Clone()
  );
  symbols.PutSymbolFunction(
    "rest", 
    {"(rest iterable) -> iterable"},
    "alias for (tail)",
    {},
    StdLib::Tail, 
    headDef.Clone()
  );

  symbols.PutSymbolFunction(
    "last",
    {"(last iterable) -> iterable"},
    "return last item in iterable (str, list)",
    {{"(last \"abc\")", "\"c\""}},
    StdLib::Last, 
    headDef.Clone()
  );
  symbols.PutSymbolFunction(
    "back", 
    {"(back iterable) -> iterable"},
    "alias for (last)",
    {},
    StdLib::Last, 
    headDef.Clone()
  );

  FuncDef atDef { FuncDef::Args({&Literal::TypeInstance, &Int::TypeInstance}), FuncDef::OneArg(Literal::TypeInstance)};
  symbols.PutSymbolFunction(
    "at", 
    {"(at iterable index) -> value"},
    "return index (zero based) of iterable (str, list)",
    {{"(at \"abc\" 1)", "\"b\""}},
    StdLib::At,
    atDef.Clone()
  );
  symbols.PutSymbolFunction(
    "nth", 
    {"(nth iterable index) -> value"},
    "alias for (at)",
    {},
    StdLib::At, 
    atDef.Clone()
  );
  //TODO: Lists
  //TODO: ("abc" 1) => "b" ??????????
  //TODO: "abc"[1] => b
  //TODO: ([] "abc" 1) => b ????
  //TODO: ([1] "abc") => b ????


  // Float

  symbols.PutSymbolFunction(
    "exp", 
    {"(exp float) -> float"},
    "E to the power",
    {},
    StdLib::Exp, 
    FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "log", 
    {"(log float) -> float"},
    "natural logarithm (base E)",
    {},
    StdLib::Log, 
    FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "sqrt", 
    {"(sqrt float) -> float"},
    "square root",
    {},
    StdLib::Sqrt, 
    FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "ceil", 
    {"(ceil float) -> float"},
    "ceiling",
    {{"(ceil 1.4)", "2.0"}},
    StdLib::Ceil, 
    FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "floor", 
    {"(floor float) -> float"},
    "floor",
    {{"(floor 1.4)", "1.0"}},
    StdLib::Floor, 
    FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "round", 
    {"(round float) -> float"},
    "round to the nearest whole number",
    {{"(round 1.4)", "1.0"}},
    StdLib::Round, 
    FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "cos", 
    {"(cos radians) -> float"},
    "cosine of angle in radians",
    {{"(cos (PI / 2))", "0"}},
    StdLib::Cos, 
    FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "sin", 
    {"(sin radians) -> float"},
    "sine of angle in radians",
    {{"(sin (PI / 2))", "1.0"}},
    StdLib::Sin, 
    FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "tan", 
    {"(tan radians) -> float"},
    "tangent of angle in radians",
    {},
    StdLib::Tan, 
    FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "acos", 
    {"(acos float) -> float"},
    "arc cosine",
    {},
    StdLib::ACos, 
    FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "asin", 
    {"(asin float) -> float"},
    "arc sine",
    {},
    StdLib::ASin, 
    FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "atan", 
    {"(atan float) -> float"},
    "arc tangent",
    {},
    StdLib::ATan, 
    FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "atan2", 
    {"(atan2 x y) -> float"},
    "arc tangent of y/x (both must be floats)",
    {},
    StdLib::ATan2, 
    FuncDef { FuncDef::ManyArgs(Float::TypeInstance, 2), FuncDef::OneArg(Float::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "cosh", 
    {"(cosh float) -> float"},
    "hyperbolic cosine",
    {},
    StdLib::Cosh, 
    FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "sinh", 
    {"(sinh float) -> float"},
    "hyperbolic sine",
    {},
    StdLib::Sinh, 
    FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "tanh", 
    {"(tanh float) -> float"},
    "hyperbolic tan",
    {},
    StdLib::Tanh, 
    FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "acosh", 
    {"(acosh float) -> float"},
    "arc hyperbolic cosine",
    {},
    StdLib::ACosh, 
    FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "asinh", 
    {"(asinh float) -> float"},
    "arc hyperbolic sine",
    {},
    StdLib::ASinh, 
    FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "atanh", 
    {"(atanh float) -> float"},
    "arc hyperbolic tangent",
    {},
    StdLib::ATanh, 
    FuncDef { FuncDef::OneArg(Float::TypeInstance), FuncDef::OneArg(Float::TypeInstance) }
  );

  // Numerical

  symbols.PutSymbolFunction(
    "incr", 
    {"(incr int) -> int"},
    "alias for (++)",
    {},
    StdLib::Incr, 
    FuncDef { FuncDef::OneArg(Int::TypeInstance), FuncDef::OneArg(Int::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "decr", 
    {"(decr int) -> int"},
    "alias for (--)",
    {},
    StdLib::Decr, 
    FuncDef { FuncDef::OneArg(Int::TypeInstance), FuncDef::OneArg(Int::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "%", 
    {"(% .. values) -> int"},
    "modulus",
    {{"(% 5 2)", "1"}},
    StdLib::Mod, 
    FuncDef { FuncDef::AtleastOneArg(Int::TypeInstance), FuncDef::OneArg(Int::TypeInstance) }
  );

  FuncDef baseFn { FuncDef::OneArg(Int::TypeInstance), FuncDef::OneArg(Str::TypeInstance) };
  symbols.PutSymbolFunction(
    "hex", 
    {"(hex int) -> str"},
    "return str of int converted to base 16",
    {{"(hex 12)", "\"0xc\""}},
    StdLib::Hex, 
    baseFn.Clone()
  );
  symbols.PutSymbolFunction(
    "bin", 
    {"(bin int) -> str"},
    "return str of int converted to base 2",
    {{"(bin 12)", "\"0b1100\""}},
    StdLib::Bin,
    baseFn.Clone()
  );
  symbols.PutSymbolFunction(
    "dec", 
    {"(dec int) -> str"},
    "return str of int converted to base 10",
    {{"(dec 0xc)", "\"12\""}},
    StdLib::Dec, 
    baseFn.Clone()
  );

  FuncDef intPredDef { FuncDef::OneArg(Int::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) };
  symbols.PutSymbolFunction(
    "even?", 
    {"(even? int) -> bool"},
    "true if int is even",
    {{"(even? 42)", "true"}},
    StdLib::Even, 
    intPredDef.Clone()
  );
  symbols.PutSymbolFunction(
    "odd?", 
    {"(odd? int) -> bool"},
    "true if int is odd",
    {{"(odd? 42)", "false"}},
    StdLib::Odd, 
    intPredDef.Clone()
  );
  symbols.PutSymbolFunction(
    "zero?", 
    {"(zero? int) -> bool"},
    "true if int is zero",
    {{"(zero? 42)", "false"}},
    StdLib::Zero, 
    intPredDef.Clone()
  );

  // Bitwise

  symbols.PutSymbolFunction(
    "<<", 
    {"(<< .. ints) -> int"},
    "left bitwise shift",
    {{"(<< 1 3)", "8"}},
    StdLib::LeftShift, 
    FuncDef { FuncDef::AtleastOneArg(Int::TypeInstance), FuncDef::OneArg(Int::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    ">>", 
    {"(>> .. ints) -> int"},
    "right bitwise shift",
    {{"(>> 8 3)", "1"}},
    StdLib::RightShift, 
    FuncDef { FuncDef::AtleastOneArg(Int::TypeInstance), FuncDef::OneArg(Int::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "&", 
    {"(& .. ints) -> int"},
    "bitwise and",
    {{"(& 3 6)", "2"}},
    StdLib::BitAnd, 
    FuncDef { FuncDef::AtleastOneArg(Int::TypeInstance), FuncDef::OneArg(Int::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "|", 
    {"(| .. ints) -> int"},
    "bitwise or",
    {{"(| 3 4)", "7"}},
    StdLib::BitOr, 
    FuncDef { FuncDef::AtleastOneArg(Int::TypeInstance), FuncDef::OneArg(Int::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "^", 
    {"(^ .. ints) -> int"},
    "bitwise xor",
    {{"(^ 3 6)", "5"}},
    StdLib::BitXor, 
    FuncDef { FuncDef::AtleastOneArg(Int::TypeInstance), FuncDef::OneArg(Int::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "~", 
    {"(~ int) -> int"},
    "bitwise not",
    {{"(~ 0)", "-1"}},
    StdLib::BitNot, 
    FuncDef { FuncDef::OneArg(Int::TypeInstance), FuncDef::OneArg(Int::TypeInstance) }
  );

  // Str 

  FuncDef trimDef { FuncDef::OneArg(Str::TypeInstance), FuncDef::OneArg(Str::TypeInstance) };
  symbols.PutSymbolFunction(
    "trim", 
    {"(trim str) -> str"},
    "return str that has leading and trailing whitespace removed",
    {{"(trim \" abc  \")", "\"abc\""}},
    StdLib::Trim, 
    trimDef.Clone()
  );
  symbols.PutSymbolFunction(
    "upper", 
    {"(upper str) -> str"},
    "return upper cased str",
    {{"(uppper \"aBc\")", "\"ABC\""}},
    StdLib::Upper, 
    trimDef.Clone()
  );
  symbols.PutSymbolFunction(
    "lower", 
    {"(lower str) -> str"},
    "return lower cased str",
    {{"(lower \"AbC\")", "\"abc\""}},
    StdLib::Lower,
    trimDef.Clone()
  );

  symbols.PutSymbolFunction(
    "substr", 
    {"(substr str start) -> str", "(substr str start count) -> str"},
    "returns new str starting at start (zero based) and optionally spanning a maximum of count number of characters",
    {{"(substr \"abcde\" 1)", "\"bcde\""}, {"(substr \"abcde\" 1 2)", "\"bc\""}},
    StdLib::SubStr, 
    FuncDef { FuncDef::ManyArgs(Literal::TypeInstance, 2, 3), FuncDef::OneArg(Str::TypeInstance) }
  );

  FuncDef compareDef { FuncDef::ManyArgs(Str::TypeInstance, 2), FuncDef::OneArg(Int::TypeInstance) };
  symbols.PutSymbolFunction(
    "compare", 
    {"(compare str1 str2) -> int"},
    "lexiographical comparison of str1 and str2.\nstr1 < str2 returns -1\nstr1 == str2 returns 0\nstr1 > str2 returns 1",
    {{"(compare \"abc\" \"abc\")", "0"}},
    StdLib::Compare, 
    compareDef.Clone()
  );

  FuncDef findDef { FuncDef::ManyArgs(Literal::TypeInstance, 2, 3), FuncDef::OneArg(Int::TypeInstance) };
  symbols.PutSymbolFunction(
    "find", 
    {"(find haystack needle) -> int", "(find haystack needle start) -> int"},
    "find needle in haystack, optionally starting at start (zero based)",
    {{"(find \"hello caramellow!\" \"lo\")", "3"}, {"(find \"hello caramellow!\" \"lo\" 5)", "13"}},
    StdLib::Find, 
    findDef.Clone()
  );
  symbols.PutSymbolFunction(
    "rfind", 
    {"(rfind haystack needle) -> int", "(rfind haystack needle start) -> int"},
    "find needle in haystack starting at the end of haystack, optionally starting at start (zero based)",
    {},
    StdLib::RFind, 
    findDef.Clone()
  );

  FuncDef containsDef { FuncDef::ManyArgs(Str::TypeInstance, 2), FuncDef::OneArg(Bool::TypeInstance) };
  symbols.PutSymbolFunction(
    "contains", 
    {"(contains haystack needle) -> bool"},
    "true if haystack contains needle",
    {{"(contains \"abcde\" \"cd\")", "true"}},
    StdLib::Contains, 
    containsDef.Clone()
  ); 
  symbols.PutSymbolFunction(
    "startswith",
    {"(startswith haystack needle) -> bool"},
    "true if haystack starts with needle",
    {{"(startswith \"abcde\" \"ab\")", "true"}},
    StdLib::StartsWith, 
    containsDef.Clone()
  );
  symbols.PutSymbolFunction(
    "endswith", 
    {"(endswith haystack needle) -> bool"},
    "true if haystack ends with needle",
    {{"(endswith \"abcde\" \"de\")", "true"}},
    StdLib::EndsWith, 
    containsDef.Clone()
  );

  symbols.PutSymbolFunction(
    "replace", 
    {"(replace haystack needle) -> str", "(replace haystack needle replacement) -> str",
     "(replace haystack needle replacement maxReplacements) -> str"},
    "replace needle with replacement inside haystack for maxReplacements times. if replacement isn't specified, the empty string is used.",
    {{"(replace \"hello world\" \"o\")", "\"hell wrld\""},
     {"(replace \"hello world\" \"o\" \"X\")", "\"hellX wXrld\""},
     {"(replace \"hello world\" \"o\" \"X\" 1)", "\"hellX world\""}},
    StdLib::Replace, 
    FuncDef { FuncDef::ManyArgs(Literal::TypeInstance, 2, 4), FuncDef::OneArg(Str::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "split", 
    {"(split str delimiter) -> list"},
    "split str using delimiter", 
    {{"(split \"jon,42\" \",\")", "(\"jon\" \"42\")"}},
    StdLib::Split, 
    FuncDef { FuncDef::ManyArgs(Literal::TypeInstance, 2, 3), FuncDef::OneArg(Quote::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "join", 
    {"(join list delimiter) -> str"},
    "build a str by joining all items in list together with delimiter",
    {{"(join (\"jon\" \"42\") \",\")", "jon,42"}},
    StdLib::Join, 
    FuncDef { FuncDef::ManyArgs(Literal::TypeInstance, 2, 3), FuncDef::OneArg(Str::TypeInstance) }
  );

  symbols.PutSymbolFunction(
    "format", 
    {"(format pattern .. args) -> str"},
    "build a str using pattern and args.\n{} means pick next positional arg\n{0} means pick first arg\n{foo} means get value of variable foo in the current scope",
    {{"(format \"{} is {} years old\" \"jon\" 42)", "jon is 42 years old"},
     {"(format \"{0} is {1} years old\" \"jon\" 42)", "jon is 42 years old"},
     {"(let ((name \"jon\") (age 42)) (format \"{name} is {age} years old\"))", "jon is 42 years old"}},
    StdLib::Format, 
    FuncDef { FuncDef::ManyArgs(Literal::TypeInstance, 1, ArgDef::ANY_ARGS), FuncDef::OneArg(Str::TypeInstance) }
  );
  
  // Logical

  symbols.PutSymbolBool("true", true);
  symbols.PutSymbolBool("false", false);

  // Lists

  symbols.PutSymbolQuote("nil", ExpressionPtr { new Sexp(SourceContext_) });
  settings.PutListFunction(CompiledFunction {
    SourceContext_,
    FuncDef { FuncDef::AnyArgs(), FuncDef::OneArg(Quote::TypeInstance) },
    StdLib::List
  });

  symbols.PutSymbolFunction(
    "list",
    {"(list .. values) -> list"},
    "construct a list from values",
    {{"(list 1 2)", "(1 2)"}},
    StdLib::List,
    FuncDef { FuncDef::AnyArgs(), FuncDef::OneArg(Quote::TypeInstance) }
  );

  symbols.PutSymbolFunction("list", CompiledFunction {
    SourceContext_,
    FuncDef { FuncDef::AnyArgs(), FuncDef::OneArg(Quote::TypeInstance) },
    StdLib::List
  });

  FuncDef lstTransformDef { FuncDef::Args({&Function::TypeInstance, &Sexp::TypeInstance}), FuncDef::OneArg(Quote::TypeInstance) };
  symbols.PutSymbolFunction(
    "map", 
    {"(map fn list) -> list"},
    "returns new list with fn applied to each element",
    {{"(map even? (1 2 3))", "(false true false)"}},
    StdLib::Map,
    lstTransformDef.Clone()
  );
  symbols.PutSymbolFunction(
    "filter", 
    {"(filter predicate list) -> list"},
    "returns new list containing only elements which match the predicate",
    {{"(filter even? (1 2 3))", "(2)"}},
    StdLib::Filter, 
    lstTransformDef.Clone()
  );
  symbols.PutSymbolFunction(
    "reduce", 
    {"(reduce binaryFn list) -> value"},
    "aggregate elements in list using binaryFn",
    {{"(reduce + (1 2 3))", "6"}},
    StdLib::Reduce,
    FuncDef { FuncDef::Args({&Function::TypeInstance, &Sexp::TypeInstance}), FuncDef::OneArg(Literal::TypeInstance) }
  );

  FuncDef takeDef { FuncDef::ManyArgs(Sexp::TypeInstance, 2), FuncDef::OneArg(Quote::TypeInstance) };
  symbols.PutSymbolFunction(
    "take", 
    {"(take num list) -> list", "(take predicate list) -> list"},
    "take from list num elements or as long as each element matches predicate",
    {{"(take 2 (4 6 7 8))", "(4 6)"}, {"(take even? (4 6 7 8))", "(4 6)"}},
    StdLib::Take, 
    takeDef.Clone()
  );
  symbols.PutSymbolFunction(
    "skip", 
    {"(skip num list) -> list", "(skip predicate list) -> list"},
    "skip over num elements in list or until predicate no longer matches",
    {{"(skip 2 (4 6 7 8))", "(7 8)"}, {"(skip even? (4 6 7 8))", "(7 8)"}},
    StdLib::Skip, 
    takeDef.Clone()
  );

  FuncDef listPredDef { FuncDef::Args({&Function::TypeInstance, &Sexp::TypeInstance}), FuncDef::OneArg(Bool::TypeInstance) };
  symbols.PutSymbolFunction(
    "any",
    {"(any predicate list) -> bool"},
    "true if any element in list matches predicate",
    {{"(any even? (1 2 3))", "true"}},
    StdLib::Any,
    listPredDef.Clone()
  );
  symbols.PutSymbolFunction(
    "all", 
    {"(all predicate list) -> bool"},
    "true if all elements in list match predicate",
    {{"(all even? (1 2 3))", "false"}},
    StdLib::All, 
    listPredDef.Clone()
  );

  symbols.PutSymbolFunction(
    "zip", 
    {"(zip .. lists) -> list", "(zip fn .. lists) -> list"},
    "zip one or more lists together, optionally using applying fn",
    {{"(zip (1 2) (99 98))", "((1 99) (2 98))"}, {"(zip + (1 2) (99 98))", "(100 100)"}},
    StdLib::Zip,
    FuncDef { FuncDef::ManyArgs(Sexp::TypeInstance, 1, ArgDef::ANY_ARGS), FuncDef::OneArg(Quote::TypeInstance) }
  );

  symbols.PutSymbolFunction(
    "cons", 
    {"(cons item list) -> list"},
    "returns a new list with item at front ",
    {{"(cons 3 (4))", "(3 4)"}},
    StdLib::Push,
    FuncDef { FuncDef::Args({&Literal::TypeInstance, &Quote::TypeInstance}), FuncDef::OneArg(Quote::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "push-front", 
    {"(push-front list item) -> list"},
    "returns a new list with item at front",
    {{"(push-front (4) 3)", "(3 4)"}},
    StdLib::Push,
    FuncDef { FuncDef::Args({&Quote::TypeInstance, &Literal::TypeInstance}), FuncDef::OneArg(Quote::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "push-front!", 
    {"(push-front! list item) -> nil"},
    "add item to front of existing list (in place)",
    {{"(set a '(4))", "(4)"}, {"(push-front! a 3)", "nil"}, {"a", "(3 4)"}},
    StdLib::Push,
    FuncDef { FuncDef::Args({&Symbol::TypeInstance, &Literal::TypeInstance}), FuncDef::OneArg(Quote::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "push-back", 
    {"(push-back list item) -> list"},
    "returns a new list with item at back",
    {{"(push-back (4) 3)", "(4 3)"}},
    StdLib::Push,
    FuncDef { FuncDef::Args({&Quote::TypeInstance, &Literal::TypeInstance}), FuncDef::OneArg(Quote::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "push-back!", 
    {"(push-back! list item) -> nil"},
    "add item to back of existing list (in place)",
    {{"(set a '(4))", "(4)"}, {"(push-back! a 3)", "nil"}, {"a", "(4 3)"}},
    StdLib::Push,
    FuncDef { FuncDef::Args({&Symbol::TypeInstance, &Literal::TypeInstance}), FuncDef::OneArg(Quote::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "pop-front", 
    {"(pop-front list) -> list"},
    "remove item from front of list",
    {{"(pop-front (3 4))", "(4)"}},
    StdLib::Pop,
    FuncDef { FuncDef::Args({&Quote::TypeInstance}), FuncDef::OneArg(Quote::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "pop-front!", 
    {"(pop-front! list) -> nil"},
    "remove item from front of existing list (in place)",
    {{"(set a '(3 4))", ""}, {"(pop-front! a)", "()"}, {"a", "(4)"}},
    StdLib::Pop,
    FuncDef { FuncDef::Args({&Symbol::TypeInstance}), FuncDef::OneArg(Quote::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "pop-back", 
    {"(pop-back list) -> list"},
    "remove item from back of list",
    {{"(pop-back (3 4))", "(3)"}},
    StdLib::Pop,
    FuncDef { FuncDef::Args({&Quote::TypeInstance}), FuncDef::OneArg(Quote::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "pop-back!", 
    {"(pop-back! list) -> nil"},
    "remove item from back of existing list (in place)",
    {{"(set a '(3 4))", ""}, {"(pop-back! a)", "()"}, {"a", "(3)"}},
    StdLib::Pop,
    FuncDef { FuncDef::Args({&Symbol::TypeInstance}), FuncDef::OneArg(Quote::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "range", 
    {"(range start end) -> list", "(range start end stride) -> list"},
    "return a list of all numbers between and start and end, optionally increasing by stride (defaults to 1)",
    {{"(range 2 8)", "(2 3 4 5 6 7 8)"}, {"(range 2 8 4)", "(2 6)"}},
    StdLib::Range,
    FuncDef { FuncDef::ManyArgs(Int::TypeInstance, 2, 3), FuncDef::OneArg(Quote::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "..", 
    {"(.. start end) -> list", "(.. start end stride) -> list"},
    "alias for (range)",
    {},
    StdLib::Range, 
    FuncDef { FuncDef::ManyArgs(Int::TypeInstance, 2), FuncDef::OneArg(Quote::TypeInstance) }
  );

  // Logical

  symbols.PutSymbolFunction(
    "and", 
    {"(and .. expressions) -> bool"},
    "logical and of each boolean expression. shortcuits if false expression encountered",
    {{"(and false true)", "false"}},
    StdLib::And, 
    FuncDef { FuncDef::ManyArgs(Sexp::TypeInstance, 2, ArgDef::ANY_ARGS), FuncDef::OneArg(Bool::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "&&", 
    {"(&& .. expressions) -> bool"},
    "alias for (and)",
    {},
    StdLib::And, 
    FuncDef { FuncDef::ManyArgs(Sexp::TypeInstance, 2, ArgDef::ANY_ARGS), FuncDef::OneArg(Bool::TypeInstance) }
  );

  symbols.PutSymbolFunction(
    "or", 
    {"(or .. expressions) -> bool"},
    "logical or of each expression. shortcuits if true expression encountered",
    {{"(or true false)", "true"}},
    StdLib::Or, 
    FuncDef { FuncDef::ManyArgs(Sexp::TypeInstance, 2, ArgDef::ANY_ARGS), FuncDef::OneArg(Bool::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "||", 
    {"(|| .. expressions) -> bool"},
    "alias for (or)",
    {},
    StdLib::Or, 
    FuncDef { FuncDef::ManyArgs(Sexp::TypeInstance, 2, ArgDef::ANY_ARGS), FuncDef::OneArg(Bool::TypeInstance) }
  );

  symbols.PutSymbolFunction(
    "not", 
    {"(not expression) -> bool"},
    "logical not of expression",
    {{"(not false)", "true"}},
    StdLib::Not, 
    FuncDef { FuncDef::OneArg(Sexp::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "!", 
    {"(! expression) -> bool"},
    "alias for (not)",
    {},
    StdLib::Not, 
    FuncDef { FuncDef::OneArg(Sexp::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) }
  );

  // Comparison

  symbols.PutSymbolFunction(
    "==", 
    {"(== .. values) -> bool"},
    "true if all values are equal",
    {{"(== 3 4 5)", "false"}},
    StdLib::Eq, 
    FuncDef { FuncDef::AtleastOneArg(), FuncDef::OneArg(Bool::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "!=", 
    {"(!= .. values) -> bool"},
    "true if all values are not equal",
    {{"(!= 3 4 5)", "true"}},
    StdLib::Ne, 
    FuncDef { FuncDef::AtleastOneArg(), FuncDef::OneArg(Bool::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "<", 
    {"(< .. ints) -> bool"},
    "true if all values are monotonically decreasing",
    {{"(< 4 3 2)", "true"}},
    StdLib::Lt, 
    FuncDef { FuncDef::AtleastOneArg(), FuncDef::OneArg(Bool::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    ">", 
    {"(> .. ints) -> bool"},
    "true if all values are monotonically increasing",
    {{"(> 2 3 4)", "true"}},
    StdLib::Gt, 
    FuncDef { FuncDef::AtleastOneArg(), FuncDef::OneArg(Bool::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "<=", 
    {"(<= .. values) -> bool"},
    "true if all values are less than or equal to each other",
    {{"(<= 4 3 3)", "true"}},
    StdLib::Lte, 
    FuncDef { FuncDef::AtleastOneArg(), FuncDef::OneArg(Bool::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    ">=", 
    {"(>= .. values) -> bool"},
    "true if all values are greater than or equal to each other",
    {{"(>= 2 3 3)", "true"}},
    StdLib::Gte, 
    FuncDef { FuncDef::AtleastOneArg(), FuncDef::OneArg(Bool::TypeInstance) }
  );

  // Branching, scoping, evaluation

  symbols.PutSymbolFunction(
    "quote", 
    {"(quote expression) -> quote"},
    "quote expression (prevent evaluation)",
    {{"(quote (+ 3 4))", "(+ 3 4)"}},
    StdLib::QuoteFn, 
    FuncDef { FuncDef::OneArg(Sexp::TypeInstance), FuncDef::OneArg(Quote::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "'", 
    {"(' expression) -> quote"},
    "alias for (quote)",
    {},
    StdLib::QuoteFn, 
    FuncDef { FuncDef::OneArg(Sexp::TypeInstance), FuncDef::OneArg(Quote::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "unquote", 
    {"(unquote quotedExpression) -> value"},
    "unquote (evaluate) expression",
    {{"(unquote (quote (+ 3 4)))", "7"}},
    StdLib::Unquote, 
    FuncDef { FuncDef::OneArg(Sexp::TypeInstance), FuncDef::OneArg(Quote::TypeInstance) }
  );

  symbols.PutSymbolFunction(
    "if", 
    {"(if expr trueBranch) -> value", "(if expr trueBranch falseBranch) -> value"},
    "if expr is true, evaluate trueBranch. Otherwise, evaluate falseBranch (if defined)",
    {{"(if (2 < 3) true)", "true"}, {"(if (2 > 3) true false)", "false"}},
    StdLib::If,
    FuncDef { FuncDef::ManyArgs(Sexp::TypeInstance, 2, 3), FuncDef::OneArg(Literal::TypeInstance) }
  );

  symbols.PutSymbolFunction(
    "cond", 
    {"(cond ((expr1 branch1) .. (exprN branchN) (true defaultBranch))) -> value"},
    "generalization of if/else if",
    {{"(cond ((2 < 3) \"less than\") ((2 > 3) \"greater than\") (true \"equal\"))", "less than"}},
    StdLib::Cond, 
    FuncDef { FuncDef::ManyArgs(Sexp::TypeInstance, 1, ArgDef::ANY_ARGS), FuncDef::OneArg(Literal::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "switch", 
    {"(switch expr (value1 branch1) .. (valueN branchN) (defaultBranch)) -> value",
    "(switch expr (case value1 branch1) .. (case valueN branchN) (default defaultBranch)) -> value"},
    "evaluate expr, then evaluate the statement that matches appropriate value (otherwise evaluate defaultValueStatement)",
    {{"(switch 3 (1 \"one\") (2 \"two\") (\"other\"))", "\"other\""}},
    StdLib::Switch, 
    FuncDef { FuncDef::ManyArgs(Sexp::TypeInstance, 3, ArgDef::ANY_ARGS), FuncDef::OneArg(Literal::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "while", 
    {"(while expr .. statements) -> value"},
    "execute statements as long as expr evaluates to true",
    {{"(while false \"this line doesn't evalute\")", "nil"}},
    StdLib::While, 
    FuncDef { FuncDef::ManyArgs(Sexp::TypeInstance, 2, ArgDef::ANY_ARGS), FuncDef::OneArg(Literal::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "let", 
    {"(let ((name1 value1) ..) .. statements) -> value"},
    "evaluate statements with bound variables",
    {{"(let ((a 3) (b 4)) (+ a b))", "7"}},
    StdLib::Let, 
    FuncDef { FuncDef::ManyArgs(Sexp::TypeInstance, 2, ArgDef::ANY_ARGS), FuncDef::OneArg(Literal::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "begin", 
    {"(begin .. statements) -> value"},
    "evaluate statements",
    {},
    StdLib::Begin, 
    FuncDef { FuncDef::AtleastOneArg(Sexp::TypeInstance), FuncDef::OneArg(Literal::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "lambda", 
    {"(lambda (.. vars) .. statements) -> fn"},
    "create anonymous function",
    {{"(lambda (x) (+ x 10))", "<Function>"}},
    StdLib::Lambda, 
    FuncDef { FuncDef::ManyArgs(Sexp::TypeInstance, 2), FuncDef::OneArg(Function::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "fn", 
    {"(fn (.. vars) .. statements) -> fn"},
    "alias for (lambda)",
    {},
    StdLib::Lambda, 
    FuncDef { FuncDef::ManyArgs(Sexp::TypeInstance, 2), FuncDef::OneArg(Function::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "def", 
    {"(def name (.. vars) .. statements) -> fn"},
    "define named function",
    {{"(def add (a b) (+ a b))", "<Function:add>"}},
    StdLib::Def, 
    FuncDef { FuncDef::Args({&Symbol::TypeInstance, &Sexp::TypeInstance, &Sexp::TypeInstance}), FuncDef::OneArg(Function::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "apply", 
    {"(apply fn arglist) -> value"},
    "evaluate fn with arglist",
    {{"(apply + (1 2 3))", "6"}},
    StdLib::Apply, 
    FuncDef { FuncDef::Args({ &Function::TypeInstance, &Sexp::TypeInstance }), FuncDef::OneArg(Literal::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "error", 
    {"(error msg) -> value"},
    "raise an error with msg",
    {{"(error \"boom!\")", ""}},
    StdLib::Error, 
    FuncDef { FuncDef::OneArg(Str::TypeInstance), FuncDef::NoArgs() }
  );
  symbols.PutSymbolFunction(
    "try", 
    {"(try expr catchExpr) -> value"},
    "try expr, invoke catchExpr on error",
    {{"(try (/ 1 0) (error \"boom!\"))", ""}},
    StdLib::Try, 
    FuncDef { FuncDef::ManyArgs(Sexp::TypeInstance, 2), FuncDef::NoArgs() }
  );

  // Conversion operators

  symbols.PutSymbolFunction(
    "type", 
    {"(type value) -> type"},
    "get the type of value",
    {{"(type 42)", "int"}},
    StdLib::TypeFunc, 
    FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Function::TypeInstance) }
  );

  symbols.PutSymbolFunction(
    "atom?", 
    {"(atom? value) -> bool"},
    "is value not a list?",
    {{"(atom? 42)", "true"}},
    StdLib::TypeQFunc, 
    FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "bool?", 
    {"(bool? value) -> bool"},
    "is value a bool?",
    {{"(bool? 42)", "false"}},
    StdLib::TypeQFunc, 
    FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "int?", 
    {"(int? value) -> bool"},
    "is value an int?",
    {{"(int? 42)", "true"}},
    StdLib::TypeQFunc, 
    FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "float?", 
    {"(float? value) -> bool"},
    "is value a float?",
    {{"(float? 42)", "false"}},
    StdLib::TypeQFunc, 
    FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "str?", 
    {"(str? value) -> bool"},
    "is value a str?",
    {{"(str? 42)", "false"}},
    StdLib::TypeQFunc, 
    FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "symbol?", 
    {"(symbol? value) -> bool"},
    "is value a symbol?",
    {{"(symbol? 42)", "false"}},
    StdLib::TypeQFunc, 
    FuncDef { FuncDef::OneArg(Symbol::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "list?", 
    {"(list? value) -> bool"},
    "is value a list?",
    {{"(list? 42)", "false"}},
    StdLib::TypeQFunc, 
    FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "fn?", 
    {"(fn? value) -> bool"},
    "is value a fn?",
    {{"(fn? 42)", "false"}},
    StdLib::TypeQFunc, 
    FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) }
  );

  symbols.PutSymbolFunction(
    "bool",
    {"(bool value) -> bool"},
    "convert value to bool",
    {{"(bool 42)", "true"}},
    StdLib::BoolFunc, 
    FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Bool::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "int", 
    {"(int value) -> int"},
    "convert value to int",
    {{"(int 42)", "42"}},
    StdLib::IntFunc, 
    FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Int::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "float", 
    {"(float value) -> float"},
    "convert value to float",
    {{"(float 42)", "42"}},
    StdLib::FloatFunc, 
    FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Float::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "str", 
    {"(str value) -> str"},
    "convert value to str",
    {{"(str 42)", "\"42\""}},
    StdLib::StrFunc, 
    FuncDef { FuncDef::OneArg(Literal::TypeInstance), FuncDef::OneArg(Str::TypeInstance) }
  );
  symbols.PutSymbolFunction(
    "symbol", 
    {"(symbol str) -> symbol"},
    "convert str to symbol",
    {{"(symbol \"foo\")", "foo"}},
    StdLib::SymbolFunc, 
    FuncDef { FuncDef::OneArg(Str::TypeInstance), FuncDef::OneArg(Symbol::TypeInstance) }
  );

  // Debug
  symbols.PutSymbolFunction(
    "breakpoint",
    {"(breakpoint) -> nil"},
    "Native breakpoint",
    {{"(breakpoint)", "nil"}},
    StdLib::Breakpoint,
    FuncDef { FuncDef::NoArgs(), FuncDef::NoArgs() }
  );

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
  return true;
}

void StdLib::UnLoad(Interpreter &interpreter) {
  //TODO
}

void StdLib::SetInteractiveMode(Interpreter &interpreter, bool enabled) {
  auto &settings = interpreter.GetSettings();
  if (enabled) {
    settings.PutDefaultFunction(CompiledFunction {
      SourceContext_,
      FuncDef { FuncDef::AnyArgs(), FuncDef::NoArgs() },
      &StdLib::DefaultFunction
    }); 
  }
  else {
    settings.PutDefaultFunction(CompiledFunction {
      SourceContext_,
      FuncDef { FuncDef::AnyArgs(Literal::TypeInstance), FuncDef::NoArgs() },
      [](EvaluationContext &ctx) {
        return ctx.ReturnNil();
      }
    }); 
  }
}

void AddCommandLineArgs(const SourceContext &sourceContext, SymbolTable &symbols, const string &name, const vector<string> &args) {
  ExpressionPtr argsExpr { new Sexp(sourceContext) };
  Sexp &argsList = static_cast<Sexp&>(*argsExpr);
  
  for (auto &arg : args)
    argsList.Args.emplace_back(new Str(sourceContext, arg));
  symbols.PutSymbol(name, ExpressionPtr { new Quote(sourceContext, move(argsExpr)) });
}

void StdLib::LoadEnvironment(SymbolTable &symbols, const Environment &env) {
  const SlispVersion &version = env.Version;
  symbols.PutSymbolStr("sys.version", version.ToString());
  symbols.PutSymbolInt("sys.versionNumber.major", version.Major);
  symbols.PutSymbolInt("sys.versionNumber.minor", version.Minor);
  symbols.PutSymbolInt("sys.versionNumber.subMinor", version.SubMinor);
  symbols.PutSymbolInt("sys.versionNumber.build", version.Build);
  symbols.PutSymbolStr("sys.program", env.Program); 
  symbols.PutSymbolStr("sys.script", env.Script);
  AddCommandLineArgs(SourceContext_, symbols, "sys.args", env.Args);
}

// Interpreter Functions

bool StdLib::EvaluateListSexp(EvaluationContext &ctx) {
  if (auto list = ctx.New<Sexp>()) {
    ArgListHelper::CopyTo(ctx.Args, list.Val.Args);
    if (ctx.Evaluate(list.Expr, "list")) {
      ctx.Args.clear();
      ctx.Args.push_back(move(list.Expr));
      return Display(ctx);
    }
  }
  return false;
}

bool StdLib::DefaultFunction(EvaluationContext &ctx) {
  if (ctx.Args.size() > 1)
    return EvaluateListSexp(ctx);
  return Display(ctx);
}

bool StdLib::Render(EvaluationContext &ctx, bool isDisplay) {
  ExpressionPtr curr;
  auto &cmdInterface = ctx.Interp.GetCommandInterface();
  int argNum = 1;
  while (!ctx.Args.empty()) {
    stringstream out;
    curr = move(ctx.Args.front());
    ctx.Args.pop_front();
    if (ctx.Evaluate(curr, argNum)) {
      if (isDisplay)
        out << *curr; 
      else
        curr->Print(out);
      out << endl;
      cmdInterface.WriteOutputLine(out.str());
    }
    else
      return false; 
    ++argNum;
  }

  return ctx.ReturnNil();
}

bool StdLib::Display(EvaluationContext &ctx) {
  return Render(ctx, true);
}

bool StdLib::Print(EvaluationContext &ctx) {
  return Render(ctx, false);
}

bool StdLib::Prompt(EvaluationContext &ctx) {
  string prefix; 
  if (!ctx.Args.empty()) {
    if (auto *prefixValue = ctx.GetRequiredValue<Str>(ctx.Args.front()))
      prefix = prefixValue->Value; 
    else
      return false;
  }

  auto &cmdInt = ctx.Interp.GetCommandInterface();
  string inputLine;

  bool oldInteractiveMode = false;
  cmdInt.GetInteractiveMode(oldInteractiveMode);
  cmdInt.SetInteractiveMode(true);
  bool result = cmdInt.ReadLine(prefix, inputLine);
  cmdInt.SetInteractiveMode(oldInteractiveMode);

  if (result)
    return ctx.ReturnNew<Str>(inputLine);
  else
    return ctx.Error("ReadLine failed");
}

bool StdLib::Quit(EvaluationContext &ctx) {
  ctx.Interp.Stop();
  return ctx.ReturnNil();
}

bool StdLib::Exit(EvaluationContext &ctx) {
  ExpressionPtr exitCodeValue { move(ctx.Args.front()) };
  ctx.Args.pop_front();
  if (auto *value = ctx.GetRequiredValue<Int>(exitCodeValue)) {
    ctx.Interp.SetExitCode(static_cast<int>(value->Value));
    return Quit(ctx);
  }
  else
    return false;
}

bool StdLib::Symbols(EvaluationContext &ctx) {
  if (auto sexp = ctx.New<Sexp>()) {
    ctx.Interp.GetDynamicSymbols(ctx.GetSourceContext()).ForEach([&ctx, &sexp](const std::string &symName, ExpressionPtr &expr) {
      sexp.Val.Args.emplace_back(ctx.Alloc<Str>(symName));
    });
    return ctx.ReturnNew<Quote>(move(sexp.Expr));
  }
  return false;
}

bool LookupSymbol(EvaluationContext &ctx, ExpressionPtr &currArg, string &symName, ExpressionPtr &symValue) {
  if (ctx.Evaluate(currArg, 1)) {
    if (auto *fn = TypeHelper::GetValue<Function>(currArg)) {
      if (auto *sym = TypeHelper::GetValue<Symbol>(fn->Symbol)) {
        symName = sym->Value;
        symValue = fn->Clone();
        return true;
      }
      else
        return ctx.Error("no symbol info for function");
    }
    else if (auto *str = TypeHelper::GetValue<Str>(currArg)) {
      symName = str->Value;
      if (ctx.GetSymbol(symName, symValue))
        return true;
      else
        return ctx.UnknownSymbolError(symName);
    }
    else
      return ctx.TypeError("symbol or str", currArg);
  }
  else
    return true;
}

bool StdLib::Help(EvaluationContext &ctx) {
  string defaultSexp = ctx.Interp.GetSettings().GetDefaultSexp();
  stringstream ss;
  bool fullHelp = false;
  Interpreter::SymbolFunctor functor = [&ss, &defaultSexp, &fullHelp](const string &symbolName, ExpressionPtr &expr) {
    if (symbolName != defaultSexp) {
      if (auto fn = TypeHelper::GetValue<Function>(expr)) {
        for (auto &sig : fn->Signatures)
          ss << sig << endl;
        if (fullHelp) {
          if (!fn->Doc.empty())
            ss << fn->Doc << endl;
          for (auto &ex : fn->Examples) {
            ss << "Example: " << ex.Code << " => " << ex.ExpectedValue << endl;
          }
          ss << endl;
        }
      }
    }
  };

  auto &symbols = ctx.Interp.GetDynamicSymbols(ctx.GetSourceContext());
  if (ctx.Args.empty()) {
    fullHelp = false;
    symbols.ForEach(functor);
  }
  else {
    fullHelp = true;
    while (!ctx.Args.empty()) {
      ExpressionPtr currArg = move(ctx.Args.front());
      ctx.Args.pop_front();

      string symName;
      ExpressionPtr symValue;
      if (LookupSymbol(ctx, currArg, symName, symValue) && symValue)
        functor(symName, symValue);
      else
        return false;
    }
  }

  ctx.Interp.GetCommandInterface().WriteOutputLine(ss.str());
  return ctx.ReturnNil();
}

template <class F>
bool HelpSubFunction(EvaluationContext &ctx, F functor) {
  string symName;
  ExpressionPtr symValue;
  if (LookupSymbol(ctx, ctx.Args.front(), symName, symValue)) {
    if (auto fn = TypeHelper::GetValue<Function>(symValue))
      return ctx.Return(functor(*fn));
  }
  return false;
}

bool StdLib::HelpSignatures(EvaluationContext &ctx) {
  return HelpSubFunction(ctx, [&ctx](Function &fn) { 
    if (auto list = ctx.New<Sexp>()) {
      for (auto &sig : fn.Signatures)
        list.Val.Args.emplace_back(ctx.Alloc<Str>(sig));
      return ctx.Alloc<Quote>(move(list.Expr)); 
    }
    return (Quote*)nullptr;
  });
}

bool StdLib::HelpDoc(EvaluationContext &ctx) {
  return HelpSubFunction(ctx, [&ctx](Function &fn) { 
    return ctx.Alloc<Str>(fn.Doc); 
  });
}

bool StdLib::HelpExamples(EvaluationContext &ctx) {
  return HelpSubFunction(ctx, [&ctx](Function &fn) { 
    if (auto list = ctx.New<Sexp>()) {
      for (auto &example : fn.Examples) {
        if (auto ex = ctx.New<Sexp>()) {
          ex.Val.Args.emplace_back(ctx.Alloc<Str>(example.Code));
          ex.Val.Args.emplace_back(ctx.Alloc<Str>(example.ExpectedValue));
          list.Val.Args.emplace_back(ctx.Alloc<Quote>(move(ex.Expr)));
        }
        else
          return (Quote*)nullptr;
      }
      return ctx.Alloc<Quote>(move(list.Expr));
    }
    else
      return (Quote*)nullptr;
  });
}

bool BuildOpSexp(EvaluationContext &ctx, const string &op, ExpressionPtr &symToSetExpr) {
  if (auto opExpr = ctx.New<Sexp>()) {
    opExpr.Val.Args.emplace_back(ctx.Alloc<Symbol>(op));
    opExpr.Val.Args.push_back(symToSetExpr->Clone());
    ArgListHelper::CopyTo(ctx.Args, opExpr.Val.Args);
    
    if (!ctx.Args.empty())
      ctx.Args.pop_front();
    ctx.Args.push_front(move(opExpr.Expr));
    return true;
  }
  else
    return false;
}

bool StdLib::Set(EvaluationContext &ctx) {
  ExpressionPtr symToSetExpr = move(ctx.Args.front());
  ctx.Args.pop_front();
  if (auto symToSet = ctx.GetRequiredValue<Symbol>(symToSetExpr)) { 
    string symToSetName = symToSet->Value;
    const string setOp = ctx.GetThisFunctionName();
    if (!setOp.empty()) {
      bool buildOpResult = true;
      if (setOp.length() > 1 && setOp.back() == '=') { 
        string op = setOp.substr(0, setOp.length() - 1);
        buildOpResult = BuildOpSexp(ctx, op, symToSetExpr);
      }
      else if (setOp == "++")
        buildOpResult = BuildOpSexp(ctx, "incr", symToSetExpr);
      else if (setOp == "--")
        buildOpResult = BuildOpSexp(ctx, "decr", symToSetExpr);

      if (!buildOpResult)
        return false;

      ExpressionPtr value = move(ctx.Args.front());
      ctx.Args.pop_front();
      if (ctx.Evaluate(value, "value")) {
        auto &currStackFrame = ctx.Interp.GetCurrentStackFrame();
        ExpressionPtr temp;
        bool ret = ctx.Return(value->Clone());
        value->SetSourceContext(symToSetExpr->GetSourceContext());
        if (currStackFrame.GetLocalSymbols().GetSymbol(symToSetName, temp))
          currStackFrame.PutLocalSymbol(symToSetName, move(value));
        else
          currStackFrame.PutDynamicSymbol(symToSetName, move(value));
        return ret;
      }
    }
    return false;
  }
  else
    return false;
}

bool StdLib::UnSet(EvaluationContext &ctx) {
  ExpressionPtr sym = move(ctx.Args.front());
  ctx.Args.pop_front();

  if (auto symE = ctx.GetRequiredValue<Symbol>(sym)) {
    string symName = symE->Value;

    ExpressionPtr value;
    auto &currFrame = ctx.Interp.GetCurrentStackFrame();
    if (currFrame.GetSymbol(symName, value)) {
      currFrame.DeleteSymbol(symName);
      return ctx.Return(value);
    }
    else
      return ctx.UnknownSymbolError(symName);
  }
  else
    return false;
}

bool StdLib::InfixRegistrationFunction(EvaluationContext &ctx, const string &name, bool unregister) {
  auto sym = static_cast<Symbol&>(*ctx.Args.front());
  ExpressionPtr fnExpr;
  if (ctx.GetSymbol(sym.Value, fnExpr)) {
    if (auto fn = ctx.GetRequiredValue<Function>(fnExpr)) { 
      auto &settings = ctx.Interp.GetSettings();
      if (unregister)
        settings.UnregisterInfixSymbol(sym.Value);
      else
        settings.RegisterInfixSymbol(sym.Value);
      return ctx.ReturnNil();
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

// IO Functions


bool StdLib::Exists(EvaluationContext &ctx) {
  if (auto filename = ctx.GetRequiredValue<Str>(ctx.Args.front()))
    return ctx.ReturnNew<Bool>(FileSystem().Exists(filename->Value));
  else
    return false;
}

bool StdLib::Delete(EvaluationContext &ctx) {
  if (auto filename = ctx.GetRequiredValue<Str>(ctx.Args.front())) 
    return ctx.ReturnNew<Bool>(FileSystem().Delete(filename->Value));
  else
    return false;
}

bool StdLib::ReadLines(EvaluationContext &ctx) {
  ExpressionPtr filenameArg = move(ctx.Args.front());
  ctx.Args.pop_front();
  if (auto filename = ctx.GetRequiredValue<Str>(filenameArg)) {
    FileSystem fs;
    FilePtr file = fs.Open(filename->Value, FileSystemInterface::Read);
    if (file) {
      if (auto lines = ctx.New<Sexp>()) {
        string currLine;
        while (file->ReadLine(currLine))
          lines.Val.Args.emplace_back(ctx.Alloc<Str>(currLine));
        return ctx.ReturnNew<Quote>(move(lines.Expr));
      }
      else
        return false;
    }
    else
      return ctx.Error("Could not open \"" + filename->Value + "\" for reading");
  }
  else
    return false;
}

bool StdLib::WriteLines(EvaluationContext &ctx) {
  ExpressionPtr filenameArg = move(ctx.Args.front());
  ctx.Args.pop_front();
  if (auto filename = ctx.GetRequiredValue<Str>(filenameArg)) {
    FileSystem fs;
    FilePtr file = fs.Open(filename->Value, FileSystemInterface::Write);
    if (file) {
      ExpressionPtr linesArgs = move(ctx.Args.front());
      ctx.Args.pop_front();
      if (auto lines = ctx.GetRequiredListValue(linesArgs)) {
        while (!lines->Args.empty()) {
          ExpressionPtr line = move(lines->Args.front());
          lines->Args.pop_front();
          if (auto lineValue = ctx.GetRequiredValue<Str>(line)) {
            if (!file->WriteLine(lineValue->Value))
              return ctx.Error("Failed to WriteLine");
          }
          else
            return false;
        }
        return ctx.ReturnNew<Bool>(true);
      }
      else
        return false;
    }
    else
      return ctx.Error("Could not open \"" + filename->Value + "\" for writing");
  }
  else
    return false;
}

// Generic Functions

template<class BeginIt, class EndIt>
bool ReverseGeneric(EvaluationContext &ctx, ExpressionPtr &arg, BeginIt beginIt, EndIt endIt) {
  reverse(beginIt, endIt);
  return ctx.Return(arg);
}

bool StdLib::Reverse(EvaluationContext &ctx) {
  ExpressionPtr arg = move(ctx.Args.front());
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
  ExpressionPtr itArg = move(ctx.Args.front());
  ctx.Args.pop_front();
  ExpressionPtr idxArg = move(ctx.Args.front());
  ctx.Args.pop_front();

  if (auto *idxInt = ctx.GetRequiredValue<Int>(idxArg)) {
    int64_t idx = idxInt->Value; 
    if (auto *str = dynamic_cast<Str*>(itArg.get())) {
      if (idx < 0)
        idx += str->Value.length();
      try {
        return ctx.ReturnNew<Str>(string(1, str->Value.at(static_cast<size_t>(idx))));
      }
      catch (out_of_range) {
        return ctx.Error("index " + to_string(idx) + " is out of bounds");
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
            if (currIdx == idx)
              return ctx.Return(curr->Clone());
          }
          ++currIdx;
        } while (more);
        return ctx.Error("index " + to_string(idx) + " is out of bounds");
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
    [&ctx](string &value)  { return ctx.Alloc<Int>(value.size()); },
    [&ctx](ArgList &value) { return ctx.Alloc<Int>(value.size()); }
  );
}

bool StdLib::EmptyQ(EvaluationContext &ctx) {
  return SequenceFn(ctx, 
    [&ctx](string &value)  { return ctx.Alloc<Bool>(value.empty()); },
    [&ctx](ArgList &value) { return ctx.Alloc<Bool>(value.empty()); }
  );
}

template <class S, class L>
bool StdLib::SequenceFn(EvaluationContext &ctx, S strFn, L listFn) {
  ExpressionPtr arg = move(ctx.Args.front());
  ctx.Args.clear();
  if (ctx.Evaluate(arg, 1)) {
    if (auto str = TypeHelper::GetValue<Str>(arg))
      return ctx.Return(strFn(str->Value));
    else if (auto quote = TypeHelper::GetValue<Quote>(arg)) {
      if (ctx.IsQuoteAList(*quote)) {
        if (auto listSexp = TypeHelper::GetValue<Sexp>(quote->Value))
          return ctx.Return(listFn(listSexp->Args));
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

// (foreach e lst (display e))
// (foreach e in lst (display e))
// (foreach lst display)
bool StdLib::Foreach(EvaluationContext &ctx) {
  auto firstArg = move(ctx.Args.front());
  ctx.Args.pop_front();
  auto secondArg = move(ctx.Args.front());
  ctx.Args.pop_front();

  if (auto firstSym = ctx.GetRequiredValue<Symbol>(firstArg)) {
    Symbol* currElementSym = nullptr; 
    ExpressionPtr iterableValueOrSym;
    Function *fn = nullptr;
    auto nRemainingArgs = ctx.Args.size();
    if (nRemainingArgs == 0) {
      iterableValueOrSym = move(firstArg);
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
            secondArg = move(ctx.Args.front());
            ctx.Args.pop_front();
          }
        }
      }
      iterableValueOrSym = move(secondArg);
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
            Scope scope(ctx.Interp.GetCurrentStackFrame().GetLocalSymbols(), ctx.GetSourceContext());
            scope.PutSymbol(currElementSym->Value, ExpressionPtr { ctx.Alloc<Ref>(curr) }); 
            ctx.Args.clear();
            ArgListHelper::CopyTo(bodyCopy, ctx.Args);
            if (!Begin(ctx))
              return false;
          }
          else {
            if (auto fnEval = ctx.New<Sexp>()) {
              fnEval.Val.Args.push_back(fn->Clone());
              fnEval.Val.Args.emplace_back(ctx.Alloc<Ref>(curr));
              if (ctx.Evaluate(fnEval.Expr, "function evaluation"))
                ctx.Return(fnEval.Expr);
              else
                return false;
            }
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
  stringstream ss;
  ss << "0x" << hex << *ctx.Args.front();
  return ctx.ReturnNew<Str>(ss.str());
}

bool StdLib::Bin(EvaluationContext &ctx) {
  stringstream ss;
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

  string s = ss.str();
  reverse(begin(s), end(s));

  return ctx.ReturnNew<Str>(s);
}

bool StdLib::Dec(EvaluationContext &ctx) {
  stringstream ss;
  ss << *ctx.Args.front();
  return ctx.ReturnNew<Str>(ss.str());
}

bool StdLib::PowInt(EvaluationContext &ctx) {
  return BinaryFunction<Int>(ctx, [](int64_t a, int64_t b) { return static_cast<int64_t>(pow(a, b)); });
}

bool StdLib::AbsInt(EvaluationContext &ctx) {
  return UnaryFunction<Int>(ctx, [](int64_t n) { return abs(n); });
}

bool StdLib::MaxInt(EvaluationContext &ctx) {
  return BinaryFunction<Int>(ctx, [](int64_t a, int64_t b) { return max(a, b); });
}

bool StdLib::MinInt(EvaluationContext &ctx) {
  return BinaryFunction<Int>(ctx, [](int64_t a, int64_t b) { return min(a, b); });
}

bool EvenOddHelper(EvaluationContext &ctx, bool isEven) {
  ExpressionPtr numExpr = ctx.Args.front()->Clone();
  if (auto num = ctx.GetRequiredValue<Int>(numExpr)) {
    if (num->Value < 0)
      return ctx.Error("expecting positive " + Int::TypeInstance.Name());
    return ctx.ReturnNew<Bool>((num->Value % 2) == (isEven ? 0 : 1));
  }
  else
    return false;
}

bool StdLib::Even(EvaluationContext &ctx) {
  return EvenOddHelper(ctx, true);
}

bool StdLib::Odd(EvaluationContext &ctx) {
  return EvenOddHelper(ctx, false);
}

bool StdLib::Zero(EvaluationContext &ctx) {
  return UnaryFunction<Int, Bool>(ctx, [](int64_t n) { return n == 0; });
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
  return UnaryFunction<Float>(ctx, [](double n) { return abs(n); });
}

bool StdLib::MaxFloat(EvaluationContext &ctx) {
  return BinaryFunction<Float>(ctx, [](double a, double b) { return max(a, b); });
}

bool StdLib::MinFloat(EvaluationContext &ctx) {
  return BinaryFunction<Float>(ctx, [](double a, double b) { return min(a, b); });
}

bool StdLib::PowFloat(EvaluationContext &ctx) {
  return BinaryFunction<Float>(ctx, [](double a, double b) { return pow(a, b); });
}

bool StdLib::Exp(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return exp(a); });
}

bool StdLib::Log(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return log(a); });
}

bool StdLib::Sqrt(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return sqrt(a); });
}

bool StdLib::Ceil(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return ceil(a); });
}

bool StdLib::Floor(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return floor(a); });
}

bool StdLib::Round(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return round(a); });
}

bool StdLib::Cos(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return cos(a); });
}

bool StdLib::Sin(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return sin(a); });
}

bool StdLib::Tan(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return tan(a); });
}

bool StdLib::ACos(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return acos(a); });
}

bool StdLib::ASin(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return asin(a); });
}

bool StdLib::ATan(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return atan(a); });
}

bool StdLib::ATan2(EvaluationContext &ctx) {
  return BinaryFunction<Float>(ctx, [](double a, double b) { return atan2(a, b); });
}

bool StdLib::Cosh(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return cosh(a); });
}

bool StdLib::Sinh(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return sinh(a); });
}

bool StdLib::Tanh(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return tanh(a); });
}

bool StdLib::ACosh(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return acosh(a); });
}

bool StdLib::ASinh(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return asinh(a); });
}

bool StdLib::ATanh(EvaluationContext &ctx) {
  return UnaryFunction<Float>(ctx, [](double a) { return atanh(a); });
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
  if (auto num = ctx.GetRequiredValue<Int>(ctx.Args.front()))
    return ctx.ReturnNew<Int>(~num->Value);
  return false;
}

// Str functions

bool StdLib::Trim(EvaluationContext &ctx) {
  ExpressionPtr firstArg = move(ctx.Args.front());
  ctx.Args.pop_front();
  if (auto str = ctx.GetRequiredValue<Str>(firstArg)) {
    if (auto result = ctx.New<Str>()) {
      string &resultValue = result.Val.Value;
      string &val = str->Value;
      size_t firstNonSpace = val.find_first_not_of(' ');
      size_t lastNonSpace = val.find_last_not_of(' ');

      if (firstNonSpace != string::npos) 
        resultValue.assign(val.begin() + firstNonSpace, val.begin() + lastNonSpace + 1);

      return ctx.Return(result.Expr);
    }
    else
      return false;
  }
  else
    return false;
}

template<typename F>
static bool CharTransform(EvaluationContext &ctx, F fn) {
  if (auto str = ctx.GetRequiredValue<Str>(ctx.Args.front())) {
    for_each(str->Value.begin(), str->Value.end(), [&fn](char &ch) { ch = fn(ch); });
    return ctx.Return(str->Clone());
  }
  else
    return false;
}

char CharToUpper(const char &ch) { return toupper(ch); }
char CharToLower(const char &ch) { return tolower(ch); }

bool StdLib::Upper(EvaluationContext &ctx) {
  return CharTransform(ctx, CharToUpper);
}

bool StdLib::Lower(EvaluationContext &ctx) {
  return CharTransform(ctx, CharToLower);
}

bool StdLib::SubStr(EvaluationContext &ctx) {
  auto strArg = move(ctx.Args.front());
  ctx.Args.pop_front();
  if (auto str = ctx.GetRequiredValue<Str>(strArg)) {
    auto startArg = move(ctx.Args.front());
    ctx.Args.pop_front();
    if (auto startIdx = ctx.GetRequiredValue<Int>(startArg)) {
      size_t count = string::npos;
      if (!ctx.Args.empty()) {
        auto countArg = move(ctx.Args.front());
        ctx.Args.pop_front();
        if (auto countValue = ctx.GetRequiredValue<Int>(countArg))
          count = static_cast<size_t>(countValue->Value);
        else
          return false;
      }

      try {
        return ctx.ReturnNew<Str>(str->Value.substr(static_cast<size_t>(startIdx->Value), count));
      }
      catch (out_of_range) {
        return ctx.Error("index " + to_string(startIdx->Value) + " is out of range");
      }
    }
    else
      return false;
  }
  else
    return false;
}

template<class F>
bool BinaryStrFunction(EvaluationContext &ctx, F fn) {
  auto arg1 = move(ctx.Args.front());
  ctx.Args.pop_front();
  auto arg2 = move(ctx.Args.front());
  ctx.Args.pop_front();
  if (auto str1 = ctx.GetRequiredValue<Str>(arg1)) {
    if (auto str2 = ctx.GetRequiredValue<Str>(arg2)) {
      return fn(str1->Value, str2->Value);
    }
    else
      return false;
  }
  else
    return false;
}

bool FindFunction(EvaluationContext &ctx, bool reverse) {
  return BinaryStrFunction(ctx, [&ctx, &reverse](const string &haystack, const string &needle) {
    size_t start = reverse ? string::npos : 0;
    if (!ctx.Args.empty()) {
      auto startArg = move(ctx.Args.front());
      ctx.Args.pop_front();
      if (auto startValue = ctx.GetRequiredValue<Int>(startArg))
        start = static_cast<size_t>(startValue->Value);
      else
        return false;
    }
    size_t idx = reverse ? haystack.rfind(needle, start) : haystack.find(needle, start);
    return ctx.ReturnNew<Int>((idx == string::npos) ? -1LL : idx);
  });
}

bool StdLib::Find(EvaluationContext &ctx) {
  return FindFunction(ctx, false);
}

bool StdLib::RFind(EvaluationContext &ctx) {
  return FindFunction(ctx, true);
}

bool StdLib::Compare(EvaluationContext &ctx) {
  return BinaryStrFunction(ctx, [&ctx](const string &haystack, const string &needle) {
    return ctx.ReturnNew<Int>(haystack.compare(needle));
  });
}

bool StdLib::Contains(EvaluationContext &ctx) {
  return BinaryStrFunction(ctx, [&ctx](const string &haystack, const string &needle) {
    return ctx.ReturnNew<Bool>(haystack.find(needle) != string::npos);
  });
}

bool StdLib::StartsWith(EvaluationContext &ctx) {
  return BinaryStrFunction(ctx, [&ctx](const string &haystack, const string &needle) {
    return ctx.ReturnNew<Bool>(haystack.find(needle) == 0);
  });
}

bool StdLib::EndsWith(EvaluationContext &ctx) {
  return BinaryStrFunction(ctx, [&ctx](const string &haystack, const string &needle) {
    size_t pos = haystack.rfind(needle); 
    return ctx.ReturnNew<Bool>(pos != string::npos && pos == (haystack.length() - needle.length()));
  });
}

bool StdLib::Replace(EvaluationContext &ctx) {
  return BinaryStrFunction(ctx, [&ctx](string &haystack, const string &needle) {
    string replacement = "";
    int64_t maxReplacements = numeric_limits<int64_t>::max();
    if (!ctx.Args.empty()) {
      auto replacementArg = move(ctx.Args.front());
      ctx.Args.pop_front();
      if (auto replacementValue = ctx.GetRequiredValue<Str>(replacementArg))
        replacement = replacementValue->Value;
      else
        return false;

      if (!ctx.Args.empty()) {
        auto maxReplacementsArg = move(ctx.Args.front());
        ctx.Args.pop_front();
        if (auto maxReplacementsValue = ctx.GetRequiredValue<Int>(maxReplacementsArg)) {
          if (maxReplacementsValue->Value != -1)
            maxReplacements = maxReplacementsValue->Value;
        }
        else
          return false;
      }
    }

    size_t offset = 0;
    size_t lastFind = 0;
    size_t needleCount = needle.length();
    size_t replacementCount = replacement.length();
    int64_t nReplacements = 0;
    while (nReplacements < maxReplacements) {
      lastFind = haystack.find(needle, offset);
      if (lastFind == string::npos)
        break;

      haystack.replace(lastFind, needleCount, replacement);
      offset = lastFind + replacementCount;
      ++nReplacements;
    } 

    return ctx.ReturnNew<Str>(haystack);
  });
}

bool StdLib::Split(EvaluationContext &ctx) {
  return BinaryStrFunction(ctx, [&ctx](const string &haystack, const string &needle) {
    bool flattenEmptyValues = true;
    if (!ctx.Args.empty()) {
      auto flattenArg = move(ctx.Args.front());
      if (auto flattenValue = ctx.GetRequiredValue<Bool>(flattenArg))
        flattenEmptyValues = flattenValue->Value;
      else
        return false;
    }

    if (auto result = ctx.New<Sexp>()) {
      if (!haystack.empty()) {
        if (!needle.empty()) {
          size_t offset = 0;
          size_t lastFind = 0;
          size_t needleLength = needle.length();
          bool more = true;
          while (more) {
            lastFind = haystack.find(needle, offset);
            if (lastFind == string::npos) {
              lastFind = haystack.length();
              more = false;
            }

            string newElem(haystack, offset, lastFind - offset);
            if (!flattenEmptyValues || !newElem.empty())
              result.Val.Args.emplace_back(ctx.Alloc<Str>(move(newElem)));
            offset = lastFind + needle.length();
          } 
        }
        else
          result.Val.Args.emplace_back(ctx.Alloc<Str>(haystack));
      }

      return ctx.ReturnNew<Quote>(move(result.Expr));
    }
    else
      return false;
  });
}

bool StdLib::Join(EvaluationContext &ctx) {
  auto listArg = move(ctx.Args.front());
  ctx.Args.pop_front();
  if (auto list = ctx.GetRequiredListValue(listArg)) {
    auto delimArg = move(ctx.Args.front());
    ctx.Args.pop_front();
    if (auto delim = ctx.GetRequiredValue<Str>(delimArg)) {
      bool flattenEmptyValues = true;
      if (!ctx.Args.empty()) {
        auto flattenArg = move(ctx.Args.front());
        ctx.Args.pop_front();
        if (auto flatten = ctx.GetRequiredValue<Bool>(flattenArg))
          flattenEmptyValues = flatten->Value;
        else
          return false;
      }

      stringstream ss;
      size_t i = 0;
      for (auto &elemExpr : list->Args) {
        if (auto elem = TypeHelper::GetValue<Str>(elemExpr)) {
          if (!flattenEmptyValues || !elem->Value.empty()) {
            if (i)
              ss << delim->Value;
            ss << elem->Value;
          }
        }
        else
          return ctx.Error("Element " + to_string(i) + " is not a " + Str::TypeInstance.Name());
        ++i;
      }

      return ctx.ReturnNew<Str>(ss.str());
    }
    else
      return false;
  }
  else
     return false;
}

bool FormatSpecifier(EvaluationContext &ctx, stringstream &ss, const string &pattern, vector<ExpressionPtr> &formatValues, size_t &lastOpenCurly, size_t &lastCloseCurly, size_t &lastFormatValueIdx) {
  lastCloseCurly = pattern.find("}", lastOpenCurly);
  if (lastCloseCurly == string::npos)
    return ctx.Error("pattern has matched {");

  string specifier = "";
  if ((lastCloseCurly - lastOpenCurly) > 1)
    specifier.assign(pattern, lastOpenCurly + 1, (lastCloseCurly - lastOpenCurly - 1));

  size_t formatValueIdx = -1;
  ExpressionPtr formatValue;
  if (specifier.empty()) {
    formatValueIdx = lastFormatValueIdx;
    ++lastFormatValueIdx; 
  }
  else if (isdigit(specifier[0])) {
    formatValueIdx = atoi(specifier.c_str());
  }

  if (formatValueIdx != -1) {
    if (formatValueIdx >= formatValues.size()) 
      return ctx.Error("format value index " + to_string(formatValueIdx) + " is out of range");
    formatValue = formatValues[formatValueIdx]->Clone();
  }
  else {
    if (!ctx.Interp.GetCurrentStackFrame().GetSymbol(specifier, formatValue))
      return ctx.Error("format specifier \"" + specifier + "\" not found");
  }

  formatValue->Print(ss);
  return true;
}

bool HandleRemainingCloseCurlies(EvaluationContext &ctx, stringstream &ss, const string &pattern, size_t &offset, size_t &lastCloseCurly, size_t &lastCharIdx) {
  do {
    lastCloseCurly = pattern.find("}", offset);
    if (lastCloseCurly == string::npos)
      break;

    if (lastCloseCurly == lastCharIdx || pattern[lastCloseCurly + 1] != '}')
      return ctx.Error("pattern has unmatched }");

    ss << pattern.substr(offset, (lastCloseCurly - offset) + 1);
    offset = lastCloseCurly + 2;
  } while (lastCloseCurly != string::npos);

  return true;
}

bool StdLib::Format(EvaluationContext &ctx) {
  auto patternArg = move(ctx.Args.front());
  ctx.Args.pop_front();
  if (auto patternValue = ctx.GetRequiredValue<Str>(patternArg)) {
    vector<ExpressionPtr> formatValues;
    string &pattern = patternValue->Value;
    stringstream ss;
    size_t offset = 0;
    size_t lastOpenCurly = string::npos;
    size_t lastCloseCurly = string::npos;
    size_t lastCharIdx = pattern.length() - 1;
    size_t lastFormatValueIdx = 0;
    bool more = true;

    while (!ctx.Args.empty()) {
      formatValues.push_back(move(ctx.Args.front()));
      ctx.Args.pop_front();
    }

    while (more) {
      lastOpenCurly = pattern.find("{", offset);
      if (lastOpenCurly == string::npos) {
        if (!HandleRemainingCloseCurlies(ctx, ss, pattern, offset, lastCloseCurly, lastCharIdx))
          return false;
        lastOpenCurly = lastCharIdx + 1;
        more = false;
      }
      else if (lastOpenCurly == lastCharIdx)
        return ctx.Error("pattern has unmatched {");

      ss << pattern.substr(offset, lastOpenCurly - offset);
      if (more) {
        char afterCurly = pattern[lastOpenCurly + 1];
        if (afterCurly == '{') {
          ss << "{";
          offset = lastOpenCurly + 2;
        }
        else {
          if (!FormatSpecifier(ctx, ss, pattern, formatValues, lastOpenCurly, lastCloseCurly, lastFormatValueIdx))
            return false;
          offset = lastCloseCurly + 1;
        }
      }
    }

    return ctx.ReturnNew<Str>(ss.str());
  }
  else
    return false;
}

bool StdLib::AddStr(EvaluationContext &ctx) {
  stringstream ss;
  while (!ctx.Args.empty()) {
    if (auto str = ctx.GetRequiredValue<Str>(ctx.Args.front()))
      ss << str->Value;
    else
      return false;
    ctx.Args.pop_front();
  }
  return ctx.ReturnNew<Str>(ss.str());
}

// Lists

bool StdLib::List(EvaluationContext &ctx) {
  if (auto list = ctx.New<Sexp>()) {
    int argNum = 1;
    while (!ctx.Args.empty()) {
      ExpressionPtr arg = move(ctx.Args.front());
      ctx.Args.pop_front();
      if (ctx.Evaluate(arg, argNum))
        list.Val.Args.push_back(move(arg));
      else
        return false; 
      ++argNum;
    }
    return ctx.ReturnNew<Quote>(move(list.Expr));
  }
  else
    return false;
}

// TODO: Refactor
bool StdLib::TransformList(EvaluationContext &ctx, ListTransforms transform) {
  ExpressionPtr fnExpr { move(ctx.Args.front()) };
  ctx.Args.pop_front();

  ExpressionPtr listExpr { move(ctx.Args.front()) };
  ctx.Args.pop_front();

  ExpressionPtr resultExpr { };
  if (transform == ListTransforms::Map ||
      transform == ListTransforms::Filter ||
      transform == ListTransforms::Take ||
      transform == ListTransforms::Skip)
    resultExpr.reset(ctx.Alloc<Sexp>());
  else if (transform == ListTransforms::Any)
    resultExpr.reset(ctx.Alloc<Bool>(false));
  else if (transform == ListTransforms::All)
    resultExpr.reset(ctx.Alloc<Bool>(true));

  auto resultList = static_cast<Sexp*>(resultExpr.get());

  int i = -1;
  if (auto fn = ctx.GetRequiredValue<Function>(fnExpr)) {
    if (auto list = ctx.GetRequiredListValue(listExpr)) {
      if ((transform == ListTransforms::Reduce ||
           transform == ListTransforms::Any ||
           transform == ListTransforms::All)
          && list->Args.empty())
        return ctx.Error("empty list not allowed");

      while (!list->Args.empty()) {
        ++i;
        ExpressionPtr item = move(list->Args.front());
        list->Args.pop_front();

        auto eval = ctx.New<Sexp>();
        if (!eval)
          return false;

        eval.Val.Args.push_back(fn->Clone());

        if (transform == ListTransforms::Reduce) {
          if (resultExpr)
            eval.Val.Args.push_back(resultExpr->Clone());
          else {
            resultExpr = item->Clone();
            continue;
          }
        }

        eval.Val.Args.push_back(item->Clone());
        if (!ctx.EvaluateNoError(eval.Expr))
          return ctx.Error("Failed to call " +  fn->ToString() + " on item " + to_string(i));

        if (transform == ListTransforms::Map)
          resultList->Args.push_back(move(eval.Expr));
        else if (transform == ListTransforms::Filter ||
                 transform == ListTransforms::Any ||
                 transform == ListTransforms::All ||
                 transform == ListTransforms::Take ||
                 transform == ListTransforms::Skip) {
          if (auto predResult = ctx.GetRequiredValue<Bool>(eval.Expr)) {
            if (transform == ListTransforms::Filter || transform == ListTransforms::Take) {
              if (predResult->Value)
                resultList->Args.push_back(item->Clone());
              else if (transform == ListTransforms::Take) 
                return ctx.ReturnNew<Quote>(move(resultExpr));
            }
            else if (transform == ListTransforms::Skip) {
              if (!predResult->Value) {
                resultList->Args.push_back(item->Clone());
                ArgListHelper::CopyTo(list->Args, resultList->Args);
                return ctx.ReturnNew<Quote>(move(resultExpr));
              }
            }
            else if (transform == ListTransforms::Any) {
              if (predResult->Value) 
                return ctx.ReturnNew<Bool>(true);
            }
            else if (transform == ListTransforms::All) {
              if (!predResult->Value)
                return ctx.ReturnNew<Bool>(false);
            }
          }
          else
            return false;
        }
        else if (transform == ListTransforms::Reduce)
          resultExpr = move(eval.Expr);
      }
    }
    else 
      return false;
  }
  else
    return false;

  return ctx.ReturnNew<Quote>(move(resultExpr));
}

bool StdLib::Map(EvaluationContext &ctx) {
  return TransformList(ctx, ListTransforms::Map);
}

bool StdLib::Filter(EvaluationContext &ctx) {
  return TransformList(ctx, ListTransforms::Filter);
}

bool StdLib::Reduce(EvaluationContext &ctx) {
  return TransformList(ctx, ListTransforms::Reduce);
}

bool StdLib::Any(EvaluationContext &ctx) {
  return TransformList(ctx, ListTransforms::Any);
}

bool StdLib::All(EvaluationContext &ctx) {
  return TransformList(ctx, ListTransforms::All);
}

bool StdLib::TakeSkip(EvaluationContext &ctx, bool isTake) {
  ExpressionPtr &firstArg = ctx.Args.front();
  if (ctx.Evaluate(firstArg, 1)) {
    if (TypeHelper::IsA<Int>(firstArg)) {
      ExpressionPtr countExpr = move(ctx.Args.front());
      ctx.Args.pop_front();
      Int &count = static_cast<Int&>(*countExpr);
      if (count.Value < 0)
        return ctx.Error("count cannot be < 0");

      ExpressionPtr listExpr = move(ctx.Args.front());
      if (auto list = ctx.GetRequiredListValue(listExpr)) {
        if (auto newList = ctx.New<Sexp>()) {
          if (isTake) {
            while (!list->Args.empty() && count.Value--) {
              newList.Val.Args.push_back(move(list->Args.front()));
              list->Args.pop_front();
            }
          }
          else {
            while (!list->Args.empty() && count.Value--)
              list->Args.pop_front();
            ArgListHelper::CopyTo(list->Args, newList.Val.Args);
          }
          return ctx.ReturnNew<Quote>(move(newList.Expr));
        }
        else
          return false;
      }
      else
        return false;
    }
    else if (TypeHelper::IsA<Function>(ctx.Args.front()))
      return TransformList(ctx, isTake ? ListTransforms::Take : ListTransforms::Skip);
    else
      return ctx.TypeError("int/function", firstArg);
  }
  else
    return false;
}

bool StdLib::Take(EvaluationContext &ctx) {
  return TakeSkip(ctx, true);
}

bool StdLib::Skip(EvaluationContext &ctx) {
  return TakeSkip(ctx, false);
}

bool StdLib::Zip(EvaluationContext &ctx) {
  auto result = ctx.New<Sexp>();
  if (!result)
    return false;
  
  int ctxArgNum = 1;
  bool evalArg = true;
  ExpressionPtr fnArg;
  if (ctx.Evaluate(ctx.Args.front(), ctxArgNum)) {
    if (TypeHelper::IsA<Function>(ctx.Args.front())) {
      fnArg = move(ctx.Args.front());
      ctx.Args.pop_front();
    }
    else {
      fnArg.reset(ctx.Alloc<Symbol>("list"));
      evalArg = false; // don't eval first arg twice
    }
  }
  else
    return false;
  
  vector<Sexp*> lists;
  for (auto &listArg : ctx.Args) {
    if (evalArg) {
      if (!ctx.Evaluate(listArg, ctxArgNum++))
        return false;
    }
    evalArg = true;

    if (auto listValue = ctx.GetRequiredListValue(listArg))
      lists.push_back(listValue);
    else
      return false;
  }

  if (lists.empty())
    return ctx.Error("expected at least one list");

  int elementNum = 1;
  bool more = true;
  while (more) {
    auto eval = ctx.New<Sexp>();
    if (!eval)
      return false;

    eval.Val.Args.push_back(fnArg->Clone());

    int listNum = 1;
    for (auto &list : lists) {
      if (listNum > 1) {
        if (list->Args.empty() == more)
          return ctx.Error("list " + to_string(listNum) + " has a different length than list 1");
      }
      else
        more = !list->Args.empty();

      if (more) {
        eval.Val.Args.push_back(move(list->Args.front()));
        list->Args.pop_front();
      }
      ++listNum;
    }

    if (more) {
      if (ctx.Evaluate(eval.Expr, "element " + to_string(elementNum++)))
        result.Val.Args.emplace_back(move(eval.Expr));
      else
        return false;
    }
  }

  return ctx.ReturnNew<Quote>(move(result.Expr));
}

// const symbols!!
Sexp* GetSexpFromListExpr(EvaluationContext &ctx, ExpressionPtr &listExpr) {
  if (auto *sym = ctx.GetRequiredValue<Symbol>(listExpr)) {
    if (sym->Value != "nil") {
      Expression *value = nullptr;
      if (ctx.GetSymbol(sym->Value, value) && value) {
        if (auto *quotedValue = dynamic_cast<Quote*>(value)) {
          if (quotedValue->Value) {
            if (auto *sexp = dynamic_cast<Sexp*>(quotedValue->Value.get()))
              return sexp;
          }
        }
        ctx.TypeError("list", listExpr);
      }
      else
        ctx.UnknownSymbolError(sym->Value);
    }
    else
      ctx.Error("can't modify a constant symbol");
  }
  return nullptr;
}

template <class F>
bool PerformListOp(EvaluationContext &ctx, ExpressionPtr &listExpr, const string &thisFnName, F fn) {
  bool inplace = thisFnName.back() == '!';
  Sexp *list = inplace ? GetSexpFromListExpr(ctx, listExpr) : ctx.GetRequiredListValue(listExpr);
  if (list) {
    if (fn(list->Args, thisFnName)) {
      if (inplace)
        return ctx.ReturnNil();
      else
        return ctx.Return(listExpr);
    }
    else
      return ctx.Error("Internal error: unknown function");
  }
  return false;
}

bool StdLib::Push(EvaluationContext &ctx) {
  auto arg1 = move(ctx.Args.front());
  ctx.Args.pop_front();
  auto arg2 = move(ctx.Args.front());
  ctx.Args.pop_front();

  ExpressionPtr listExpr;
  ExpressionPtr itemExpr;
  string thisFnName = ctx.GetThisFunctionName();
  if (thisFnName == "cons") {
    listExpr = move(arg2);
    itemExpr = move(arg1);
  }
  else {
    listExpr = move(arg1);
    itemExpr = move(arg2);
  }

  return PerformListOp(ctx, listExpr, thisFnName, [&itemExpr](ArgList &list, const string &fnName) {
    if (fnName == "cons" || (fnName.find("push-front") != string::npos))
      list.push_front(move(itemExpr));
    else if (fnName.find("push-back") != string::npos)
      list.push_back(move(itemExpr));
    else
      return false;
    return true;
  });
}

bool StdLib::Pop(EvaluationContext &ctx) {
  auto listExpr = move(ctx.Args.front());
  ctx.Args.pop_front();
  string thisFnName = ctx.GetThisFunctionName();
  return PerformListOp(ctx, listExpr, thisFnName, [](ArgList &list, const string &fnName) {
    if (!list.empty()) {
      if (fnName.find("pop-front") != string::npos)
        list.pop_front();
      else if (fnName.find("pop-back") != string::npos)
        list.pop_back();
      else
        return false; 
    }
    return true;
  });
}

bool StdLib::AddList(EvaluationContext &ctx) {
  if (auto result = ctx.New<Sexp>()) {
    while (!ctx.Args.empty()) {
      if (auto list = ctx.GetRequiredListValue(ctx.Args.front())) {
        while (!list->Args.empty()) {
          result.Val.Args.push_back(move(list->Args.front()));
          list->Args.pop_front();
        }
      }
      else
        return false;
      ctx.Args.pop_front();
    }
    return ctx.ReturnNew<Quote>(move(result.Expr));
  }
  else
    return false;
}

bool StdLib::Head(EvaluationContext &ctx) {
  ExpressionPtr seqArg { move(ctx.Args.front()) };
  if (auto str = TypeHelper::GetValue<Str>(seqArg)) {
    if (str->Value.empty())
      return ctx.ReturnNew<Str>();
    else 
      return ctx.ReturnNew<Str>(string(1, str->Value[0]));
  }
  else if (auto list = ctx.GetRequiredListValue(seqArg)) {
    if (list->Args.empty())
      return ctx.ReturnNil();
    else {
      bool ret = ctx.Return(move(list->Args.front()));
      list->Args.pop_front();
      return ret;
    }
  }
  else
    return false;
}

bool StdLib::Tail(EvaluationContext &ctx) {
  ExpressionPtr seqArg { move(ctx.Args.front()) };
  if (auto str = TypeHelper::GetValue<Str>(seqArg)) {
    if (str->Value.empty())
      return ctx.ReturnNew<Str>();
    else
      return ctx.ReturnNew<Str>(str->Value.substr(1));
  }
  else if (auto list = ctx.GetRequiredListValue(seqArg)) {
    if (list->Args.empty())
      return ctx.ReturnNil();
    else {
      list->Args.pop_front();
      if (auto newList = ctx.New<Sexp>()) {
        while (!list->Args.empty()) {
          newList.Val.Args.push_back(move(list->Args.front()));
          list->Args.pop_front();
        }
        return ctx.ReturnNew<Quote>(move(newList.Expr));
      }
      else
        return false;
    }
  }
  else
    return false;
}

bool StdLib::Last(EvaluationContext &ctx) {
  ExpressionPtr seqArg { move(ctx.Args.front()) };
  if (auto str = TypeHelper::GetValue<Str>(seqArg)) {
    if (str->Value.empty())
      return ctx.ReturnNew<Str>();
    else
      return ctx.ReturnNew<Str>(string(1, str->Value.back()));
  }
  else if (auto list = ctx.GetRequiredListValue(seqArg)) {
    if (list->Args.empty())
      return ctx.ReturnNil();
    else
      return ctx.Return(list->Args.back()->Clone());
  }
  else
    return false;
}

bool StdLib::Range(EvaluationContext &ctx) {
  ExpressionPtr startExpr = move(ctx.Args.front());
  ctx.Args.pop_front();
  ExpressionPtr endExpr = move(ctx.Args.front());
  ctx.Args.pop_front();

  int64_t step = 1;
  if (!ctx.Args.empty()) {
    ExpressionPtr stepExpr = move(ctx.Args.front());
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

      if (auto list = ctx.New<Sexp>()) {
        for (int64_t i = start; positiveStep ? (i <= end) : (i >= end); i += step)
          list.Val.Args.emplace_back(ctx.Alloc<Int>(i));
        return ctx.ReturnNew<Quote>(move(list.Expr));
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

// Logical

bool StdLib::BinaryLogicalFunc(EvaluationContext &ctx, bool isAnd) {
  int argNum = 1;
  while (!ctx.Args.empty()) {
    ExpressionPtr currArg = move(ctx.Args.front());
    ctx.Args.pop_front();
    if (ctx.Evaluate(currArg, argNum)) {
      if (auto argValue = ctx.GetRequiredValue<Bool>(currArg)) {
        bool value = argValue->Value;
        if (isAnd ? !value : value)
          return ctx.ReturnNew<Bool>(isAnd ? false : true);
      }
      else
        return false;
    }
    else
      return false; 

    ++argNum;
  }
  return ctx.ReturnNew<Bool>(isAnd ? true : false);
}

bool StdLib::And(EvaluationContext &ctx) {
  return BinaryLogicalFunc(ctx, true);
}

bool StdLib::Or(EvaluationContext &ctx) {
  return BinaryLogicalFunc(ctx, false);
}

bool StdLib::Not(EvaluationContext &ctx) {
  if (!ctx.Args.empty()) {
    ExpressionPtr arg = move(ctx.Args.front());
    ctx.Args.pop_front();
    if (ctx.Evaluate(arg, 1)) {
      if (auto boolArg = ctx.GetRequiredValue<Bool>(arg))
        return ctx.ReturnNew<Bool>(!boolArg->Value);
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

using ExpressionPredicate = function<bool(const Expression &, const Expression &)>;
bool ExpressionPredicateFn(EvaluationContext &ctx, ExpressionPredicate fn) {
  int argNum = 0;
  if (!ctx.Args.empty()) {
    ExpressionPtr firstArg = move(ctx.Args.front());
    ctx.Args.pop_front();
    ++argNum;
    if (ctx.Evaluate(firstArg, argNum)) {
      ExpressionPtr prevArg = move(firstArg);
      bool result = false;
      while (!ctx.Args.empty()) {
        ExpressionPtr currArg = move(ctx.Args.front());
        ctx.Args.pop_front();
        ++argNum;
        if (ctx.Evaluate(currArg, argNum)) {
          if (!fn(*prevArg, *currArg))
            return ctx.ReturnNew<Bool>(false);
        }
        else
          return false; 

        prevArg = move(currArg);
      }
      return ctx.ReturnNew<Bool>(true);
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
  return ctx.ReturnNew<Quote>(move(ctx.Args.front()));
}

bool StdLib::Unquote(EvaluationContext &ctx) {
  if (ctx.Evaluate(ctx.Args.front(), "1")) {
    ExpressionPtr quoteExpr { move(ctx.Args.front()) };
    ExpressionPtr toEvaluate;

    if (auto quote = TypeHelper::GetValue<Quote>(quoteExpr))
      toEvaluate = move(quote->Value);
    else 
      toEvaluate = move(quoteExpr);

    if (ctx.Evaluate(toEvaluate, "expression"))
      return ctx.Return(toEvaluate);
    else
      return false;
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
    ExpressionPtr arg = move(ctx.Args.front());
    ctx.Args.pop_front();
    if (auto argSexp = ctx.GetRequiredValue<Sexp>(arg)) {
      if (argSexp->Args.size() == 2) {
        ExpressionPtr boolExpr = move(argSexp->Args.front());
        argSexp->Args.pop_front();
        ExpressionPtr statementExpr = move(argSexp->Args.front());
        argSexp->Args.pop_front();
        if (ctx.Evaluate(boolExpr, "condition")) {
          if (auto boolResult = ctx.GetRequiredValue<Bool>(boolExpr)) {
            if (boolResult->Value) {
              if (ctx.Evaluate(statementExpr, "statement"))
                return ctx.Return(statementExpr);
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
        return ctx.Error("arg " + to_string(argNum) + ": expected 2 args. got " + to_string(argSexp->Args.size()));
    }
    else
      return false;
    ++argNum;
  }
  return ctx.ReturnNil();
}

// (switch (type x)                  #  (switch (type x)
//   (case int "x is an int")        #    (int "x is an int")
//   (case float "x is a float")     #    (float "x is a float")
//   (default "x is not a number"))  #    ("x is not a number"))

// TODO: needs refactoring!
bool StdLib::Switch(EvaluationContext &ctx) {
  ArgList argCopy;
  ArgListHelper::CopyTo(ctx.Args, argCopy);
  ExpressionPtr varExpr = move(argCopy.front());
  argCopy.pop_front();
  int argNum = 1;
  if (ctx.Evaluate(varExpr, "variable")) {
    while (!argCopy.empty()) {
      ExpressionPtr arg = move(argCopy.front());
      argCopy.pop_front();
      bool isDefault = argCopy.empty();
      if (auto argSexp = ctx.GetRequiredValue<Sexp>(arg)) {
        const string optionalSymbolName = isDefault ? "default" : "case";
        if (argSexp->Args.size() == (isDefault ? 2 : 3)) {
          ExpressionPtr optionalCaseSymbolExpr = move(argSexp->Args.front());
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
            ExpressionPtr statementExpr = move(argSexp->Args.front());
            argSexp->Args.pop_front();
            if (ctx.Evaluate(statementExpr, "statement"))
              return ctx.Return(statementExpr);
            else
              return false;
          }
          else
            return ctx.Error("arg " + to_string(argNum) + ": expected 1 args. got " + to_string(argSexp->Args.size()));
        }
        else {
          if (argSexp->Args.size() == 2) {
            ExpressionPtr valueExpr = move(argSexp->Args.front());
            argSexp->Args.pop_front();
            ExpressionPtr statementExpr = move(argSexp->Args.front());
            argSexp->Args.pop_front();
            ctx.Args.clear();
            ctx.Args.push_front(move(valueExpr));
            ctx.Args.push_front(varExpr->Clone());
            if (Eq(ctx)) {
              if (auto boolResult = ctx.GetRequiredValue<Bool>(ctx.Expr_)) {
                if (boolResult->Value) {
                  if (ctx.Evaluate(statementExpr, "statement"))
                    return ctx.Return(statementExpr);
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
            return ctx.Error("arg " + to_string(argNum) + ": expected 2 args. got " + to_string(argSexp->Args.size()));
        }
      }
      else
        return false;
      ++argNum;
    }
  }
  else
    return false;
  return ctx.ReturnNil();
}

bool StdLib::While(EvaluationContext &ctx) {
  ExpressionPtr lastStatementResult = List::GetNil(ctx.GetSourceContext());
  ArgList loopArgs;
  while (true) {
    loopArgs.clear();
    ArgListHelper::CopyTo(ctx.Args, loopArgs);
    ExpressionPtr condExpr = move(loopArgs.front());
    loopArgs.pop_front();
    if (ctx.Evaluate(condExpr, "condition")) {
      if (auto condResult = ctx.GetRequiredValue<Bool>(condExpr)) {
        if (condResult->Value) {
          int bodyStatementNum = 1;
          while (!loopArgs.empty()) {
            ExpressionPtr currBodyStatement = move(loopArgs.front());
            loopArgs.pop_front();
            if (ctx.Evaluate(currBodyStatement, "body" + to_string(bodyStatementNum)))
              lastStatementResult = move(currBodyStatement);
            else
              return false;
            ++bodyStatementNum;
          }
        }
        else 
          return ctx.Return(lastStatementResult);
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
  ExpressionPtr condExpr = move(ctx.Args.front());
  ctx.Args.pop_front();

  ExpressionPtr trueExpr = move(ctx.Args.front());
  ctx.Args.pop_front();
   
  ExpressionPtr falseExpr;
  if (!ctx.Args.empty()) {
    falseExpr = move(ctx.Args.front());
    ctx.Args.pop_front();
  }

  if (ctx.Evaluate(condExpr, "condition")) {
    if (auto cond = ctx.GetRequiredValue<Bool>(condExpr)) {
      ExpressionPtr branchExpr;
      if (cond->Value)
        branchExpr = move(trueExpr);
      else if (falseExpr)
        branchExpr = move(falseExpr);
      else 
        branchExpr = List::GetNil(ctx.GetSourceContext());

      if (ctx.Evaluate(branchExpr, "branch"))
        return ctx.Return(branchExpr);
      else
        return false; 
    }
    else
      return false;
  }
  else
    return false;
}

// TODO: Go through all the code and harden, perform additional argument checking
bool StdLib::Let(EvaluationContext &ctx) {
  Scope scope(ctx.Interp.GetCurrentStackFrame().GetLocalSymbols(), ctx.GetSourceContext());
  ExpressionPtr varsExpr = move(ctx.Args.front());
  ctx.Args.pop_front();
  if (auto vars = ctx.GetRequiredValue<Sexp>(varsExpr)) {
    for (auto &varExpr : vars->Args) {
      if (auto var = ctx.GetRequiredValue<Sexp>(varExpr, "(name1 value1)")) {
        size_t nVarArgs = var->Args.size();
        if (nVarArgs == 2) {
          ExpressionPtr varNameExpr = move(var->Args.front());
          var->Args.pop_front();

          ExpressionPtr varValueExpr = move(var->Args.front());
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
          return ctx.Error("Expected 2 args: (name1 value1). Got " + to_string(nVarArgs) + " args");
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
    currCodeExpr = move(ctx.Args.front());
    ctx.Args.pop_front();

    if (!ctx.Evaluate(currCodeExpr, "body"))
      return false;
  }
  return ctx.Return(currCodeExpr);
}

bool StdLib::Lambda(EvaluationContext &ctx) {
  ExpressionPtr formalsExpr = move(ctx.Args.front());
  ctx.Args.pop_front();

  ExpressionPtr codeExpr = move(ctx.Args.front());
  ctx.Args.pop_front();
  
  ArgList anonFuncArgs;
  int nArgs = 0;
  if (LambdaPrepareFormals(ctx, formalsExpr, anonFuncArgs, nArgs)) {
    auto func = ctx.New<InterpretedFunction>(
      FuncDef {
        FuncDef::ManyArgs(Literal::TypeInstance, nArgs),
        FuncDef::AnyArgs()
      },
      move(codeExpr),
      move(anonFuncArgs)
    );
    if (func) {
      auto &locals = ctx.Interp.GetCurrentStackFrame().GetLocalSymbols();
      locals.ForEach([&func](const string &name, ExpressionPtr &value) {
        func.Val.Closure.emplace(name, value->Clone());
      });
      return ctx.Return(func.Expr);
    }
    else
      return false;
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
  ExpressionPtr symbolExpr { move(ctx.Args.front()) };
  ctx.Args.pop_front();
  if (auto lambda = ctx.New<Sexp>()) {
    lambda.Val.Args.emplace_back(ctx.Alloc<Symbol>("lambda"));
    lambda.Val.Args.push_back(move(ctx.Args.front()));
    ctx.Args.pop_front();
    lambda.Val.Args.push_back(move(ctx.Args.front()));
    ctx.Args.pop_front();

    if (ctx.Evaluate(lambda.Expr, "lambdaExpr")) {
      if (auto set = ctx.New<Sexp>()) {
        set.Val.Args.emplace_back(ctx.Alloc<Symbol>("set"));
        set.Val.Args.push_back(move(symbolExpr));
        set.Val.Args.push_back(move(lambda.Expr)); 
        if (ctx.Evaluate(set.Expr, "setExpr"))
          return ctx.Return(set.Expr);
      }
      else
        return false;
    }
    return false; 
  }
  else
    return false;
}

bool StdLib::Apply(EvaluationContext &ctx) {
  auto application = ctx.New<Sexp>();
  if (!application)
    return false;

  application.Val.Args.push_back(move(ctx.Args.front()));
  ctx.Args.pop_front();

  ExpressionPtr appArgsExpr { move(ctx.Args.front()) };
  ctx.Args.pop_front();

  if (ctx.Evaluate(appArgsExpr, "argsList")) {
    ExpressionPtr newArgs;
    if (auto quote = TypeHelper::GetValue<Quote>(appArgsExpr))
      newArgs = move(quote->Value);
    else
      newArgs = move(appArgsExpr);

    if (auto appArgsSexp = ctx.GetRequiredValue<Sexp>(newArgs, "list")) {
      while (!appArgsSexp->Args.empty()) {
        application.Val.Args.push_back(move(appArgsSexp->Args.front()));
        appArgsSexp->Args.pop_front();
      }

      if (ctx.Evaluate(application.Expr, "functionApplication"))
        return ctx.Return(application.Expr);
      else
        return false; 
    }
    else
      return false;
  }
  else
    return false; 
}

bool StdLib::Error(EvaluationContext &ctx) {
  ExpressionPtr msgExpr { move(ctx.Args.front()) };
  if (auto *msgVal = ctx.GetRequiredValue<Str>(msgExpr))
    return ctx.Error(msgVal->Value);
  else
    return false;
}

bool StdLib::Try(EvaluationContext &ctx) {
  ExpressionPtr expr { move(ctx.Args.front()) };
  ctx.Args.pop_front();
  ExpressionPtr catchExpr { move(ctx.Args.front()) };
  ctx.Args.pop_front();
  if (ctx.EvaluateNoError(expr))
    return ctx.Return(expr);
  else {
    auto &errors = ctx.Interp.GetErrors();
    Scope scope(ctx.Interp.GetCurrentStackFrame().GetLocalSymbols(), ctx.GetSourceContext());
    scope.PutSymbol("$error.msg", ExpressionPtr { ctx.Alloc<Str>(errors.empty() ? "<unknown>" : errors.front().What) });

    if (auto stack = ctx.New<Sexp>()) {
      for (auto &frame : ctx.Interp.GetErrorStackTrace())
        stack.Val.Args.emplace_back(ctx.Alloc<Str>(frame));
      scope.PutSymbol("$error.stack", ExpressionPtr { ctx.Alloc<Quote>(move(stack.Expr)) });
      ctx.Interp.ClearErrors();
      if (ctx.Evaluate(catchExpr, "catch"))
        return ctx.Return(catchExpr);
      else
        return false;
    }
    else
      return false;
  }
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
      return ctx.ReturnNew<Bool>(value);
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
        value = llround(floatValue->Value);
      else if (auto strValue = TypeHelper::GetValue<Str>(expr)) {
        if (!strValue->Value.empty()) {
          try {
            NumConverter::Convert(strValue->Value, value);
          }
          catch (...) {
          }
        }
      }
      return ctx.ReturnNew<Int>(value);
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
      return ctx.ReturnNew<Float>(value);
    }
    else
      return false;
  }
  return false;
}

bool StdLib::StrFunc(EvaluationContext &ctx) {
  string value = "";
  if (auto &expr = ctx.Args.front()) {
    if (ctx.Evaluate(expr, 1))
      return ctx.ReturnNew<Str>(expr->ToString());
    else
      return false;
  }
  return false;
}

bool IsStrASymbol(const string &str) {
  return !str.empty() && 
         (str.find(" ") == string::npos) && 
         !isdigit(str[0]) &&
         (str[0] != '-' || str.length() == 1 || !isdigit(str[1]));
}

bool StdLib::SymbolFunc(EvaluationContext &ctx) {
  if (auto &expr = ctx.Args.front()) {
    if (ctx.Evaluate(expr, 1)) {
      if (auto *str = ctx.GetRequiredValue<Str>(expr)) {
        if (IsStrASymbol(str->Value))
          return ctx.ReturnNew<Symbol>(str->Value);
        else
          return ctx.Error("symbol can't be empty, contain a space or start with a digit");
      }
    }
  }
  return false;
}

bool StdLib::Breakpoint(EvaluationContext &ctx) {
#ifdef WIN32
  __debugbreak();
#else
  std::raise(SIGABRT);
#endif
  return ctx.ReturnNil();
}

bool StdLib::TypeFunc(EvaluationContext &ctx) {
  string typeName;
  if (auto quote = TypeHelper::GetValue<Quote>(ctx.Args.front())) {
    if (ctx.IsQuoteAList(*quote)) 
      typeName = "list";
    else
      typeName = quote->Type().Name();
  }
  else
    typeName = ctx.Args.front()->Type().Name();

  ExpressionPtr typeSymbol;
  if (ctx.GetSymbol(typeName, typeSymbol))
    return ctx.Return(typeSymbol);
  else
    return ctx.Error("unknown type");
}

bool StdLib::TypeQFunc(EvaluationContext &ctx) {
  string thisFuncName  = ctx.GetThisFunctionName();
  if (!thisFuncName.empty()) {
    if (thisFuncName.back() == '?') {
      thisFuncName.erase(thisFuncName.end() - 1);
      if (thisFuncName == "symbol") {
        if (auto *sym = ctx.GetRequiredValue<Symbol>(ctx.Args.front())) {
          Expression *value = nullptr;
          return ctx.ReturnNew<Bool>(ctx.GetSymbol(sym->Value, value));
        }
        else
          return false;
      }
      else {
        bool isAtomFunc = thisFuncName == "atom";
        if (auto type = ctx.New<Sexp>()) {
          type.Val.Args.emplace_back(ctx.Alloc<Symbol>("type"));
          type.Val.Args.push_back(ctx.Args.front()->Clone());
          ctx.Args.clear();
          ctx.Args.emplace_back(ctx.Alloc<Symbol>(isAtomFunc ? "list" : thisFuncName));
          ctx.Args.push_back(move(type.Expr));
          return isAtomFunc ? Ne(ctx) : Eq(ctx);
        }
      }
    }
  }
  return false; 
}

// Helpers

template <class T, class F>
bool StdLib::UnaryFunction(EvaluationContext &ctx, F fn) {
  return UnaryFunction<T, T, F>(ctx, fn);
}

template <class T, class R, class F>
bool StdLib::UnaryFunction(EvaluationContext &ctx, F fn) {
  ExpressionPtr numExpr = ctx.Args.front()->Clone();
  if (auto num = ctx.GetRequiredValue<T>(numExpr))
    return ctx.ReturnNew<R>(fn(num->Value));
  return false;
}

template<class T, class F>
bool StdLib::BinaryFunction(EvaluationContext &ctx, F fn) {
  bool first = true;
  T result { ctx.GetSourceContext(), 0 };
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

  return ctx.Return(result.Clone());
}

template <class B, class I, class F, class S>
bool StdLib::BinaryPredicate(EvaluationContext &ctx, B bFn, I iFn, F fFn, S sFn) {
  auto currArg = ctx.Args.begin();
  if (currArg != ctx.Args.end()) {
    if (ctx.Evaluate(*currArg, 1)) {
      Bool defaultValue { ctx.GetSourceContext(), true };
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
  R result(ctx.GetSourceContext(), defaultResult.Value);
  ExpressionPtr firstArg = move(ctx.Args.front());
  ctx.Args.pop_front();
  if (auto last = ctx.GetRequiredValue<T>(firstArg)) {
    int argNum = 1;
    while (!ctx.Args.empty()) {
      if (ctx.Evaluate(ctx.Args.front(), argNum)) {
        auto curr = TypeHelper::GetValue<T>(ctx.Args.front());
        if (curr) {
          R tmp(ctx.GetSourceContext(), fn(result, *last, *curr));
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

  return ctx.Return(result.Clone());
}
