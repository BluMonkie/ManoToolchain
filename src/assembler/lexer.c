#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "lexer.h"

typedef struct {
    const char *str;
    TokenType type;
} Keyword;

static const Keyword KEYWORDS[] = {
    {"AND", TOKEN_AND},
    {"ADD", TOKEN_ADD},
    {"LDA", TOKEN_LDA},
    {"STA", TOKEN_STA},
    {"BUN", TOKEN_BUN},
    {"BSA", TOKEN_BSA},
    {"ISZ", TOKEN_ISZ},
    {"CLA", TOKEN_CLA},
    {"CLE", TOKEN_CLE},
    {"CMA", TOKEN_CMA},
    {"CME", TOKEN_CME},
    {"CIR", TOKEN_CIR},
    {"CIL", TOKEN_CIL},
    {"INC", TOKEN_INC},
    {"SPA", TOKEN_SPA},
    {"SNA", TOKEN_SNA},
    {"SZA", TOKEN_SZA},
    {"SZE", TOKEN_SZE},
    {"HLT", TOKEN_HLT},
    {"INP", TOKEN_INP},
    {"OUT", TOKEN_OUT},
    {"SKI", TOKEN_SKI},
    {"SKO", TOKEN_SKO},
    {"ION", TOKEN_ION},
    {"IOF", TOKEN_IOF},
    {"ORG", TOKEN_ORG},
    {"END", TOKEN_END},
    {"DEC", TOKEN_DEC},
    {"HEX", TOKEN_HEX},
    {"I", TOKEN_I}
};

static void scan_token(LexerState *state);
static void scan_identifier(LexerState *state);
static void scan_decimal_digit(LexerState *state);
static void scan_hex_digit(LexerState *state);

static char advance(LexerState *state);
static char peek(LexerState *state);

static void add_token(LexerState *state, TokenType type);
static void add_number_token(
    LexerState *state,
    TokenType type,
    int16_t number
);

static TokenType match_lexeme_type(
    const char *str,
    size_t len
);

static void skip_horizontal_whitespace(LexerState *state);

static bool is_alpha(char c);
static bool is_number(char c);
static bool is_hex_digit(char c);
static bool is_alphanumeric(char c);
static bool is_at_end(LexerState *state);

LexerState lexer_new(char *src) {
    Vector vec = vector_new(sizeof(Token));

    return (LexerState) {
        .src = src,
        .begin = 0,
        .current = 0,
        .tokens = vec,
        .had_error = false
    };
}

void lexer_scan_tokens(LexerState *state) {
    while (!is_at_end(state)) {
        state->begin = state->current;
        scan_token(state);
    }
    state->begin = state->current;
    add_token(state, TOKEN_EOF);
}

void scan_token(LexerState *state) {
    char c = advance(state);

    if (is_alpha(c)) {
        scan_identifier(state);
        return;
    }

    switch (c) {
        case ',':
            add_token(state, TOKEN_COMMA);
            break;
        case '/':
            while (!is_at_end(state) && peek(state) != '\n') {
                advance(state);
            }
            break;
        case ' ':
        case '\t':
        case '\r':
            break;
        case '\n':
            add_token(state, TOKEN_NEWLINE);
            break;   
        default:
    }
}

void scan_identifier(LexerState *state) {
    size_t length = 1;
    while (is_alphanumeric(peek(state))) {
        advance(state);
        length++;
    }

    if (length > 3) {
        state->had_error = true;
        fprintf(stderr, "ERROR: %.*s is not allowed. An identifier may only be three characters long at maximum.\n", (int) length, state->src + state->begin);
        return;
    }

    TokenType type = match_lexeme_type(state->src + state->begin, length);
    add_token(state, type);
    if (type == TOKEN_DEC) {
        skip_horizontal_whitespace(state);
        state->begin = state->current;
        scan_decimal_digit(state);
    } else if (type == TOKEN_HEX || type == TOKEN_ORG) {
        skip_horizontal_whitespace(state);
        state->begin = state->current;
        scan_hex_digit(state);
    }
}

void scan_decimal_digit(LexerState *state) {
    bool neg = false;

    if (peek(state) == '-') {
        neg = true;
        advance(state);
    }

    size_t digits_start = state->current;
    int64_t num = 0;

    while (is_number(peek(state))) {
        char c = advance(state);
        num = num * 10 + (c - '0');
    }

    int64_t limit = neg ? -(int64_t) INT16_MIN : INT16_MAX;

    if (state->current == digits_start) {
        state->had_error = true;
        fprintf(stderr, "ERROR: Expected decimal value.\n");
        return;
    }

    if (num > limit) {
        state->had_error = true;
        fprintf(
            stderr,
            "ERROR: Decimal value %.*s can't fit in a 16-bit signed word.\n",
            (int) (state->current - state->begin),
            state->src + state->begin
        );
        return;
    }

    if (neg) num = -num;

    add_number_token(state, TOKEN_DEC_NUMBER, (int16_t) num);
}

void scan_hex_digit(LexerState *state) {
    size_t digits_start = state->current;
    int64_t num = 0;

    while (is_hex_digit(peek(state))) {
        char c = advance(state);
        int digit;

        if (c >= '0' && c <= '9') {
            digit = c - '0';
        }
        else if (c >= 'A' && c <= 'F') {
            digit = c - 'A' + 10;
        } else {
            digit = c - 'a' + 10;
        }

        num = num * 16 + digit;
    }

    if (state->current == digits_start) {
        state->had_error = true;
        fprintf(stderr, "ERROR: Expected decimal value.\n");
        return;
    }

    if (num > UINT16_MAX) {
        state->had_error = true;
        fprintf(
            stderr,
            "ERROR: Hexadecimal value %.*s can't fit in a 16-bit word.\n",
            (int)(state->current - state->begin),
            state->src + state->begin
        );
        return;
    }

    add_number_token(state, TOKEN_HEX_NUMBER, (uint16_t)num);
}

char advance(LexerState *state) {
    return state->src[state->current++];
}

char peek(LexerState *state) {
    return state->src[state->current];
}

void add_token(LexerState *state, TokenType type) {
    Token token = {
        .lexeme_start = state->begin,
        .lexeme_end = state->current,
        .type = type,
    };

    vector_push(&state->tokens, &token);
}

void add_number_token(LexerState *state, TokenType type, int16_t number) {
    Token token = {
        .lexeme_start = state->begin,
        .lexeme_end = state->current,
        .type = type,
        .number = number
    };

    vector_push(&state->tokens, &token);
}

TokenType match_lexeme_type(const char *str, size_t len) {
    size_t length = sizeof(KEYWORDS) / sizeof(Keyword);

    for (size_t i = 0; i < length; i++) {
        const Keyword *keyword = &KEYWORDS[i];
        if (strlen(keyword->str) == len && strncmp(str, keyword->str, len) == 0) {
            return keyword->type;
        }
    }

    return TOKEN_IDENTIFIER;
}

void skip_horizontal_whitespace(LexerState *state) {
    while (peek(state) == ' ' || peek(state) == '\t' || peek(state) == '\r') {
        advance(state);
    }
}

bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_number(char c) {
    return c >= '0' && c <= '9';
}

bool is_hex_digit(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool is_alphanumeric(char c) {
    return is_alpha(c) || is_number(c);
}

bool is_at_end(LexerState *state) {
    return peek(state) == '\0';
}