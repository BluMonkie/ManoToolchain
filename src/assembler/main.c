#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/file.h"
#include "lexer.h"
#include "debug.h"
#include "parser.h"
#include "encoder.h"
#include "cli.h"

#define MANO_ASM_FILE_EXTENSION ".masm"
#define MANO_BIN_FILE_EXTENSION ".mano"
#define MANO_TOKENS_FILE_EXTENSION "_tokens.txt"
#define MANO_SYMBOLS_FILE_EXTENSION "_symbols.txt"
#define MANO_AST_FILE_EXTENSION "_ast.txt"

void print_help() {
    printf("ManoASM - A mano basic assembly assembler\n");
    printf("Usage: manoasm [options] input_file.masm\n");
    printf("Valid options: --help, --version, --dump-tokens, --dump-ast, --dump-symbols, --output output_file\n");
}

void print_version() {
    printf("ManoASM - A mano basic assembly assembler\n");
    printf("Version v1.0.0\n");
}

int main(int argc, char** argv) {
    CliOptions options = parse_cli_arguments(argc, argv);

    if (options.had_error) {
        exit(EXIT_FAILURE);
    }

    if (options.print_help) {
        print_help();
        exit(EXIT_SUCCESS);
    }

    if (options.print_version) {
        print_version();
        exit(EXIT_SUCCESS);
    }

    if (options.input_file_path == NULL) {
        fprintf(stderr, "ERROR: Missing source file. See manoasm --help.\n");
        exit(EXIT_FAILURE);
    }

    char *src = read_file_string(options.input_file_path);
    if (src == NULL) {
        fprintf(stderr, "ERROR: File %s not found.\n", options.input_file_path);
        exit(EXIT_FAILURE);
    }

    LexerState lexer = lexer_new(src);
    lexer_scan_tokens(&lexer);

    if (lexer.had_error) {
        exit(EXIT_FAILURE);
    }

    if (options.dump_tokens) {
        char *tokens_file_path = replace_or_add_extension(options.input_file_path, MANO_ASM_FILE_EXTENSION, MANO_TOKENS_FILE_EXTENSION);
        if (tokens_file_path == NULL) {
            fprintf(stderr, "ERROR: Failed to allocate output path.\n");
            exit(EXIT_FAILURE);
        }
        FILE *tokens_file = fopen(tokens_file_path, "w");
        if (tokens_file == NULL) {
            fprintf(stderr, "ERROR: Can't write to file %s.\n", tokens_file_path);
            exit(EXIT_FAILURE);
        }
        print_tokens(&lexer, tokens_file);
        fclose(tokens_file);
        free(tokens_file_path);
    }
    
    ParserState parser = parser_new(src, lexer.tokens);
    parse_tokens(&parser);

    if (parser.had_error) {
        exit(EXIT_FAILURE);
    }

    if (options.dump_symbols) {
        char *symbols_file_path = replace_or_add_extension(options.input_file_path, MANO_ASM_FILE_EXTENSION, MANO_SYMBOLS_FILE_EXTENSION);
        if (symbols_file_path == NULL) {
            fprintf(stderr, "ERROR: Failed to allocate output path.\n");
            exit(EXIT_FAILURE);
        }
        FILE *symbols_file = fopen(symbols_file_path, "w");
        if (symbols_file == NULL) {
            fprintf(stderr, "ERROR: Can't write to file %s.\n", symbols_file_path);
            exit(EXIT_FAILURE);
        }
        print_symbols(&parser.symbols, symbols_file);
        fclose(symbols_file);
        free(symbols_file_path);
    }

    if (options.dump_ast) {
        char *ast_file_path = replace_or_add_extension(options.input_file_path, MANO_ASM_FILE_EXTENSION, MANO_AST_FILE_EXTENSION);
        if (ast_file_path == NULL) {
            fprintf(stderr, "ERROR: Failed to allocate output path.\n");
            exit(EXIT_FAILURE);
        }
        FILE *ast_file = fopen(ast_file_path, "w");
        if (ast_file == NULL) {
            fprintf(stderr, "ERROR: Can't write to file %s.\n", ast_file_path);
            exit(EXIT_FAILURE);
        }
        print_ast(&parser, ast_file);
        fclose(ast_file);
        free(ast_file_path);
    }

    EncoderState encoder = encoder_new(parser.statements);
    encode_instructions(&encoder);

    if (options.output_file_path == NULL) {
        options.output_file_path = replace_or_add_extension(options.input_file_path, MANO_ASM_FILE_EXTENSION, MANO_BIN_FILE_EXTENSION);
    }

    write_buffer16_be(encoder.memory, MANO_MEMORY_SIZE, options.output_file_path);
}