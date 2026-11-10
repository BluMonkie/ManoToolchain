#include <stdio.h>
#include <inttypes.h>

#include "debug.h"
#include "defs.h"

static const char *const TOKEN_TYPE_NAMES[] = {
    [TOKEN_PLUS] = "TOKEN_PLUS",
    [TOKEN_MINUS] = "TOKEN_MINUS",
    [TOKEN_STAR] = "TOKEN_STAR",
    [TOKEN_PERCENT] = "TOKEN_PERCENT",
    [TOKEN_AT] = "TOKEN_AT",

    [TOKEN_SLASH] = "TOKEN_SLASH",
    [TOKEN_EQUAL] = "TOKEN_EQUAL",
    [TOKEN_EQUAL_EQUAL] = "TOKEN_EQUAL_EQUAL",

    [TOKEN_IDENTIFIER] = "TOKEN_IDENTIFIER",
    [TOKEN_NUMBER] = "TOKEN_NUMBER",

    [TOKEN_VAR] = "TOKEN_VAR",

    [TOKEN_COLON] = "TOKEN_COLON",
    [TOKEN_SEMICOLON] = "TOKEN_SEMICOLON",

    [TOKEN_EOF] = "TOKEN_EOF",
};

void print_tokens(FILE *stream, const Vector tokens) {
    for (size_t i = 0; i < tokens.size; i++) {
        const Token *token = vector_at_const(&tokens, i);

        fprintf(
            stream,
            "%-20s \"%.*s\"",
            token_type_name(token->type),
            (int)token->lexeme.len,
            token->lexeme.start
        );

        switch (token->type) {
            case TOKEN_NUMBER:
                fprintf(stream, "  %" PRId64, token->data.number);
                break;

            default:
                break;
        }

        fprintf(stream, "\n");
    }
}