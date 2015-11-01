#pragma once

#include "ExpressionTreeVisitor.h"

class ExpressionTreeIterator {
  public:
    bool Iterate(Sexp *exprTree, ExpressionTreeVisitor &visitor);
};