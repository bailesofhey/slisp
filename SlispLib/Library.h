#pragma once

#include "Interpreter.h"

class Library {
  public:
    virtual ~Library();
    virtual void Load(Interpreter &interpreter) = 0;
    virtual void UnLoad(Interpreter &interpreter) = 0;
    virtual void SetInteractiveMode(Interpreter &interpreter, bool enabled);
};