#include "spine/lexer.h"
#include "spine/util/log.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void addToken(spTokenList* list, spToken token) {
    if (list->count >= list->capacity) {
        list->capacity = list->capacity == 0 ? 16 : list->capacity * 2;
        list->tokens = realloc(list->tokens, list->capacity * sizeof(spToken));
    }

    list->tokens[list->count++] = token;
}

spTokenList tokenize(const char* input, const char* filename) {
    spTokenList list = {0};

    int i = 0;
    int line = 1;
    int col = 1;

    while (input[i] != '\0') {
        char c = input[i];

        // new line
        if (c == '\n') {
            line++;
            col = 1;
            i++;
            continue;
        }

        // whitespaces
        if (c == ' ' || c == '\t' || c == '\r') {
            i++;
            col++;
            continue;
        }

        // number
        if (isdigit(c)) {
            int start_i = i;
            spToken tk = {0};
            tk.type = SP_TOKEN_TYPE_NUMBER;
            tk.line = line;
            tk.column = col;

            int j = 0;
            bool has_dot = false;

            while (isdigit(input[i]) || (input[i] == '.' && !has_dot)) {
                if (input[i] == '.') has_dot = true;
                if (j < SP_MAX_TOKEN_VALUE_SIZE - 1)
                    tk.value[j++] = input[i];
                i++;
                col++;
            }

            tk.value[j] = '\0';

            if (isalpha(input[i])) {
                i = start_i;
                col = tk.column;
                goto undefined_token;
            }

            addToken(&list, tk);
            continue;
        }

        // identifier / keyword
        if (isalpha(c) || c == '_') {
            spToken tk = {0};
            tk.line = line;
            tk.column = col;

            int j = 0;

            while (isalnum(input[i]) || input[i] == '_') {
                if (j < SP_MAX_TOKEN_VALUE_SIZE - 1)
                    tk.value[j++] = input[i];
                i++;
                col++;
            }

            tk.value[j] = '\0';

            // keyword check
            if (strcmp(tk.value, "i8") == 0) tk.type = SP_TOKEN_TYPE_KW_I8;
            else if (strcmp(tk.value, "i16") == 0) tk.type = SP_TOKEN_TYPE_KW_I16;
            else if (strcmp(tk.value, "i32") == 0) tk.type = SP_TOKEN_TYPE_KW_I32;
            else if (strcmp(tk.value, "i64") == 0) tk.type = SP_TOKEN_TYPE_KW_I64;
            else if (strcmp(tk.value, "u8") == 0) tk.type = SP_TOKEN_TYPE_KW_U8;
            else if (strcmp(tk.value, "u16") == 0) tk.type = SP_TOKEN_TYPE_KW_U16;
            else if (strcmp(tk.value, "u32") == 0) tk.type = SP_TOKEN_TYPE_KW_U32;
            else if (strcmp(tk.value, "u64") == 0) tk.type = SP_TOKEN_TYPE_KW_U64;
            else if (strcmp(tk.value, "f32") == 0) tk.type = SP_TOKEN_TYPE_KW_F32;
            else if (strcmp(tk.value, "f64") == 0) tk.type = SP_TOKEN_TYPE_KW_F64;
            else tk.type = SP_TOKEN_TYPE_IDENTIFIER;

            addToken(&list, tk);
            continue;
        }

        // string
        if (c == '"') {
            spToken tk = {0};
            tk.type = SP_TOKEN_TYPE_STRING;
            tk.line = line;
            tk.column = col;

            i++;
            col++;

            int j = 0;

            while (input[i] != '"' && input[i] != '\0') {
                if (j < SP_MAX_TOKEN_VALUE_SIZE - 1)
                    tk.value[j++] = input[i];
                i++;
                col++;
            }

            tk.value[j] = '\0';

            if (input[i] == '"') {
                i++;
                col++;
            }

            addToken(&list, tk);
            continue;
        }

        // symbols
        {
            spToken tk = {0};
            tk.line = line;
            tk.column = col;

            switch (c) {
                case '+':
                    tk.type = SP_TOKEN_TYPE_PLUS;
                    break;
                case '-':
                    tk.type = SP_TOKEN_TYPE_MINUS;
                    break;
                case '*':
                    tk.type = SP_TOKEN_TYPE_MULT;
                    break;
                case '/':
                    tk.type = SP_TOKEN_TYPE_DIV;
                    break;
                case '=':
                    tk.type = SP_TOKEN_TYPE_EQUALS;
                    break;
                case ';':
                    tk.type = SP_TOKEN_TYPE_SEMICOLON;
                    break;
                default:
                    goto undefined_token;
                    break;
            }

            tk.value[0] = c;
            tk.value[1] = '\0';
            addToken(&list, tk);
            col++;
            i++;
            continue;
        }

        // undefined
        undefined_token:
        {
            spToken tk = {0};
            tk.type = SP_TOKEN_TYPE_UNDEFINED;
            tk.line = line;
            tk.column = col;

            int j = 0;

            while (input[i] != '\0' && !isspace(input[i])) {
                if (j < SP_MAX_TOKEN_VALUE_SIZE - 1) tk.value[j++] = input[i];
                i++;
                col++;
            }
            
            tk.value[j] = '\0';

            addToken(&list, tk);
            
            spLogInfo l;
            l.code = SP_LEXER_UNDEFINED_TOKEN;
            l.col = tk.column;
            l.line = tk.line;
            l.file = filename;
            l.sev = SP_SEV_FATAL;
            l.title = "Undefined Token";
            l.desc = "Undefined token in source file (%s).";
            l.hint = "follow the language grammar.";
            spEmitLog(l, tk.value);
        }
    }

    spToken tk = {0};
    tk.line = line;
    tk.column = col;
    tk.type = SP_TOKEN_TYPE_EOF;
    tk.value[0] = '\0';
    addToken(&list, tk);

    return list;
}