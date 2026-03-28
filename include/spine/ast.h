#pragma once

#include <stdlib.h>
#include <stdint.h>

typedef enum {
    SP_NODE_PROGRAM,
    SP_NODE_VAR_DECL,

    SP_NODE_BINARY_EXPR,
    SP_NODE_LITERAL,
    SP_NODE_IDENTIFIER,
    SP_NODE_ASSIGN,
} spNodeType;

typedef enum {
    SP_VAL_TYPE_I8,
    SP_VAL_TYPE_I16,
    SP_VAL_TYPE_I32,
    SP_VAL_TYPE_I64,

    SP_VAL_TYPE_U8,
    SP_VAL_TYPE_U16,
    SP_VAL_TYPE_U32,
    SP_VAL_TYPE_U64,

    SP_VAL_TYPE_F32,
    SP_VAL_TYPE_F64,
} spValueType;

typedef enum {
    SP_OP_TYPE_ADD,
    SP_OP_TYPE_SUB,
    SP_OP_TYPE_DIV,
    SP_OP_TYPE_MUL,
} spOperationType;

typedef struct spASTNode {
    spNodeType type;
} spASTNode;

typedef struct {
    spASTNode base;
    spValueType value_type;

    union {
        int64_t  i64;
        uint64_t u64;
        double   f64;
    };

} spLiteralNode;

typedef struct {
    spASTNode base;
    char* name;
} spIdentifierNode;

typedef struct {
    spASTNode base;
    spASTNode* left;
    spASTNode* right;
    spOperationType op;
} spBinaryExprNode;

typedef struct {
    spASTNode base;
    char* name;
    spValueType var_type;
    spASTNode* value;
} spVarDeclNode;

typedef struct {
    spASTNode base;
    char* name;
    spASTNode* value;
} spAssignNode;

typedef struct {
    spASTNode base;
    spASTNode** statements;
    size_t statement_count;
    size_t statement_capacity;
} spProgramNode;

void spPrintAST(spASTNode* root);

spProgramNode* spNewProgram();
void spProgramAddStatement(spProgramNode* program, spASTNode* stmt);

spLiteralNode* spNewLiteralF64(double value);
spLiteralNode* spNewLiteralF32(float value);

spLiteralNode* spNewLiteralI64(int64_t value);
spLiteralNode* spNewLiteralI32(int32_t value);
spLiteralNode* spNewLiteralI16(int16_t value);
spLiteralNode* spNewLiteralI8(int8_t value);

spLiteralNode* spNewLiteralU64(uint64_t value);
spLiteralNode* spNewLiteralU32(uint32_t value);
spLiteralNode* spNewLiteralU16(uint16_t value);
spLiteralNode* spNewLiteralU8(uint8_t value);


spIdentifierNode* spNewIdentifier(const char* name);
spBinaryExprNode* spNewBinary(spASTNode* left, spASTNode* right, spOperationType op);
spVarDeclNode* spNewVarDecl(const char* name, spValueType type, spASTNode* value);
spAssignNode* spNewAssign(const char* name, spASTNode* value);

void spFreeAST(spASTNode* node);