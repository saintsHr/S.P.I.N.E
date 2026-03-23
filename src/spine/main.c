#include <stdbool.h>
#include <string.h>

#include "spine/log.h"

typedef struct {
    char* output_file;
    int   optimization_level;
    bool  warnings;
} CompilerOptions;

void parseFlags(int argc, char* argv[], CompilerOptions* options) {
    for (int i = 1; i < argc; i++) {
        bool hasNext = i + 1 < argc;

        // output file
        if (strcmp(argv[i], "-o") == 0) {
            if (!hasNext) {
                spLogInfo l;
                l.code = SP_MAIN_NO_OUTPUT_FILE;
                l.col = 0;
                l.line = 0;
                l.file = "N/A";
                l.title = "No output file";
                l.desc = "Output file not provided";
                l.hint = "choose a file to output or remove '-o' flag";
                l.sev = SP_SEV_FATAL;
                spEmitLog(l, argv[i]);
            }
            options->output_file = argv[i + 1];
            i++;
            continue;
        }

        // activates warnings
        if (strcmp(argv[i], "-w") == 0) {
            options->warnings = true;
            continue;
        }

        // optimization off
        if (strcmp(argv[i], "-O0") == 0) {
            options->optimization_level = 0;
            continue;
        }

        // optimization level 1
        if (strcmp(argv[i], "-O1") == 0) {
            options->optimization_level = 1;
            continue;
        }

        // optimization level 2
        if (strcmp(argv[i], "-O2") == 0) {
            options->optimization_level = 2;
            continue;
        }

        // optimization level 3
        if (strcmp(argv[i], "-O3") == 0) {
            options->optimization_level = 3;
            continue;
        }

        // unknown flag
        spLogInfo l;
        l.code = SP_MAIN_UNKNOWN_FLAG;
        l.col = 0;
        l.line = 0;
        l.file = "N/A";
        l.title = "Unknown flag";
        l.desc = "An unknown flag (%s) has been provided.";
        l.hint = "try using the --help flag or removing this flag.";
        l.sev = SP_SEV_FATAL;
        spEmitLog(l, argv[i]);
    }
}

int main(int argc, char* argv[]) {
    // creates compiler options and fills with defaults
    CompilerOptions options;
    options.warnings = false;
    options.output_file = "output";
    options.optimization_level = 0;

    // parses for flags
    parseFlags(argc, argv, &options);

    return 0;
}