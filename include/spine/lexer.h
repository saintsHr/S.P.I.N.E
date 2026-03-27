#pragma once

#define SP_MAX_TOKEN_VALUE_SIZE 64

#include <stddef.h>

typedef enum {
    SP_TOKEN_TYPE_IDENTIFIER, // 0
    SP_TOKEN_TYPE_NUMBER,     // 1
    SP_TOKEN_TYPE_STRING,     // 2

    SP_TOKEN_TYPE_PLUS,   // 3
    SP_TOKEN_TYPE_MINUS,  // 4
    SP_TOKEN_TYPE_MULT,   // 5
    SP_TOKEN_TYPE_DIV,    // 6
    SP_TOKEN_TYPE_EQUALS, // 7

    SP_TOKEN_TYPE_SEMICOLON, // 8

    SP_TOKEN_TYPE_KW_I8,  // 9
    SP_TOKEN_TYPE_KW_I16, // 10
    SP_TOKEN_TYPE_KW_I32, // 11
    SP_TOKEN_TYPE_KW_I64, // 12

    SP_TOKEN_TYPE_KW_U8,  // 13
    SP_TOKEN_TYPE_KW_U16, // 14
    SP_TOKEN_TYPE_KW_U32, // 15
    SP_TOKEN_TYPE_KW_U64, // 16

    SP_TOKEN_TYPE_KW_F32, // 17
    SP_TOKEN_TYPE_KW_F64, // 18

    SP_TOKEN_TYPE_UNDEFINED, // 19
    SP_TOKEN_TYPE_EOF,       // 20
} spTokenType;

typedef struct {
    spTokenType type;
    char value[SP_MAX_TOKEN_VALUE_SIZE];
    int line;
    int column;
} spToken;

typedef struct {
    spToken* tokens;
    size_t count;
    size_t capacity;
} spTokenList;

spTokenList tokenize(const char* input, const char* filename);
void addToken(spTokenList* list, spToken token);