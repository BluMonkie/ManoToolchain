#include <stdio.h>

#include "common/file.h"
#include "lexer.h"
#include "debug.h"
#include "ast_printer.h"
#include "parser.h"

int main(int argc, char **argv) {
    char *src = read_file_string(argv[1]);

    LexerState lexer = lexer_new(src);
    scan_tokens(&lexer);

    print_tokens(stdout, lexer.tokens);
    printf("\n\n\n\n");

    ParserState parser = parser_new(src, lexer.tokens);
    Program prog = parse_program(&parser);

    print_ast(&prog);
}