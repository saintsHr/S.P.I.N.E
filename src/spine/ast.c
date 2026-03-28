#include "spine/ast.h"
#include "spine/util/log.h"

#include <string.h>
#include <stdio.h>

static void spPrintIndent(int indent) {
    for (int i = 0; i < indent; i++) printf("  ");
}

static const char* spOpToString(spOperationType op) {
    switch(op) {
        case SP_OP_TYPE_ADD: return "+";
        case SP_OP_TYPE_SUB: return "-";
        case SP_OP_TYPE_MUL: return "*";
        case SP_OP_TYPE_DIV: return "/";
        default: return "?";
    }
}

static const char* spTypeToString(spValueType type) {
    switch(type) {
        case SP_VAL_TYPE_I8:  return "i8";
        case SP_VAL_TYPE_I16: return "i16";
        case SP_VAL_TYPE_I32: return "i32";
        case SP_VAL_TYPE_I64: return "i64";

        case SP_VAL_TYPE_U8:  return "u8";
        case SP_VAL_TYPE_U16: return "u16";
        case SP_VAL_TYPE_U32: return "u32";
        case SP_VAL_TYPE_U64: return "u64";

        case SP_VAL_TYPE_F32: return "f32";
        case SP_VAL_TYPE_F64: return "f64";

        default: return "?";
    }
}

static void spPrintASTNode(spASTNode* node, int indent) {
    if (!node) return;

    switch (node->type) {

        case SP_NODE_PROGRAM: {
            spProgramNode* prog = (spProgramNode*)node;

            spPrintIndent(indent);
            printf("Program\n");

            for (size_t i = 0; i < prog->statement_count; i++) {
                spPrintASTNode(prog->statements[i], indent + 1);
            }
            break;
        }

        case SP_NODE_VAR_DECL: {
            spVarDeclNode* var = (spVarDeclNode*)node;

            spPrintIndent(indent);
            printf("VarDecl %s : %s\n", var->name, spTypeToString(var->var_type));

            spPrintASTNode(var->value, indent + 1);
            break;
        }

        case SP_NODE_ASSIGN: {
            spAssignNode* asg = (spAssignNode*)node;

            spPrintIndent(indent);
            printf("Assign %s\n", asg->name);

            spPrintASTNode(asg->value, indent + 1);
            break;
        }

        case SP_NODE_BINARY_EXPR: {
            spBinaryExprNode* bin = (spBinaryExprNode*)node;

            spPrintIndent(indent);
            printf("Binary %s\n", spOpToString(bin->op));

            spPrintASTNode(bin->left, indent + 1);
            spPrintASTNode(bin->right, indent + 1);
            break;
        }

        case SP_NODE_IDENTIFIER: {
            spIdentifierNode* id = (spIdentifierNode*)node;

            spPrintIndent(indent);
            printf("Identifier %s\n", id->name);
            break;
        }

        case SP_NODE_LITERAL: {
            spLiteralNode* lit = (spLiteralNode*)node;

            spPrintIndent(indent);
            printf("Literal ");

            switch (lit->value_type) {
                case SP_VAL_TYPE_F32:
                case SP_VAL_TYPE_F64:
                    printf("%f\n", lit->f64);
                    break;

                case SP_VAL_TYPE_I8:
                case SP_VAL_TYPE_I16:
                case SP_VAL_TYPE_I32:
                case SP_VAL_TYPE_I64:
                    printf("%lld\n", (long long)lit->i64);
                    break;

                case SP_VAL_TYPE_U8:
                case SP_VAL_TYPE_U16:
                case SP_VAL_TYPE_U32:
                case SP_VAL_TYPE_U64:
                    printf("%llu\n", (unsigned long long)lit->u64);
                    break;
            }
            break;
        }
    }
}

void spPrintAST(spASTNode* root) {
    spPrintASTNode(root, 0);
}

spProgramNode* spNewProgram() {
    spProgramNode* program = malloc(sizeof(spProgramNode));

    program->base.type = SP_NODE_PROGRAM;
    program->statements = NULL;
    program->statement_count = 0;
    program->statement_capacity = 0;

    return program;
}

void spProgramAddStatement(spProgramNode* program, spASTNode* stmt) {
    if (program->statement_count >= program->statement_capacity) {
        program->statement_capacity = program->statement_capacity == 0 ? 8 : program->statement_capacity * 2;
        
        spASTNode** newStatements = realloc(
            program->statements,
            program->statement_capacity * sizeof(spASTNode*)
        );

        if (!newStatements) {
            spLogInfo l;
            l.code = SP_AST_REALLOC_FAILED;
            l.col = 0;
            l.line = 0;
            l.sev = SP_SEV_FATAL;
            l.file = "N/A";
            l.title = "Allocation Failed";
            l.desc = "AST program memory reallocation failed.";
            l.hint = "Make sure you have enough memory and try again.";
            spEmitLog(l);
        }

        program->statements = newStatements;
    }

    program->statements[program->statement_count++] = stmt;
}

spIdentifierNode* spNewIdentifier(const char* name) {
    spIdentifierNode* node = malloc(sizeof(spIdentifierNode));
    node->base.type = SP_NODE_IDENTIFIER;
    node->name = strdup(name);
    return node;
}

