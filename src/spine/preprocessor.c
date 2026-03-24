#include "spine/preprocessor.h"
#include "spine/util/log.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#define STR2(x) #x
#define STR(x) STR2(x)

typedef struct {
    char name[MAX_DEFINE_NAME_LENGHT];
    char value[MAX_DEFINE_VALUE_LENGHT];
} SpDefine;

SpDefine defines[MAX_DEFINES];
int defineCount = 0;

void parseDefine(char *line, const char* filename) {
    if (defineCount >= MAX_DEFINES) {
        spLogInfo l;
        l.code = SP_MAIN_CANNOT_OPEN_OUTPUT_FILE;
        l.col = 0;
        l.line = 0;
        l.file = filename;
        l.title = "Too many defines";
        l.desc = "file provided (%s) has too many defines";
        l.hint = "Remove useless defines.";
        l.sev = SP_SEV_FATAL;
        spEmitLog(l, filename);
        return;
    }

    char name[MAX_DEFINE_NAME_LENGHT] = {0};
    char value[MAX_DEFINE_VALUE_LENGHT] = {0};

    char *p = line + 7;
    while (*p == ' ' || *p == '\t') p++;

    char *n = name;
    while (*p && !isspace((unsigned char)*p)) {
        if ((n - name) < MAX_DEFINE_NAME_LENGHT - 1)
            *n++ = *p;
        p++;
    }
    *n = '\0';

    while (*p == ' ' || *p == '\t') p++;

    strncpy(value, p, MAX_DEFINE_VALUE_LENGHT - 1);
    value[MAX_DEFINE_VALUE_LENGHT - 1] = '\0';

    strncpy(defines[defineCount].name, name, MAX_DEFINE_NAME_LENGHT);
    strncpy(defines[defineCount].value, value, MAX_DEFINE_VALUE_LENGHT);
    defineCount++;
}

char* extractDefines(char *str, const char* filename) {
    size_t size = strlen(str);
    char *buffer = malloc(size + 1);
    if (!buffer) {
        spLogInfo l;
        l.code = SP_PREP_CANNOT_MALLOC_DEFINES_BUFFER;
        l.col = 0;
        l.line = 0;
        l.file = filename;
        l.title = "Memory allocation failed";
        l.desc = "Cannot allocate memory for defines buffer.";
        l.hint = "Make sure you have enough memory and try again.";
        l.sev = SP_SEV_FATAL;
        spEmitLog(l);
    }

    char *read  = str;
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
            parseDefine(trim, filename);
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

char* applyDefines(char *str, const char* filename) {
    if (!str) return NULL;

    size_t bufSize = strlen(str) + 128;
    char *buffer = malloc(bufSize);
    if (!buffer) {
        spLogInfo l;
        l.code = SP_PREP_CANNOT_MALLOC_DEFINES_BUFFER;
        l.col = 0;
        l.line = 0;
        l.file = filename;
        l.title = "Memory allocation failed";
        l.desc = "Cannot allocate memory for defines buffer.";
        l.hint = "Make sure you have enough memory and try again.";
        l.sev = SP_SEV_FATAL;
        spEmitLog(l);
    }

    char *write = buffer;
    char *read = str;

    while (*read) {
        int replaced = 0;

        for (int i = 0; i < defineCount; i++) {
            int len = strlen(defines[i].name);

            if (strncmp(read, defines[i].name, len) == 0) {
                char before = (read == str) ? ' ' : read[-1];
                char after  = read[len];

                if ((!isalnum((unsigned char)before) && before != '_') &&
                    (!isalnum((unsigned char)after) && after != '_')) {

                    size_t vlen = strlen(defines[i].value);

                    // garante espaço suficiente
                    if ((write - buffer) + vlen + 1 > bufSize) {
                        bufSize = (bufSize * 2) + vlen;
                        size_t offset = write - buffer;
                        char *tmp = realloc(buffer, bufSize);
                        if (!tmp) {
                            spLogInfo l;
                            l.code = SP_PREP_CANNOT_MALLOC_DEFINES_BUFFER;
                            l.col = 0;
                            l.line = 0;
                            l.file = filename;
                            l.title = "Memory allocation failed";
                            l.desc = "Cannot realloc memory for defines buffer.";
                            l.hint = "Make sure you have enough memory and try again.";
                            l.sev = SP_SEV_FATAL;
                            free(buffer);
                            spEmitLog(l);
                        }
                        buffer = tmp;
                        write = buffer + offset;
                    }

                    memcpy(write, defines[i].value, vlen);
                    write += vlen;
                    read += len;
                    replaced = 1;
                    break;
                }
            }
        }

        if (!replaced) {
            if ((write - buffer) + 2 > bufSize) {
                bufSize *= 2;
                size_t offset = write - buffer;
                char *tmp = realloc(buffer, bufSize);
                if (!tmp) {
                    spLogInfo l;
                    l.code = SP_PREP_CANNOT_MALLOC_DEFINES_BUFFER;
                    l.col = 0;
                    l.line = 0;
                    l.file = filename;
                    l.title = "Memory allocation failed";
                    l.desc = "Cannot realloc memory for defines buffer.";
                    l.hint = "Make sure you have enough memory and try again.";
                    l.sev = SP_SEV_FATAL;
                    free(buffer);
                    spEmitLog(l);
                }
                buffer = tmp;
                write = buffer + offset;
            }
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

    tmp = extractDefines(out, filename);
    free(out);
    out = tmp;

    tmp = applyDefines(out, filename);
    free(out);
    out = tmp;

    removeEmptyLines(out);

    return out;
}