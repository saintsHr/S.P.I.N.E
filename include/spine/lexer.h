#pragma once

#define SP_MAX_TOKEN_VALUE_SIZE 64

#include <stddef.h>

typedef enum {
    TOKEN_TYPE_IDENTIFIER,
    TOKEN_TYPE_NUMBER,
    TOKEN_TYPE_STRING,

    TOKEN_TYPE_PLUS,
    TOKEN_TYPE_MINUS,
    TOKEN_TYPE_MULT,
    TOKEN_TYPE_DIV,
    TOKEN_TYPE_EQUALS,

    TOKEN_TYPE_LPAREN,
    TOKEN_TYPE_RPAREN,
    TOKEN_TYPE_LBRACE,
    TOKEN_TYPE_RBRACE,
    TOKEN_TYPE_SEMICOLON,

    TOKEN_TYPE_UNDEFINED,
    TOKEN_TYPE_EOF,
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