#include <iostream>

#include "ExpressionTreeDumper.h"
#include "ExpressionTreeVisitor.h"
#include "Expression.h"

using std::cout;
using std::endl;

//=============================================================================

 ExpressionTreeDumper::ExpressionTreeDumper(bool displayExpression):
  ExpressionTreeVisitor {},
  Depth { 0 },
  DisplayExpression { displayExpression }
 {
 }

bool ExpressionTreeDumper::OnExpression(Sexp *expr, States state) {
  if (state == ExpressionTreeVisitor::BEGIN) {
    FirstArg = true;
    cout << endl;
    for (int i = 0; i < Depth; ++i)
      cout << "  ";
    cout << "(";
    if (DisplayExpression)
      cout << expr->ToString();

    ++Depth;
  }
  else if (state == ExpressionTreeVisitor::END) {
    cout << ")";
    --Depth;
  }
  return true;
}

bool ExpressionTreeDumper::OnExpression(Number *expr, States state) {
  return DumpValue(expr);
}

bool ExpressionTreeDumper::OnExpression(String *expr, States state) {
  char wrapper = '"';
  return DumpValue(expr, &wrapper);
}

bool ExpressionTreeDumper::OnExpression(Symbol *expr, States state) {
  return DumpValue(expr);
}

template<class T>
bool ExpressionTreeDumper::DumpValue(T *expr, char *wrapper) {
  if (!FirstArg)
    cout << " ";
  if (DisplayExpression) {
    cout << expr->ToString();
  }
  else {
    if (wrapper)
      cout << *wrapper;
    cout << expr->Value;
    if (wrapper)
      cout << *wrapper;
  }
  FirstArg = false;
  return true;
}

