#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <vadefs.h>

#include "parser.h"
#include "ast.h"
#include "defs.h"
#include "common/arena.h"
#include "common/vector.h"

static Stmt parse_decl(ParserState *state);
static Stmt parse_vardecl(ParserState *state);
static Stmt parse_fundecl(ParserState *state);
static Stmt parse_statement(ParserState *state);
static Stmt parse_asm(ParserState *state);
static Stmt parse_block(ParserState *state);
static Stmt parse_if_stmt(ParserState *state);
static Stmt parse_while_stmt(ParserState *state);

static Expr parse_expr(ParserState *state);
static Expr parse_assignment(ParserState *state);
static Expr parse_comparison(ParserState *state);
static Expr parse_bitwise_expr(ParserState *state);
static Expr parse_shift_expr(ParserState *state);
static Expr parse_term(ParserState *state);
static Expr parse_factor(ParserState *state);
static Expr parse_unary(ParserState *state);
static Expr parse_primary(ParserState *state);
static Expr parse_identifier_or_call(ParserState *state, bool is_builtin);

static Token *advance(ParserState *state);
static Token *peek(ParserState *state);
static Token* peek_next(ParserState *state);
static Token* peek_next_next(ParserState *state);
static bool check(ParserState *state, TokenType type);
static bool match(ParserState *state, TokenType type);
static bool is_at_end(ParserState *state);

static Token *previous(ParserState *state);
static Token *consume(ParserState *state, TokenType type);

static void error(ParserState *state, int line, const char *format, ...);

ParserState parser_new() {
    return (ParserState) {
        .current = 0,
        .had_error = false,
        .arena = arena_new(),
        .statements = vector_new(sizeof(Stmt))
    };
}


Program parse_program(ParserState *state, const char *src, Vector tokens) {
    state->src = src;
    state->tokens = tokens;
    state->current = 0;

    while (!is_at_end(state)) {
        Stmt stmt = parse_decl(state);

        vector_push(&state->statements, &stmt);
    }

    return (Program) {
        .statements = state->statements,
    };
}

Program parse_program_builtin(ParserState *state, const char *src, Vector tokens) {
    Stmt begin = (Stmt) { .type = STMT_PSEUDO_EN_BUILTIN };
    Stmt end = (Stmt) { .type = STMT_PSEUDO_DIS_BUILTIN };
    
    vector_push(&state->statements, &begin);
    parse_program(state, src, tokens);
    vector_push(&state->statements, &end);

    return (Program) {
        .statements = state->statements,
    };
}

