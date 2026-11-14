#include <stdio.h>
#include <stdlib.h>

#include "asm_writer.h"
#include "codegen.h"
#include "common/file.h"
#include "lexer.h"
#include "debug.h"
#include "parser.h"
#include "cli.h"

#define MS_FILE_EXTENSION ".ms"
#define MANO_ASM_FILE_EXTENSION ".masm"
#define MS_TOKENS_FILE_EXTENSION "_tokens.txt"
#define MS_AST_FILE_EXTENSION "_ast.txt"

const char BUILTIN_SRC[] = {
    #embed "builtins.ms" suffix(, '\0') if_empty('\0')
};

void print_help() {
    printf("ManoScript - A programming language targeting mano basic computer\n");
    printf("Usage: manoscript [options] input_file.ms\n");
    printf("Valid options: --help, --version, --dump-tokens, --dump-ast, --output output_file\n");
}

void print_version() {
    printf("ManoScript - A programming language targeting mano basic computer\n");
    printf("Version v1.0.0\n");
}


int main(int argc, char **argv) {
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

    LexerState lexer_builtin = lexer_new(BUILTIN_SRC);
    scan_tokens(&lexer_builtin);

    LexerState lexer_prog = lexer_new(src);
    scan_tokens(&lexer_prog);

    if (lexer_builtin.had_error || lexer_prog.had_error) {
        exit(EXIT_FAILURE);
    }
    
    if (options.dump_tokens) {
        char *tokens_file_path = replace_or_add_extension(options.input_file_path, MS_FILE_EXTENSION, MS_TOKENS_FILE_EXTENSION);
        if (tokens_file_path == NULL) {
            fprintf(stderr, "ERROR: Failed to allocate output path.\n");
            exit(EXIT_FAILURE);
        }
        FILE *tokens_file = fopen(tokens_file_path, "w");
        if (tokens_file == NULL) {
            fprintf(stderr, "ERROR: Can't write to file %s.\n", tokens_file_path);
            exit(EXIT_FAILURE);
        }
        print_tokens(tokens_file, lexer_prog.tokens);
        fclose(tokens_file);
        free(tokens_file_path);
    }
    
    ParserState parser = parser_new();
    parse_program_builtin(&parser, BUILTIN_SRC, lexer_builtin.tokens);
    Program prog = parse_program(&parser, src, lexer_prog.tokens);

    if (parser.had_error) {
        exit(EXIT_FAILURE);
    }

    if (options.dump_ast) {
        char *ast_file_path = replace_or_add_extension(options.input_file_path, MS_FILE_EXTENSION, MS_AST_FILE_EXTENSION);
        if (ast_file_path == NULL) {
            fprintf(stderr, "ERROR: Failed to allocate output path.\n");
            exit(EXIT_FAILURE);
        }
        FILE *ast_file = fopen(ast_file_path, "w");
        if (ast_file == NULL) {
            fprintf(stderr, "ERROR: Can't write to file %s.\n", ast_file_path);
            exit(EXIT_FAILURE);
        }
        print_ast(&prog, ast_file);
        fclose(ast_file);
        free(ast_file_path);
    }

    CodegenState state = codegen_new(&prog);
    emit_instructions(&state);

    if (options.output_file_path == NULL) {
        options.output_file_path = replace_or_add_extension(options.input_file_path, MS_FILE_EXTENSION, MANO_ASM_FILE_EXTENSION);
    }
    
    if (options.output_file_path == NULL) {
        fprintf(stderr, "ERROR: Failed to allocate output path.\n");
        exit(EXIT_FAILURE);
    }
    
    FILE *output_file = fopen(options.output_file_path, "w");
    if (output_file == NULL) {
        fprintf(stderr, "ERROR: Can't write to file %s.\n", options.output_file_path);
        exit(EXIT_FAILURE);
    }
    write_asm(state.instructions, output_file);
    fclose(output_file);
    free(options.output_file_path);
}