#include "spine/parser.h"
#include "spine/util/log.h"

#include <stdbool.h>

void initSymbolTable(spSymbolTable* table) {
    table->symbols = NULL;
    table->count = 0;
    table->capacity = 0;
}

void freeSymbolTable(spSymbolTable* table) {
    for (size_t i = 0; i < table->count; i++) free(table->symbols[i].name);
    free(table->symbols);
}

void addSymbol(spSymbolTable* table, const char* name, spValueType type) {
    if (table->count == table->capacity) {
        table->capacity = table->capacity == 0 ? 4 : table->capacity * 2;
        table->symbols = realloc(table->symbols, table->capacity * sizeof(spSymbol));
    }

    table->symbols[table->count].name = strdup(name);
    table->symbols[table->count].type = type;
    table->count++;
}

spValueType getSymbolType(spSymbolTable* table, const char* name) {
    for (size_t i = 0; i < table->count; i++) {
        if (strcmp(table->symbols[i].name, name) == 0) return table->symbols[i].type;
    }
    return -1;
}

static spToken peek(spTokenList list, size_t* current) {
    return list.tokens[*current];
}

static spToken advance(spTokenList list, size_t* current) {
    return list.tokens[(*current)++];
}

static bool match(spTokenList list, size_t* current, spTokenType type) {
    if (list.tokens[*current].type == type) {
        (*current)++;
        return true;
    }
    return false;
}

static void expect(spTokenList list, size_t* current, spTokenType type, const char* filename) {
    if (!match(list, current, type)) {
        spLogInfo l;
        l.code = SP_PARSER_UNEXPECTED_TOKEN;
        l.line = list.tokens[*current].line;
        l.col = list.tokens[*current].column;
        l.file = filename;
        l.sev = SP_SEV_FATAL;
        l.title = "Unexpected Token";
        l.desc = "Unexpected token '%s'.";
        l.hint = "Follow the language syntax.";
        spEmitLog(l, list.tokens[*current].value);
    }
}

static spASTNode* parse_declaration(spTokenList list, size_t* current, const char* filename, spSymbolTable* table) {
    // gets type
    spToken typeToken = advance(list, current);
    spValueType type;
    switch (typeToken.type) {
        case SP_TOKEN_TYPE_KW_F32: type = SP_VAL_TYPE_F32; break;
        case SP_TOKEN_TYPE_KW_F64: type = SP_VAL_TYPE_F64; break;
        case SP_TOKEN_TYPE_KW_I8:  type = SP_VAL_TYPE_I8;  break;
        case SP_TOKEN_TYPE_KW_I16: type = SP_VAL_TYPE_I16; break;
        case SP_TOKEN_TYPE_KW_I32: type = SP_VAL_TYPE_I32; break;
        case SP_TOKEN_TYPE_KW_I64: type = SP_VAL_TYPE_I64; break;
        case SP_TOKEN_TYPE_KW_U8:  type = SP_VAL_TYPE_U8;  break;
        case SP_TOKEN_TYPE_KW_U16: type = SP_VAL_TYPE_U16; break;
        case SP_TOKEN_TYPE_KW_U32: type = SP_VAL_TYPE_U32; break;
        case SP_TOKEN_TYPE_KW_U64: type = SP_VAL_TYPE_U64; break;
        default: 
            spLogInfo l = {0}; 
            l.code = SP_PARSER_UNEXPECTED_TOKEN;
            l.line = typeToken.line;
            l.col = typeToken.column;
            l.file = filename;
            l.sev = SP_SEV_FATAL;
            l.title = "Unexpected Token";
            l.desc = "Unexpected token, expected a type keyword.";
            l.hint = "Follow the language syntax.";
            spEmitLog(l, typeToken.value);
    }

    // gets identifier
    spToken nameToken = advance(list, current);
    char* name = nameToken.value;

    // gets value
    spASTNode* val = NULL;

    if (match(list, current, SP_TOKEN_TYPE_EQUALS)) {
        spToken valueToken = advance(list, current);
        switch(type) {
            case SP_VAL_TYPE_F32: val = (spASTNode*)spNewLiteralF32((float) atof(valueToken.value)); break;
            case SP_VAL_TYPE_F64: val = (spASTNode*)spNewLiteralF64((double)atof(valueToken.value)); break;

            case SP_VAL_TYPE_I8:  val = (spASTNode*)spNewLiteralI8 ((int8_t) strtoll(valueToken.value, NULL, 10)); break;
            case SP_VAL_TYPE_I16: val = (spASTNode*)spNewLiteralI16((int16_t)strtoll(valueToken.value, NULL, 10)); break;
            case SP_VAL_TYPE_I32: val = (spASTNode*)spNewLiteralI32((int32_t)strtoll(valueToken.value, NULL, 10)); break;
            case SP_VAL_TYPE_I64: val = (spASTNode*)spNewLiteralI64((int64_t)strtoll(valueToken.value, NULL, 10)); break;

            case SP_VAL_TYPE_U8:  val = (spASTNode*)spNewLiteralU8 ((uint8_t) strtoull(valueToken.value, NULL, 10)); break;
            case SP_VAL_TYPE_U16: val = (spASTNode*)spNewLiteralU16((uint16_t)strtoull(valueToken.value, NULL, 10)); break;
            case SP_VAL_TYPE_U32: val = (spASTNode*)spNewLiteralU32((uint32_t)strtoull(valueToken.value, NULL, 10)); break;
            case SP_VAL_TYPE_U64: val = (spASTNode*)spNewLiteralU64((uint64_t)strtoull(valueToken.value, NULL, 10)); break;
        }
    } else {
        // defaults
        switch(type) {
            case SP_VAL_TYPE_F32: val = (spASTNode*)spNewLiteralF32(0.0f); break;
            case SP_VAL_TYPE_F64: val = (spASTNode*)spNewLiteralF64(0.0);  break;

            case SP_VAL_TYPE_I8:  val = (spASTNode*)spNewLiteralI8 (0);    break;
            case SP_VAL_TYPE_I16: val = (spASTNode*)spNewLiteralI16(0);    break;
            case SP_VAL_TYPE_I32: val = (spASTNode*)spNewLiteralI32(0);    break;
            case SP_VAL_TYPE_I64: val = (spASTNode*)spNewLiteralI64(0);    break;

            case SP_VAL_TYPE_U8:  val = (spASTNode*)spNewLiteralU8 (0);    break;
            case SP_VAL_TYPE_U16: val = (spASTNode*)spNewLiteralU16(0);    break;
            case SP_VAL_TYPE_U32: val = (spASTNode*)spNewLiteralU32(0);    break;
            case SP_VAL_TYPE_U64: val = (spASTNode*)spNewLiteralU64(0);    break;
        }
    }

    // expect ;
    expect(list, current, SP_TOKEN_TYPE_SEMICOLON, filename);

    addSymbol(table, name, type);

    // creates & returns node
    return (spASTNode*)spNewVarDecl(name, type, val);
}

