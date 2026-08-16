#pragma once

#include <stdbool.h>

typedef struct {
    bool had_error;
    
    bool print_help;
    bool print_version;
    bool dump_tokens;
    bool dump_ast;
    bool dump_symbols;
    char *input_file_path;
    char *output_file_path;
} CliOptions;

CliOptions parse_cli_arguments(int argc, char **argv);