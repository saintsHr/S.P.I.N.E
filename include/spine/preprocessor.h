#pragma once

#define MAX_DEFINES             128
#define MAX_DEFINE_NAME_LENGHT  64
#define MAX_DEFINE_VALUE_LENGHT 128

char* preprocess(const char* source, long srcSize, const char* filename);