static spASTNode* parse_assign(spTokenList list, size_t* current, const char* filename, spValueType type) {
    // gets identifier
    spToken nameToken = advance(list, current);
    char* name = nameToken.value;

    // expects =
    expect(list, current, SP_TOKEN_TYPE_EQUALS, filename);

    // gets value
    spToken valueToken = advance(list, current);
    spASTNode* val = NULL;
    switch(type) {
        case SP_VAL_TYPE_F32: val = (spASTNode*)spNewLiteralF32((float)atof(valueToken.value)); break;
        case SP_VAL_TYPE_F64: val = (spASTNode*)spNewLiteralF64((double)atof(valueToken.value)); break;
        case SP_VAL_TYPE_I8:  val = (spASTNode*)spNewLiteralI8((int8_t)atoi(valueToken.value)); break;
        case SP_VAL_TYPE_I16: val = (spASTNode*)spNewLiteralI16((int16_t)atoi(valueToken.value)); break;
        case SP_VAL_TYPE_I32: val = (spASTNode*)spNewLiteralI32((int32_t)atoi(valueToken.value)); break;
        case SP_VAL_TYPE_I64: val = (spASTNode*)spNewLiteralI64((int64_t)atoll(valueToken.value)); break;
        case SP_VAL_TYPE_U8:  val = (spASTNode*)spNewLiteralU8((uint8_t)atoi(valueToken.value)); break;
        case SP_VAL_TYPE_U16: val = (spASTNode*)spNewLiteralU16((uint16_t)atoi(valueToken.value)); break;
        case SP_VAL_TYPE_U32: val = (spASTNode*)spNewLiteralU32((uint32_t)atoi(valueToken.value)); break;
        case SP_VAL_TYPE_U64: val = (spASTNode*)spNewLiteralU64((uint64_t)atoll(valueToken.value)); break;
    }

    // expect ;
    expect(list, current, SP_TOKEN_TYPE_SEMICOLON, filename);

    // creates & returns node
    return (spASTNode*)spNewAssign(name, val);
}

static bool is_type(spToken token) {
    if (token.type == SP_TOKEN_TYPE_KW_I8)  return true;
    if (token.type == SP_TOKEN_TYPE_KW_I16) return true;
    if (token.type == SP_TOKEN_TYPE_KW_I32) return true;
    if (token.type == SP_TOKEN_TYPE_KW_I64) return true;

    if (token.type == SP_TOKEN_TYPE_KW_U8)  return true;
    if (token.type == SP_TOKEN_TYPE_KW_U16) return true;
    if (token.type == SP_TOKEN_TYPE_KW_U32) return true;
    if (token.type == SP_TOKEN_TYPE_KW_U64) return true;

    if (token.type == SP_TOKEN_TYPE_KW_F32) return true;
    if (token.type == SP_TOKEN_TYPE_KW_F64) return true;

    return false;
}

static bool is_ident(spToken token) {
    if (token.type == SP_TOKEN_TYPE_IDENTIFIER)  return true;
    return false;
}

static spASTNode* parse_statement(spTokenList list, size_t* current, const char* filename, spSymbolTable* table) {
    spToken token = list.tokens[*current];

    if (is_type(token)) {
        spASTNode* decl = parse_declaration(list, current, filename, table);
        return decl;
    }

    if (is_ident(token)) {
        spValueType type = getSymbolType(table, token.value);
        if (type == -1) {
            spLogInfo l = {0};
            l.code = SP_PARSER_UNDECLARED_VARIABLE;
            l.line = token.line;
            l.col = token.column;
            l.file = filename;
            l.sev = SP_SEV_FATAL;
            l.title = "Undeclared Variable";
            l.desc = "Variable '%s' used before declaration.";
            l.hint = "Declare the variable before using it.";
            spEmitLog(l, token.value);
        }
        return parse_assign(list, current, filename, type);
    }

    return NULL;
}

spProgramNode* parse(spTokenList list, const char* filename) {
    spProgramNode* program = spNewProgram();
    spSymbolTable symTable;
    initSymbolTable(&symTable);

    size_t current = 0;
    while (list.tokens[current].type != SP_TOKEN_TYPE_EOF) {
        spASTNode* stmt = parse_statement(list, &current, filename, &symTable);
        spProgramAddStatement(program, stmt);
    }

    freeSymbolTable(&symTable);
    return program;
}