static Stmt parse_decl(ParserState *state) {
    if (match(state, TOKEN_VAR)) {
        return parse_vardecl(state);
    } else if (match(state, TOKEN_FUN)) {
        return parse_fundecl(state);
    } else {
        return parse_statement(state);
    }
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

static Stmt parse_fundecl(ParserState *state) {
    FunDeclStmt stmt = {0};

    stmt.identifier = consume(state, TOKEN_IDENTIFIER);
    stmt.parameters = vector_new(sizeof(FunParameter));

    consume(state, TOKEN_LEFT_PAREN);
    while (!is_at_end(state) && !match(state, TOKEN_RIGHT_PAREN)) {
        Token *id = consume(state, TOKEN_IDENTIFIER);
        consume(state, TOKEN_COLON);
        Token *type = consume(state, TOKEN_IDENTIFIER);
        if (peek(state)->type != TOKEN_RIGHT_PAREN) {
            consume(state, TOKEN_COMMA);
        }
        
        FunParameter param = {
            .identifier = id,
            .type_identifier = type,
        };
        vector_push(&stmt.parameters, &param);
    }

    if (match(state, TOKEN_ARROW)) {
        Token *id = consume(state, TOKEN_IDENTIFIER);
        consume(state, TOKEN_COLON);
        Token *type = consume(state, TOKEN_IDENTIFIER);

        FunParameter param = {
            .identifier = id,
            .type_identifier = type,
        };
        stmt.output = arena_alloc_type(&state->arena, sizeof(param), &param);
    }

    consume(state, TOKEN_LEFT_BRACE);
    Stmt body = parse_block(state);
    stmt.body = arena_alloc_type(&state->arena, sizeof(BlockStmt), &body.block);

    return (Stmt) {
        .type = STMT_FUNDECL,
        .fun_decl = stmt,
    };
}

static Stmt parse_statement(ParserState *state) {
    if (match(state, TOKEN_ASM)) {
        return parse_asm(state);
    } else if (match(state, TOKEN_LEFT_BRACE)) {
        return parse_block(state);
    } else if (match(state, TOKEN_IF)) {
        return parse_if_stmt(state);
    } else if (match(state, TOKEN_WHILE)) {
        return parse_while_stmt(state);
    }

    Expr parsed = parse_expr(state);
    Expr *expr = arena_alloc_type(&state->arena, sizeof(parsed), &parsed);
    Stmt stmt = (Stmt) {
        .type = STMT_EXPRESSION,
        .expr = { expr },
    };

    consume(state, TOKEN_SEMICOLON);
    return stmt;
}

static Stmt parse_asm(ParserState *state) {
    consume(state, TOKEN_LEFT_BRACE);
    Token *asm_block = consume(state, TOKEN_ASM_BLOCK);
    consume(state, TOKEN_RIGHT_BRACE);

    return (Stmt) {
        .type = STMT_ASM,
        .asm = {
            .asm = asm_block,
        }
    };
}

static Stmt parse_block(ParserState *state) {
    Vector stmts = vector_new(sizeof(Stmt));

    while (!is_at_end(state) && !match(state, TOKEN_RIGHT_BRACE)) {
        Stmt stmt = parse_decl(state);
        vector_push(&stmts, &stmt);
    }

    return (Stmt) {
        .type = STMT_BLOCK,
        .block = {
            .stmts = stmts,
        }
    };
}

static Stmt parse_if_stmt(ParserState *state) {
    Expr expr = parse_expr(state);

    consume(state, TOKEN_LEFT_BRACE);
    Stmt then_arm = parse_block(state);

    Stmt *else_arm = nullptr;

    if (match(state, TOKEN_ELSE)) {
        if (match(state, TOKEN_IF)) {
            Stmt stmt = parse_if_stmt(state);
            else_arm = arena_alloc_type(&state->arena, sizeof(stmt), &stmt);
        } else {
            consume(state, TOKEN_LEFT_BRACE);
            Stmt stmt = parse_block(state);
            else_arm = arena_alloc_type(&state->arena, sizeof(stmt), &stmt);
        }
    }

    return (Stmt) {
        .type = STMT_IF,
        .if_stmt = {
            .condition = arena_alloc_type(&state->arena, sizeof(expr), &expr),
            .then_arm = arena_alloc_type(&state->arena, sizeof(then_arm), &then_arm),
            .else_arm = else_arm,
        }
    };
}

static Stmt parse_while_stmt(ParserState *state) {
    Expr expr = parse_expr(state);

    consume(state, TOKEN_LEFT_BRACE);
    Stmt body = parse_block(state);

    return (Stmt) {
        .type = STMT_WHILE,
        .while_stmt = {
            .condition = arena_alloc_type(&state->arena, sizeof(expr), &expr),
            .body = arena_alloc_type(&state->arena, sizeof(body), &body),
        }
    };
}

static Expr parse_expr(ParserState *state) {
    Expr expr = parse_assignment(state);
    return expr;
}

static Expr parse_assignment(ParserState *state) {
    bool is_builtin_assign = check(state, TOKEN_AT) && 
        peek_next(state) != nullptr && peek_next(state)->type == TOKEN_IDENTIFIER && 
        peek_next_next(state) != nullptr && peek_next_next(state)->type == TOKEN_EQUAL;

    bool is_normal_assign = check(state, TOKEN_IDENTIFIER) && 
        peek_next(state) != nullptr && peek_next(state)->type == TOKEN_EQUAL;

    if (is_builtin_assign || is_normal_assign) {
        if (is_builtin_assign) {
            advance(state);
        }

        Token *id = peek(state);

        advance(state);
        advance(state);

        Expr expr = parse_assignment(state);

        return (Expr) {
            .type = EXPR_ASSIGNMENT,
            .assignment = {
                .id = id,
                .value = arena_alloc_type(&state->arena, sizeof(expr), &expr),
                .builtin = is_builtin_assign,
            }
        };
    }

    return parse_comparison(state);
}

static Expr parse_comparison(ParserState *state) {
    Expr expr = parse_bitwise_expr(state);

    if (match(state, TOKEN_LESS) || match(state, TOKEN_GREATER) || match(state, TOKEN_LESS_EQUAL) || match(state, TOKEN_GREATER_EQUAL) || match(state, TOKEN_EQUAL_EQUAL) || match(state, TOKEN_BANG_EQUAL)) {
        Token *tok = previous(state);
        
        Expr right = parse_bitwise_expr(state);

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

static Expr parse_bitwise_expr(ParserState *state) {
    Expr expr = parse_shift_expr(state);

    while (match(state, TOKEN_AMP) || match(state, TOKEN_PIPE) || match(state, TOKEN_CARET)) {
        Token *tok = previous(state);
        
        Expr right = parse_shift_expr(state);

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

static Expr parse_shift_expr(ParserState *state) {
    Expr expr = parse_term(state);

    while (match(state, TOKEN_LESS_LESS) || match(state, TOKEN_GREATER_GREATER)) {
        Token *tok = previous(state);
        
        Expr right = parse_term(state);

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

static Expr parse_term(ParserState *state) {
    Expr expr = parse_factor(state);

    while (match(state, TOKEN_PLUS) || match(state, TOKEN_MINUS)) {
        Token *tok = previous(state);
        
        Expr right = parse_factor(state);

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

static Expr parse_factor(ParserState *state) {
    Expr expr = parse_unary(state);

    while (match(state, TOKEN_STAR) || match(state, TOKEN_SLASH) || match(state, TOKEN_PERCENT)) {
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
        return parse_identifier_or_call(state, false);
    } else if (match(state, TOKEN_AT)) {
        consume(state, TOKEN_IDENTIFIER);
        return parse_identifier_or_call(state, true);
    } else if (match(state, TOKEN_NUMBER)) {
        return (Expr) {
            .type = EXPR_LITERAL,
            .literal = {
                .type = LIT_NUMBER,
                .num = previous(state)->data.number,
            }
        };
    } else {
        error(state, peek(state)->line, "Unexpected token: %s", token_type_lexeme(peek(state)->type));
        advance(state);
        return (Expr){0};
    }
}

static Expr parse_identifier_or_call(ParserState *state, bool is_builtin) {
    Token *id = previous(state);

    if (!match(state, TOKEN_LEFT_PAREN)) return (Expr) {
        .type = EXPR_IDENTIFIER,
        .identifier = {
            .id = id,
            .builtin = is_builtin,
        }
    };

    Vector arguments = vector_new(sizeof(Expr));

    while (!is_at_end(state) && !match(state, TOKEN_RIGHT_PAREN)) {
        Expr expr = parse_expr(state);
        vector_push(&arguments, &expr);

        if (peek(state)->type != TOKEN_RIGHT_PAREN) {
            consume(state, TOKEN_COMMA);
        }
    }

    return (Expr) {
        .type = EXPR_CALL,
        .call = {
            .callee = id,
            .arguments = arguments,
            .builtin = is_builtin,
        }
    };
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

static Token* peek_next_next(ParserState *state) {
    if (state->current + 2 >= state->tokens.size) return nullptr;
    return (Token *) vector_at(&state->tokens, state->current + 2);
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
    Token *token = peek(state);
    error(state, token->line, "ERROR while parsing: Expected %s, got %s.\n", token_type_lexeme(type), token_type_lexeme(peek(state)->type));
    return nullptr;
}

static bool is_at_end(ParserState *state) {
    return check(state, TOKEN_EOF);
}

static void error(ParserState *state, int line, const char *msg, ...) {
    state->had_error = true;

    va_list args;
    va_start(args, msg);

    fprintf(stderr, "ERROR while parsing at [line %d]: ", line);
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");

    va_end(args);
}