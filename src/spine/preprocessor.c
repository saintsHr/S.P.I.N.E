#include "spine/preprocessor.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

typedef struct {
    char name[MAX_DEFINE_NAME_LENGHT];
    char value[MAX_DEFINE_VALUE_LENGHT];
} SpDefine;

SpDefine defines[MAX_DEFINES];
int defineCount = 0;

void parseDefine(char *line) {
    char name[MAX_DEFINE_NAME_LENGHT];
    char value[MAX_DEFINE_VALUE_LENGHT];

    char *p = line + 7;
    while (*p == ' ') p++;

    sscanf(p, "%63s", name);

    char *valueStart = strstr(p, name) + strlen(name);
    while (*valueStart == ' ') valueStart++;

    strcpy(value, valueStart);

    strcpy(defines[defineCount].name, name);
    strcpy(defines[defineCount].value, value);
    defineCount++;
}

char* extractDefines(char *str) {
    size_t size = strlen(str);
    char *buffer = malloc(size + 1);
    if (!buffer) return str;

    char *read = str;
    char *write = buffer;
    char line[512];

    while (*read) {
        int i = 0;

        while (*read && *read != '\n' && i < sizeof(line) - 1) {
            line[i++] = *read++;
        }

        line[i] = '\0';
        if (*read == '\n') read++;

        char *trim = line;
        while (*trim == ' ' || *trim == '\t') trim++;

        if (strncmp(trim, "define ", 7) == 0) {
            parseDefine(trim);
        } else {
            size_t len = strlen(line);
            memcpy(write, line, len);
            write += len;
            *write++ = '\n';
        }
    }

    *write = '\0';
    return buffer;
}

char* applyDefines(char *str) {
    size_t size = strlen(str);

    char *buffer = malloc(size + defineCount * MAX_DEFINE_VALUE_LENGHT + 1);
    if (!buffer) return str;

    char *write = buffer;
    char *read = str;

    while (*read) {
        int replaced = 0;

        for (int i = 0; i < defineCount; i++) {
            int len = strlen(defines[i].name);

            if (strncmp(read, defines[i].name, len) == 0) {
                char before = (read == str) ? ' ' : read[-1];
                char after = read[len];

                if ((!isalnum(before) && before != '_') &&
                    (!isalnum(after) && after != '_')) {

                    size_t vlen = strlen(defines[i].value);
                    memcpy(write, defines[i].value, vlen);
                    write += vlen;
                    read += len;
                    replaced = 1;
                    break;
                }
            }
        }

        if (!replaced) {
            *write++ = *read++;
        }
    }

    *write = '\0';

    return buffer;
}

void removeCarriageReturn(char *str) {
    char *read = str;
    char *write = str;

    while (*read) {
        if (*read != '\r') *write++ = *read;
        read++;
    }

    *write = '\0';
}

void removeComments(char* str) {
    char *read = str;
    char *write = str;

    while (*read) {
        if (*read == '/' && *(read + 1) == '/') {
            while (*read && *read != '\n') read++;
        } else {
            *write = *read;
            write++;
            read++;
        }
    }

    *write = '\0';
}

void removeEmptyLines(char *str) {
    char *read = str;
    char *write = str;

    while (*read) {
        char *lineStart = read;

        while (*read == ' ' || *read == '\t') read++;

        if (*read == '\n') {
            read++;
            continue;
        }

        read = lineStart;
        while (*read && *read != '\n') {
            *write++ = *read++;
        }

        if (*read == '\n') {
            *write++ = *read++;
        }
    }

    *write = '\0';
}

char* preprocess(const char* src, long srcSize, const char* filename) {
    char* out = malloc(srcSize + 1);
    memcpy(out, src, srcSize);
    out[srcSize] = '\0';

    memset(defines, 0, sizeof(defines));
    defineCount = 0;

    removeCarriageReturn(out);
    removeComments(out);

    char *tmp;

    tmp = extractDefines(out);
    free(out);
    out = tmp;

    tmp = applyDefines(out);
    free(out);
    out = tmp;

    removeEmptyLines(out);

    return out;
}