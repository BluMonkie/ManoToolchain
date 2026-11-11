#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "lexer.h"
#include "defs.h"

static void scan_token(LexerState *state);
static void scan_identifier(LexerState *state);
static void scan_number(LexerState *state, char first);
static void skip_single_comment(LexerState *state);

static char advance(LexerState *state);
static char peek(LexerState *state);
static bool check(LexerState *state, char c);
static bool match(LexerState *state, char c);

static void add_token(LexerState *state, TokenType type);
static void add_number_token(LexerState *state, TokenType type, int64_t number);

static TokenType match_lexeme_type(const char *str, size_t len);

static bool is_alpha(char c);
static bool is_number(char c);
static bool is_hex_digit(char c);
static bool is_alphanumeric(char c);
static bool is_at_end(LexerState *state);

static void error(LexerState *state, const char *msg, ...);

typedef struct {
    const char *str;
    TokenType type;
} Keyword;

static const Keyword KEYWORDS[] = {
    {"var", TOKEN_VAR},
};

LexerState lexer_new(const char *src) {
    return (LexerState) {
        .src = src,
        .begin = 0,
        .current = 0,
        .tokens = vector_new(sizeof(Token)),
        .had_error = false,
    };
}

void scan_tokens(LexerState *state) {
    while (!is_at_end(state)) {
        state->begin = state->current;
        scan_token(state);
    }
    add_token(state, TOKEN_EOF);
}

static void scan_token(LexerState *state) {
    char c = advance(state);

    if (is_alpha(c)) {
        scan_identifier(state);
        return;
    }

    if (is_number(c)) {
        scan_number(state, c);
        return;
    }

    switch (c) {
        case '+':
            add_token(state, TOKEN_PLUS);
            break;
        case '-':
            add_token(state, TOKEN_MINUS);
            break;
        case '*':
            add_token(state, TOKEN_STAR);
            break;
        case '/':
            if (match(state, '/')) {
                skip_single_comment(state);
            } else {
                add_token(state, TOKEN_SLASH);
            }
            break;
        case '%':
            add_token(state, TOKEN_PERCENT);
            break;
        case '~':
            add_token(state, TOKEN_TILDE);
            break;
        case '=':
            if (match(state, '=')) {
                add_token(state, TOKEN_EQUAL_EQUAL);
            } else {
                add_token(state, TOKEN_EQUAL);
            }
            break;
        case '&':
            add_token(state, TOKEN_AMP);
            break;
        case '|':
            add_token(state, TOKEN_PIPE);
            break;
        case '^':
            add_token(state, TOKEN_CARET);
            break;
        case '@':
            add_token(state, TOKEN_AT);
            break;
        case ':':
            add_token(state, TOKEN_COLON);
            break;
        case ';':
            add_token(state, TOKEN_SEMICOLON);
            break;
        case '(':
            add_token(state, TOKEN_LEFT_PAREN);
            break;
        case ')':
            add_token(state, TOKEN_RIGHT_PAREN);
            break;
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            break;
        default:
            error(state, "Unexpected character: %c", c);
            break;
    }
}

static void scan_identifier(LexerState *state) {
    while (is_alphanumeric(peek(state))) {
        advance(state);
    }

    const char *start = state->src + state->begin;
    size_t len = state->current - state->begin;

    TokenType type = match_lexeme_type(start, len);

    add_token(state, type);
}

static void scan_number(LexerState *state, char first) {
    int64_t num = first - '0';

    while (is_number(peek(state))) {
        char c = advance(state);
        num = num * 10 + (c - '0');
    }

    add_number_token(state, TOKEN_NUMBER, num);
}

static void skip_single_comment(LexerState *state) {
    while (!is_at_end(state) && !check(state, '\n')) {
        advance(state);
    }
}

static char advance(LexerState *state) {
    return state->src[state->current++];
}

static char peek(LexerState *state) {
    return state->src[state->current];
}

static bool check(LexerState *state, char c) {
    return peek(state) == c;
}

static bool match(LexerState *state, char c) {
    if (check(state, c)) {
        advance(state);
        return true;
    }
    return false;
}

static void add_token(LexerState *state, TokenType type) {
    Token token = {
        .lexeme = (StringSlice) {
            .start = state->src + state->begin,
            .len = state->current - state->begin,
        },
        .type = type,
    };

    vector_push(&state->tokens, &token);
}

static void add_number_token(LexerState *state, TokenType type, int64_t number) {
    Token token = {
        .lexeme = (StringSlice) {
            .start = state->src + state->begin,
            .len = state->current - state->begin,
        },
        .type = type,
        .data = {
            .number = number,
        }
    };

    vector_push(&state->tokens, &token);
}

static TokenType match_lexeme_type(const char *str, size_t len) {
    size_t length = sizeof(KEYWORDS) / sizeof(Keyword);

    for (size_t i = 0; i < length; i++) {
        const Keyword *keyword = &KEYWORDS[i];
        if (strlen(keyword->str) == len && strncmp(str, keyword->str, len) == 0) {
            return keyword->type;
        }
    }

    return TOKEN_IDENTIFIER;
}

static bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool is_number(char c) {
    return c >= '0' && c <= '9';
}

static bool is_hex_digit(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static bool is_alphanumeric(char c) {
    return is_alpha(c) || is_number(c);
}

static bool is_at_end(LexerState *state) {
    return peek(state) == '\0';
}

static void error(LexerState *state, const char *msg, ...) {
    state->had_error = true;

    va_list args;
    va_start(args, msg);

    fprintf(stderr, "ERROR: ");
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");

    va_end(args);
}