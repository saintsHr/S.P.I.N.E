#include "spine/preprocessor.h"
#include "spine/util/log.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#define STR2(x) #x
#define STR(x) STR2(x)

#define DEFINE_KEYWORD "define"

typedef struct {
    spDefine defines[SP_MAX_DEFINES];
    int defineCount;
} spPreprocessorContext;

static inline int isIdentifierChar(char c) {
    return isalnum((unsigned char)c) || c == '_';
}

static inline char* skipWhitespace(char* p) {
    while (isspace((unsigned char)*p)) p++;
    return p;
}

static void ensureCapacity(
    char** buffer,
    char** write,
    size_t* bufSize,
    size_t needed,
    const char* filename
) {
    size_t used = *write - *buffer;

    if (used + needed + 1 > *bufSize) {
        *bufSize = (*bufSize * 2) + needed;

        char* tmp = realloc(*buffer, *bufSize);
        if (!tmp) {
            spEmitLogHelper(
                SP_SEV_FATAL,
                filename,
                0,
                0,
                SP_PREP_CANNOT_MALLOC_DEFINES_BUFFER,
                "Memory allocation failed",
                "Cannot realloc memory for defines buffer.",
                "Make sure you have enough memory and try again."
            );
            free(*buffer);
            return;
        }

        *buffer = tmp;
        *write  = *buffer + used;
    }
}

static void parseDefine(spPreprocessorContext* ctx, char* line, const char* filename) {
    if (ctx->defineCount >= SP_MAX_DEFINES) {
        spEmitLogHelper(
            SP_SEV_FATAL,
            filename,
            0,
            0,
            SP_PREP_TOO_MANY_DEFINES,
            "Too many defines",
            "file provided (%s) has too many defines",
            "Remove useless defines.",
            filename
        );
        return;
    }

    char name [SP_MAX_DEFINE_NAME_LENGTH]  = {0};
    char value[SP_MAX_DEFINE_VALUE_LENGTH] = {0};

    char* p = line + strlen(DEFINE_KEYWORD);
    p = skipWhitespace(p);

    char* n = name;
    while (*p && !isspace((unsigned char)*p)) {
        if ((size_t)(n - name) < SP_MAX_DEFINE_NAME_LENGTH - 1) *n++ = *p;
        p++;
    }
    *n = '\0';

    p = skipWhitespace(p);

    snprintf(value, sizeof(value), "%s", p);

    spDefine* def = &ctx->defines[ctx->defineCount];

    snprintf(def->name, SP_MAX_DEFINE_NAME_LENGTH, "%s", name);
    snprintf(def->value, SP_MAX_DEFINE_VALUE_LENGTH, "%s", value);

    def->nameLen  = strlen(def->name);
    def->valueLen = strlen(def->value);

    ctx->defineCount++;
}

static char* extractDefines(spPreprocessorContext* ctx, char* str, const char* filename) {
    size_t size   = strlen(str);
    char*  buffer = malloc(size + 1);

    if (!buffer) {
        spEmitLogHelper(
            SP_SEV_FATAL,
            filename,
            0,
            0,
            SP_PREP_CANNOT_MALLOC_DEFINES_BUFFER,
            "Memory allocation failed",
            "Cannot allocate memory for defines buffer.",
            "Make sure you have enough memory and try again."
        );
        return NULL;
    }

    char* read      = str;
    char* write     = buffer;
    char  line[512] = {};

    const size_t keywordLen = strlen(DEFINE_KEYWORD);

    while (*read) {
        size_t i = 0;
        while (*read && *read != '\n' && i < sizeof(line) - 1) line[i++] = *read++;
        line[i] = '\0';

        if (*read == '\n') read++;

        char* trim = skipWhitespace(line);

        if (strncmp(trim, DEFINE_KEYWORD, keywordLen) == 0 &&
            isspace((unsigned char)trim[keywordLen])) {
            parseDefine(ctx, trim, filename);
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

static char* applyDefines(spPreprocessorContext* ctx, char* str, const char* filename) {
    if (!str) return NULL;

    size_t bufSize = strlen(str) + 128;
    char* buffer = malloc(bufSize);

    if (!buffer) {
        spEmitLogHelper(
            SP_SEV_FATAL,
            filename,
            0,
            0,
            SP_PREP_CANNOT_MALLOC_DEFINES_BUFFER,
            "Memory allocation failed",
            "Cannot allocate memory for defines buffer.",
            "Make sure you have enough memory and try again."
        );
        return NULL;
    }

    char* read  = str;
    char* write = buffer;

    while (*read) {
        int replaced = 0;

        for (int i = 0; i < ctx->defineCount; i++) {
            spDefine* def = &ctx->defines[i];

            if (strncmp(read, def->name, def->nameLen) != 0) continue;

            char before = (read == str) ? ' ' : read[-1];
            char after  = read[def->nameLen];

            if (!isIdentifierChar(before) && !isIdentifierChar(after)) {
                ensureCapacity(&buffer, &write, &bufSize, def->valueLen, filename);

                memcpy(write, def->value, def->valueLen);
                write += def->valueLen;
                read  += def->nameLen;

                replaced = 1;
                break;
            }
        }

        if (!replaced) {
            ensureCapacity(&buffer, &write, &bufSize, 1, filename);
            *write++ = *read++;
        }
    }

    *write = '\0';
    return buffer;
}

static inline void removeCarriageReturn(char* str) {
    char* read = str;
    char* write = str;

    while (*read) {
        if (*read != '\r') *write++ = *read;
        read++;
    }

    *write = '\0';
}

static inline void removeComments(char* str) {
    char* read  = str;
    char* write = str;

    while (*read) {
        if (*read == '/' && *(read + 1) == '/') {
            while (*read && *read != '\n') read++;
        } else {
            *write++ = *read++;
        }
    }

    *write = '\0';
}

static inline void removeEmptyLines(char* str) {
    char* read = str;
    char* write = str;

    while (*read) {
        char* lineStart = read;

        while (isspace((unsigned char)*read) && *read != '\n') read++;

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
    spPreprocessorContext ctx = {0};

    char* out = malloc(srcSize + 1);
    memcpy(out, src, srcSize);
    out[srcSize] = '\0';

    removeCarriageReturn(out);
    removeComments(out);

    char* tmp;

    tmp = extractDefines(&ctx, out, filename);
    free(out);
    out = tmp;

    tmp = applyDefines(&ctx, out, filename);
    free(out);
    out = tmp;

    removeEmptyLines(out);

    return out;
}