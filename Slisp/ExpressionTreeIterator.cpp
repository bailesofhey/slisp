#include "ExpressionTreeVisitor.h"
#include "ExpressionTreeIterator.h"

//=============================================================================

bool ExpressionTreeIterator::Iterate(Sexp *exprTree, ExpressionTreeVisitor &visitor) {
  bool ret = visitor.OnExpression(exprTree, ExpressionTreeVisitor::BEGIN);
  for (auto &arg : exprTree->Args) {
    if (ret) {
      if (Sexp *a = dynamic_cast<Sexp*>(arg.get()))
        ret = Iterate(a, visitor);
      else if (Number *a = dynamic_cast<Number*>(arg.get()))
        ret = visitor.OnExpression(a, ExpressionTreeVisitor::MIDDLE);
      else if (String *a = dynamic_cast<String*>(arg.get()))
        ret = visitor.OnExpression(a, ExpressionTreeVisitor::MIDDLE);
      else if (Symbol *a = dynamic_cast<Symbol*>(arg.get()))
        ret = visitor.OnExpression(a, ExpressionTreeVisitor::MIDDLE);
      else
        throw std::exception("unknown expression type");
    }
  }
  return ret && visitor.OnExpression(exprTree, ExpressionTreeVisitor::END);
};