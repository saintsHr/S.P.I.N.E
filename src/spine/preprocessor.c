#include "spine/preprocessor.h"

#include <string.h>
#include <stdlib.h>

char* preprocess(const char* src, long srcSize, const char* filename) {
    char* out = malloc((srcSize + 1) * sizeof(char));
    memcpy(out, src, srcSize);
    out[srcSize] = '\0';

    for (int i = 0; out[i] != '\0'; i++) {
        
    }

    return out;
}