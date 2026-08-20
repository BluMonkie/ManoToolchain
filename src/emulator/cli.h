#pragma once

#include <stdbool.h>

typedef struct {
    bool had_error;
    
    bool print_help;
    bool print_version;
    char *input_file_path;
} CliOptions;

CliOptions parse_cli_arguments(int argc, char **argv);