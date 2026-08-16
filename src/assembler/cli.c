#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include "cli.h"

typedef enum {
    CLI_UNKNOWN,
    CLI_NOT_ARGUMENT,
    CLI_OUTPUT,
    CLI_HELP,
    CLI_VERSION,
    CLI_DUMP_TOKENS,
    CLI_DUMP_AST,
    CLI_DUMP_SYMBOLS,
} CliArgument;

CliArgument argument_from_string(const char *arg) {
    if (strcmp(arg, "--help") == 0) return CLI_HELP;
    else if (strcmp(arg, "--version") == 0) return CLI_VERSION;
    else if (strcmp(arg, "--output") == 0) return CLI_OUTPUT;
    else if (strcmp(arg, "--dump-tokens") == 0) return CLI_DUMP_TOKENS;
    else if (strcmp(arg, "--dump-ast") == 0) return CLI_DUMP_AST;
    else if (strcmp(arg, "--dump-symbols") == 0) return CLI_DUMP_SYMBOLS;
    else if (strncmp(arg, "--", 2) == 0) return CLI_UNKNOWN;
    else return CLI_NOT_ARGUMENT;
}

CliOptions parse_cli_arguments(int argc, char **argv) {
    CliOptions options = {0};

    for (int i = 1; i < argc; i++) {
        CliArgument arg = argument_from_string(argv[i]);

        switch (arg) {
            case CLI_HELP:
                options.print_help = true;
                break;
            case CLI_VERSION:
                options.print_version = true;
                break;
            case CLI_DUMP_TOKENS:
                options.dump_tokens = true;
                break;
            case CLI_DUMP_AST:
                options.dump_ast = true;
                break;
            case CLI_DUMP_SYMBOLS:
                options.dump_symbols = true;
                break;
            case CLI_OUTPUT:
                if (i == argc - 1) {
                    options.had_error = true;
                    fprintf(stderr, "ERROR: Missing output_file after --output. See manoasm --help.\n");
                    break;
                }
                char *output_file = argv[++i];
                options.output_file_path = output_file;
                break;
            case CLI_UNKNOWN:
                options.had_error = true;
                fprintf(stderr, "ERROR: Unknown CLI option %s. See manoasm --help.\n", argv[i]);
                break;
            case CLI_NOT_ARGUMENT:
                if (options.input_file_path != NULL) {
                    fprintf(stderr, "ERROR: Multiple input files specified.\n");
                    options.had_error = true;
                    break;
                }
                options.input_file_path = argv[i];
                break;
        }
    }

    return options;
}