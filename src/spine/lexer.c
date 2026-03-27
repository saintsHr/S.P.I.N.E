#include "spine/lexer.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

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
            spToken tk = {0};
            tk.type = SP_TOKEN_TYPE_NUMBER;
            tk.line = line;
            tk.column = col;

            int j = 0;

            while (isdigit(input[i]) || input[i] == '.') {
                if (j < SP_MAX_TOKEN_VALUE_SIZE - 1)
                    tk.value[j++] = input[i];
                i++;
                col++;
            }

            tk.value[j] = '\0';

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
                    tk.type = SP_TOKEN_TYPE_UNDEFINED;
                    break;
            }

            tk.value[0] = c;
            tk.value[1] = '\0';

            addToken(&list, tk);

            col++;
            i++;
            continue;
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