#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/file.h"
#include "lexer.h"
#include "debug.h"
#include "parser.h"
#include "encoder.h"

void write_buffer_be(uint16_t *buf, char *output_file_name) {
    uint8_t buffer[MANO_MEMORY_SIZE * 2];

    for (size_t i = 0; i < MANO_MEMORY_SIZE; i++) {
        uint16_t word = buf[i];

        buffer[i * 2]     = (uint8_t)(word >> 8);
        buffer[i * 2 + 1] = (uint8_t)word;
    }

    write_file_buffer(output_file_name, buffer, sizeof(buffer));
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: manoasm file.masm\n");
        return EXIT_FAILURE;
    }
    
    char *src = read_file_string(argv[1]);
    if (src == NULL) {
        fprintf(stderr, "ERROR: File %s not found.\n", argv[1]);
        return EXIT_FAILURE;
    }

    LexerState lexer = lexer_new(src);
    lexer_scan_tokens(&lexer);

    if (!lexer.had_error) {
        printf("============TOKENS============\n");
        print_tokens(&lexer);

        ParserState parser = parser_new(src, lexer.tokens);
        parse_tokens(&parser);
        printf("\n============SYMBOLS============\n");
        print_symbols(&parser.symbols);
        printf("\n============AST============\n");
        print_ast(&parser);

        EncoderState encoder = encoder_new(parser.statements);
        encode_instructions(&encoder);

        int out_file_name_len = snprintf(NULL, 0, "%s.mano", argv[1]);
        char *out_file_name = malloc(out_file_name_len + 1);

        snprintf(out_file_name, out_file_name_len + 1, "%s.mano", argv[1]);
        write_buffer_be(encoder.memory, out_file_name);
    }
}