#include <stdbool.h>
#include <string.h>

#include "spine/util/log.h"
#include "spine/preprocessor.h"
#include "spine/lexer.h"
#include "spine/ast.h"

typedef struct {
    char* output_file;
    char* input_file;
    int   optimization_level;
    bool  warnings;
} CompilerOptions;

void printTokens(spTokenList* list) {
    for (size_t i = 0; i < list->count; i++) {
        printf("Token %ld: type=%d value=%s (%d:%d)\n",
            i,
            list->tokens[i].type,
            list->tokens[i].value,
            list->tokens[i].line,
            list->tokens[i].column
        );
    }
}

void parseFlags(int argc, char* argv[], CompilerOptions* options) {
    for (int i = 1; i < argc; i++) {
        bool hasNext = i + 1 < argc;

        // input file
        if (strcmp(argv[i], "-i") == 0) {
            if (!hasNext || argv[i + 1][0] == '-') {
                spLogInfo l;
                l.code = SP_MAIN_NO_INPUT_FILE;
                l.col = 0;
                l.line = 0;
                l.file = "N/A";
                l.title = "No input file";
                l.desc = "Input file not provided";
                l.hint = "Choose a input file or remove the '-i' flag";
                l.sev = SP_SEV_FATAL;
                spEmitLog(l, argv[i]);
            }
            options->input_file = argv[i + 1];
            i++;
            continue;
        }

        // output file
        if (strcmp(argv[i], "-o") == 0) {
            if (!hasNext || argv[i + 1][0] == '-') {
                spLogInfo l;
                l.code = SP_MAIN_NO_OUTPUT_FILE;
                l.col = 0;
                l.line = 0;
                l.file = "N/A";
                l.title = "No output file";
                l.desc = "Output file not provided";
                l.hint = "Choose a output file or remove the '-o' flag";
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
        l.hint = "Try using the --help flag or removing this flag.";
        l.sev = SP_SEV_FATAL;
        spEmitLog(l, argv[i]);
    }
}

int main(int argc, char* argv[]) {
    // creates compiler options and fills with defaults
    CompilerOptions options;
    options.warnings           = false;
    options.input_file         = "input.in";
    options.output_file        = "output.out";
    options.optimization_level = 0;

    // parses for flags
    parseFlags(argc, argv, &options);

    // opens input file
    FILE* inputFile = fopen(options.input_file, "rb");
    if (inputFile == NULL) {
        spLogInfo l;
        l.code = SP_MAIN_CANNOT_OPEN_INPUT_FILE;
        l.col = 0;
        l.line = 0;
        l.file = options.input_file;
        l.title = "Cannot open file";
        l.desc = "Unable to open input file (%s) provided.";
        l.hint = "Make sure you are providing the correct filename using the -i flag.";
        l.sev = SP_SEV_FATAL;
        spEmitLog(l, options.input_file);
    }

    // gets input file size
    fseek(inputFile, 0, SEEK_END);
    long inputSize = ftell(inputFile);
    rewind(inputFile);

    // allocates memory for input
    char* input = malloc(inputSize + 1);
    if (input == NULL) {
        fclose(inputFile);
        spLogInfo l;
        l.code = SP_MAIN_CANNOT_MALLOC_INPUT_BUFFER;
        l.col = 0;
        l.line = 0;
        l.file = options.input_file;
        l.title = "Memory allocation failed";
        l.desc = "Read buffer memory allocation failed.";
        l.hint = "Make sure you have enough memory and try again.";
        l.sev = SP_SEV_FATAL;
        spEmitLog(l);
    }

    // reads input file
    fread(input, sizeof(char), inputSize, inputFile);
    input[inputSize] = '\0';

    char* output = "";
    spTokenList tokens = {0};
    
    output = preprocess(input, inputSize, options.input_file);

    tokens = tokenize(output, options.output_file);

    // debug
    printTokens(&tokens);

    printf("\n\n");

    spProgramNode* prog = spNewProgram();

    /* a */
    spASTNode* a_val = (spASTNode*)spNewLiteralF64(3.1415);
    spASTNode* a_decl = (spASTNode*)spNewVarDecl("a", SP_VAL_TYPE_F32, a_val);
    spProgramAddStatement(prog, a_decl);

    /* b */
    spASTNode* b_val = (spASTNode*)spNewLiteralF64(3.1415);
    spASTNode* b_decl = (spASTNode*)spNewVarDecl("b", SP_VAL_TYPE_F32, b_val);
    spProgramAddStatement(prog, b_decl);

    /* c = a * b */
    spASTNode* id_a = (spASTNode*)spNewIdentifier("a");
    spASTNode* id_b = (spASTNode*)spNewIdentifier("b");
    spASTNode* mul = (spASTNode*)spNewBinary(id_a, id_b, SP_OP_TYPE_MUL);
    spASTNode* c_decl = (spASTNode*)spNewVarDecl("c", SP_VAL_TYPE_F32, mul);
    spProgramAddStatement(prog, c_decl);

    spPrintAST((spASTNode*)prog);

    // opens output file
    FILE* outputFile = fopen(options.output_file, "wb");
    if (outputFile == NULL) {
        spLogInfo l;
        l.code = SP_MAIN_CANNOT_OPEN_OUTPUT_FILE;
        l.col = 0;
        l.line = 0;
        l.file = options.output_file;
        l.title = "Cannot open file";
        l.desc = "Unable to open output file (%s) provided.";
        l.hint = "Make sure you have enough disk space & write permission and try again.";
        l.sev = SP_SEV_FATAL;
        spEmitLog(l, options.output_file);
    }

    // writes to output file
    fwrite(output, sizeof(char), strlen(output), outputFile);

    // closes files & free buffers
    fclose(outputFile);
    fclose(inputFile);
    free(output);
    free(input);

    return 0;
}