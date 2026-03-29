#pragma once

#include "spine/ast.h"
#include "spine/lexer.h"

typedef struct {
    char* name;
    spValueType type;
} spSymbol;

typedef struct {
    spSymbol* symbols;
    size_t count;
    size_t capacity;
} spSymbolTable;

spProgramNode* parse(spTokenList list, const char* filename);