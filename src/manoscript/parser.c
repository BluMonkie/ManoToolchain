#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <vadefs.h>

#include "parser.h"
#include "ast.h"
#include "defs.h"
#include "common/arena.h"
#include "common/vector.h"

static Stmt parse_vardecl(ParserState *state);
static Stmt parse_statement(ParserState *state);
static Expr parse_expr(ParserState *state);
static Expr parse_assignment(ParserState *state);
static Expr parse_term(ParserState *state);
static Expr parse_unary(ParserState *state);
static Expr parse_primary(ParserState *state);

static Token *advance(ParserState *state);
static Token *peek(ParserState *state);
static Token* peek_next(ParserState *state);
static bool check(ParserState *state, TokenType type);
static bool match(ParserState *state, TokenType type);
static bool is_at_end(ParserState *state);

static Token *previous(ParserState *state);
static Token *consume(ParserState *state, TokenType type);

static void error(ParserState *state, const char *format, ...);

ParserState parser_new(char *src, Vector tokens) {
    return (ParserState) {
        .src = src,
        .current = 0,
        .tokens = tokens,
        .had_error = false,
        .arena = arena_new(),
    };
}


Program parse_program(ParserState *state) {
    Vector vec = vector_new(sizeof(Stmt));
    
    while (!is_at_end(state)) {
        Stmt stmt;
        if (match(state, TOKEN_VAR)) {
            stmt = parse_vardecl(state);
        } else {
            stmt = parse_statement(state);
        }

        vector_push(&vec, &stmt);
    }

    return (Program) {
        .statements = vec,
    };
}

static Stmt parse_vardecl(ParserState *state) {
    Token *id = consume(state, TOKEN_IDENTIFIER);
    consume(state, TOKEN_COLON);
    Token *type = consume(state, TOKEN_IDENTIFIER);

    Expr *init = nullptr;
    if (match(state, TOKEN_EQUAL)) {
        Expr expr = parse_expr(state);
        init = arena_alloc_type(&state->arena, sizeof(expr), &expr);
    }

    Stmt stmt = {
        .type = STMT_VARDECL,
        .var_decl = {
            .identifier = id,
            .type_identifier = type,
            .initializer = init,
        }
    };
    consume(state, TOKEN_SEMICOLON);
    return stmt;
}

static Stmt parse_statement(ParserState *state) {
    Expr parsed = parse_expr(state);
    Expr *expr = arena_alloc_type(&state->arena, sizeof(parsed), &parsed);
    Stmt stmt = (Stmt) {
        .type = STMT_EXPRESSION,
        .expr = { expr },
    };

    consume(state, TOKEN_SEMICOLON);
    return stmt;
}

static Expr parse_expr(ParserState *state) {
    Expr expr = parse_assignment(state);
    return expr;
}

static Expr parse_assignment(ParserState *state) {
    if (peek(state)->type == TOKEN_IDENTIFIER && peek_next(state) != nullptr && peek_next(state)->type == TOKEN_EQUAL) {
        Token *id = peek(state);

        advance(state);
        advance(state);

        Expr expr = parse_assignment(state);

        return (Expr) {
            .type = EXPR_ASSIGNMENT,
            .assignment = {
                .id = id,
                .value = arena_alloc_type(&state->arena, sizeof(expr), &expr),
            }
        };
    }

    return parse_term(state);
}

static Expr parse_term(ParserState *state) {
    Expr expr = parse_unary(state);

    while (match(state, TOKEN_PLUS) || match(state, TOKEN_MINUS)) {
        Token *tok = previous(state);
        
        Expr right = parse_unary(state);

        expr = (Expr) {
            .type = EXPR_BINARY,
            .binary = {
                .left = arena_alloc_type(&state->arena, sizeof(expr), &expr),
                .op = tok,
                .right = arena_alloc_type(&state->arena, sizeof(right), &right),
            }
        };
    }
    
    return expr;
}

static Expr parse_unary(ParserState *state) {
    if (check(state, TOKEN_PLUS) || check(state, TOKEN_MINUS) || check(state, TOKEN_TILDE)) {
        Token *tok = advance(state);

        Expr expr = parse_unary(state);

        return (Expr) {
            .type = EXPR_UNARY,
            .unary = {
                .op = tok,
                .operand = arena_alloc_type(&state->arena, sizeof(expr), &expr),
            }
        };
    }

    return parse_primary(state);
}

static Expr parse_primary(ParserState *state) {
    if (match(state, TOKEN_LEFT_PAREN)) {
        Expr expr = parse_expr(state);
        Expr *ptr = arena_alloc_type(&state->arena, sizeof(expr), &expr);

        consume(state, TOKEN_RIGHT_PAREN);
        return (Expr) {
            .type = EXPR_GROUPING,
            .grouping = {
                .expr = ptr,
            }
        };
    } else if (match(state, TOKEN_IDENTIFIER)) {
        return (Expr) {
            .type = EXPR_IDENTIFIER,
            .identifier = {
                .id = previous(state),
            }
        };
    } else if (match(state, TOKEN_NUMBER)) {
        return (Expr) {
            .type = EXPR_LITERAL,
            .literal = {
                .type = LIT_NUMBER,
                .num = previous(state)->data.number,
            }
        };
    } else {
        error(state, "Unexpected token: %s", token_type_name(peek(state)->type));
        advance(state);
        return (Expr){0};
    }
}

static Token *advance(ParserState *state) {
    return vector_at(&state->tokens, state->current++);
}

static Token* peek(ParserState *state) {
    return (Token *) vector_at(&state->tokens, state->current);
}

static Token* peek_next(ParserState *state) {
    if (state->current + 1 >= state->tokens.size) return nullptr;
    return (Token *) vector_at(&state->tokens, state->current + 1);
}

static bool check(ParserState *state, TokenType type) {
    return peek(state)->type == type;
}

static bool match(ParserState *state, TokenType type) {
    if (check(state, type)) {
        advance(state);
        return true;
    }
    return false;
}

Token *previous(ParserState *state) {
    return vector_at(&state->tokens, state->current - 1);
}

Token *consume(ParserState *state, TokenType type) {
    if (match(state, type)) {
        return previous(state);
    }
    state->had_error = true;
    fprintf(stderr, "ERROR while parsing: Expected %s, got %s.\n", token_type_name(type), token_type_name(peek(state)->type));   
    return NULL;
}

static bool is_at_end(ParserState *state) {
    return check(state, TOKEN_EOF);
}

static void error(ParserState *state, const char *format, ...) {
    state->had_error = true;

    va_list args;

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}