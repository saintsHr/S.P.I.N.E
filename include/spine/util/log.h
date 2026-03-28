#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#define SP_COLOR_BLACK   "\x1b[30m"
#define SP_COLOR_RED     "\x1b[31m"
#define SP_COLOR_GREEN   "\x1b[32m"
#define SP_COLOR_YELLOW  "\x1b[33m"
#define SP_COLOR_BLUE    "\x1b[34m"
#define SP_COLOR_MAGENTA "\x1b[35m"
#define SP_COLOR_CYAN    "\x1b[36m"
#define SP_COLOR_WHITE   "\x1b[37m"

#define SP_COLOR_BBLACK   "\x1b[90m"
#define SP_COLOR_BRED     "\x1b[91m"
#define SP_COLOR_BGREEN   "\x1b[92m"
#define SP_COLOR_BYELLOW  "\x1b[93m"
#define SP_COLOR_BBLUE    "\x1b[94m"
#define SP_COLOR_BMAGENTA "\x1b[95m"
#define SP_COLOR_BCYAN    "\x1b[96m"
#define SP_COLOR_BWHITE   "\x1b[97m"

#define SP_COLOR_RESET "\x1b[0m"

#define SP_INDENT "  "

typedef enum {
    SP_SEV_INFO,
    SP_SEV_WARNING,
    SP_SEV_ERROR,
    SP_SEV_FATAL
} spSeverity;

typedef struct {
    spSeverity sev;
    const char* file;
    int line;
    int col;
    uint16_t code;
    const char* title;
    const char* desc;
    const char* hint;
} spLogInfo;

typedef enum {
    SP_MAIN_NO_INPUT_FILE               = 0x0000,
    SP_MAIN_NO_OUTPUT_FILE              = 0x0001,
    SP_MAIN_UNKNOWN_FLAG                = 0x0002,
    SP_MAIN_CANNOT_OPEN_INPUT_FILE      = 0x0003,
    SP_MAIN_CANNOT_MALLOC_INPUT_BUFFER  = 0x0004,
    SP_MAIN_CANNOT_OPEN_OUTPUT_FILE     = 0x0005,

    SP_PREP_TOO_MANY_DEFINES             = 0x1000,
    SP_PREP_CANNOT_MALLOC_DEFINES_BUFFER = 0x1001,

    SP_LEXER_UNDEFINED_TOKEN = 0x2000,

    SP_AST_REALLOC_FAILED = 0x3000,
} spErrorCode;

static void spPrintIndented(const char *text) {
    char buffer[1024];
    strncpy(buffer, text, sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';

    char *line = strtok(buffer, "\n");
    while (line) {
        printf(SP_INDENT "%s\n", line);
        line = strtok(NULL, "\n");
    }
}

static void spEmitLog(spLogInfo info, ...) {
    printf("\n");

    const char *sevStr = "";
    switch (info.sev) {
        case SP_SEV_INFO:    sevStr = SP_COLOR_BCYAN    "INFO"    SP_COLOR_RESET; break;
        case SP_SEV_WARNING: sevStr = SP_COLOR_BMAGENTA "WARNING" SP_COLOR_RESET; break;
        case SP_SEV_ERROR:   sevStr = SP_COLOR_BRED     "ERROR"   SP_COLOR_RESET; break;
        case SP_SEV_FATAL:   sevStr = SP_COLOR_RED      "FATAL"   SP_COLOR_RESET; break;
    }

    char titleBuffer[1024];
    char descBuffer[1024];
    char hintBuffer[1024];

    va_list args;
    va_start(args, info);

    va_list args_copy1;
    va_copy(args_copy1, args);
    vsnprintf(titleBuffer, sizeof(titleBuffer), info.title, args_copy1);
    va_end(args_copy1);

    va_list args_copy2;
    va_copy(args_copy2, args);
    vsnprintf(descBuffer, sizeof(descBuffer), info.desc, args_copy2);
    va_end(args_copy2);

    va_list args_copy3;
    va_copy(args_copy3, args);
    if (info.hint)
        vsnprintf(hintBuffer, sizeof(hintBuffer), info.hint, args_copy3);
    va_end(args_copy3);

    va_end(args);

    printf(
        "[%s][%s][%d:%d][0x%04x]: %s\n\n",
        sevStr,
        info.file,
        info.line,
        info.col,
        info.code,
        titleBuffer
    );

    printf(SP_COLOR_BBLUE " Desc:\n" SP_COLOR_RESET);
    spPrintIndented(descBuffer);
    printf("\n");

    if (info.hint) {
        printf(SP_COLOR_BGREEN " Hint:\n" SP_COLOR_RESET);
        spPrintIndented(hintBuffer);
        printf("\n");
    }

    if (info.sev == SP_SEV_FATAL) exit(EXIT_FAILURE);
}