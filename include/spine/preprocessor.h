#pragma once

#define SP_MAX_DEFINES             512
#define SP_MAX_DEFINE_NAME_LENGTH  64
#define SP_MAX_DEFINE_VALUE_LENGTH 128

typedef struct {
    char name[SP_MAX_DEFINE_NAME_LENGTH];
    char value[SP_MAX_DEFINE_VALUE_LENGTH];
} spDefine;

char* preprocess(const char* source, long srcSize, const char* filename);