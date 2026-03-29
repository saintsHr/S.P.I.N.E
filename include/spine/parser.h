#pragma once

#include "spine/ast.h"
#include "spine/lexer.h"

spProgramNode* parse(spTokenList list, const char* filename);