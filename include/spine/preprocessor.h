#pragma once

#define MAX_DEFINES             512
#define MAX_DEFINE_NAME_LENGTH  64
#define MAX_DEFINE_VALUE_LENGTH 128

char* preprocess(const char* source, long srcSize, const char* filename);