spLiteralNode* spNewLiteralF64(double value) {
    spLiteralNode* node = malloc(sizeof(spLiteralNode));
    node->base.type = SP_NODE_LITERAL;
    node->value_type = SP_VAL_TYPE_F64;
    node->f64 = (double)value;
    return node;
}

spLiteralNode* spNewLiteralF32(float value) {
    spLiteralNode* node = malloc(sizeof(spLiteralNode));
    node->base.type = SP_NODE_LITERAL;
    node->value_type = SP_VAL_TYPE_F32;
    node->f64 = (double)value;
    return node;
}

spLiteralNode* spNewLiteralI64(int64_t value) {
    spLiteralNode* node = malloc(sizeof(spLiteralNode));
    node->base.type = SP_NODE_LITERAL;
    node->value_type = SP_VAL_TYPE_I64;
    node->i64 = (int64_t)value;
    return node;
}

spLiteralNode* spNewLiteralI32(int32_t value) {
    spLiteralNode* node = malloc(sizeof(spLiteralNode));
    node->base.type = SP_NODE_LITERAL;
    node->value_type = SP_VAL_TYPE_I32;
    node->i64 = (int64_t)value;
    return node;
}

spLiteralNode* spNewLiteralI16(int16_t value) {
    spLiteralNode* node = malloc(sizeof(spLiteralNode));
    node->base.type = SP_NODE_LITERAL;
    node->value_type = SP_VAL_TYPE_I16;
    node->i64 = (int64_t)value;
    return node;
}

spLiteralNode* spNewLiteralI8(int8_t value) {
    spLiteralNode* node = malloc(sizeof(spLiteralNode));
    node->base.type = SP_NODE_LITERAL;
    node->value_type = SP_VAL_TYPE_I8;
    node->i64 = (int64_t)value;
    return node;
}

spLiteralNode* spNewLiteralU64(uint64_t value) {
    spLiteralNode* node = malloc(sizeof(spLiteralNode));
    node->base.type = SP_NODE_LITERAL;
    node->value_type = SP_VAL_TYPE_U64;
    node->u64 = (uint64_t)value;
    return node;
}

spLiteralNode* spNewLiteralU32(uint32_t value) {
    spLiteralNode* node = malloc(sizeof(spLiteralNode));
    node->base.type = SP_NODE_LITERAL;
    node->value_type = SP_VAL_TYPE_U32;
    node->u64 = (uint64_t)value;
    return node;
}

spLiteralNode* spNewLiteralU16(uint16_t value) {
    spLiteralNode* node = malloc(sizeof(spLiteralNode));
    node->base.type = SP_NODE_LITERAL;
    node->value_type = SP_VAL_TYPE_U16;
    node->u64 = (uint64_t)value;
    return node;
}

spLiteralNode* spNewLiteralU8(uint8_t value) {
    spLiteralNode* node = malloc(sizeof(spLiteralNode));
    node->base.type = SP_NODE_LITERAL;
    node->value_type = SP_VAL_TYPE_U8;
    node->u64 = (uint64_t)value;
    return node;
}

spBinaryExprNode* spNewBinary(spASTNode* left, spASTNode* right, spOperationType op) {
    spBinaryExprNode* node = malloc(sizeof(spBinaryExprNode));
    node->base.type = SP_NODE_BINARY_EXPR;
    node->left = left;
    node->right = right;
    node->op = op;
    return node;
}

spVarDeclNode* spNewVarDecl(const char* name, spValueType type, spASTNode* value) {
    spVarDeclNode* node = malloc(sizeof(spVarDeclNode));
    node->base.type = SP_NODE_VAR_DECL;
    node->name = strdup(name);
    node->var_type = type;
    node->value = value;
    return node;
}

spAssignNode* spNewAssign(const char* name, spASTNode* value) {
    spAssignNode* node = malloc(sizeof(spAssignNode));
    node->base.type = SP_NODE_ASSIGN;
    node->name = strdup(name);
    node->value = value;
    return node;
}

void spFreeAST(spASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case SP_NODE_LITERAL:
            free(node);
            break;

        case SP_NODE_IDENTIFIER: {
            spIdentifierNode* id = (spIdentifierNode*)node;
            free(id->name);
            free(id);
            break;
        }

        case SP_NODE_BINARY_EXPR: {
            spBinaryExprNode* bin = (spBinaryExprNode*)node;
            spFreeAST(bin->left);
            spFreeAST(bin->right);
            free(bin);
            break;
        }

        case SP_NODE_VAR_DECL: {
            spVarDeclNode* var = (spVarDeclNode*)node;
            free(var->name);
            spFreeAST(var->value);
            free(var);
            break;
        }

        case SP_NODE_ASSIGN: {
            spAssignNode* asg = (spAssignNode*)node;
            free(asg->name);
            spFreeAST(asg->value);
            free(asg);
            break;
        }

        case SP_NODE_PROGRAM: {
            spProgramNode* prog = (spProgramNode*)node;

            for (size_t i = 0; i < prog->statement_count; i++)
                spFreeAST(prog->statements[i]);

            free(prog->statements);
            free(prog);
            break;
        }
    }
}