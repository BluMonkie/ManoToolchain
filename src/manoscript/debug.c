#include <stdio.h>

#include "debug.h"
#include "ast.h"
#include "defs.h"
#include "common/vector.h"

static void print_expr(FILE *stream, Expr *expr, const char *prefix, bool last);
static void print_stmt(FILE *stream, Stmt *stmt, const char *prefix, bool last);

static const char *const TOKEN_TYPE_NAMES[] = {
    // Operators
    // Single character
    [TOKEN_PLUS] = "TOKEN_PLUS",
    [TOKEN_MINUS] = "TOKEN_MINUS",
    [TOKEN_STAR] = "TOKEN_STAR",
    [TOKEN_PERCENT] = "TOKEN_PERCENT",
    [TOKEN_TILDE] = "TOKEN_TILDE",
    [TOKEN_CARET] = "TOKEN_CARET",

    // One or two characters
    [TOKEN_ARROW] = "TOKEN_ARROW",
    [TOKEN_SLASH] = "TOKEN_SLASH",
    [TOKEN_EQUAL] = "TOKEN_EQUAL",
    [TOKEN_EQUAL_EQUAL] = "TOKEN_EQUAL_EQUAL",
    [TOKEN_BANG_EQUAL] = "TOKEN_BANG_EQUAL",
    [TOKEN_AMP] = "TOKEN_AMP",
    [TOKEN_PIPE] = "TOKEN_PIPE",
    [TOKEN_LESS] = "TOKEN_LESS",
    [TOKEN_GREATER] = "TOKEN_GREATER",
    [TOKEN_LESS_EQUAL] = "TOKEN_LESS_EQUAL",
    [TOKEN_GREATER_EQUAL] = "TOKEN_GREATER_EQUAL",

    // Special operators
    [TOKEN_AT] = "TOKEN_AT",

    // Literals and identifiers
    [TOKEN_IDENTIFIER] = "TOKEN_IDENTIFIER",
    [TOKEN_NUMBER] = "TOKEN_NUMBER",

    // Keywords
    [TOKEN_VAR] = "TOKEN_VAR",
    [TOKEN_FUN] = "TOKEN_FUN",
    [TOKEN_ASM] = "TOKEN_ASM",
    [TOKEN_IF] = "TOKEN_IF",
    [TOKEN_ELSE] = "TOKEN_ELSE",

    // Punctuation
    [TOKEN_COMMA] = "TOKEN_COMMA",
    [TOKEN_COLON] = "TOKEN_COLON",
    [TOKEN_SEMICOLON] = "TOKEN_SEMICOLON",
    [TOKEN_LEFT_PAREN] = "TOKEN_LEFT_PAREN",
    [TOKEN_RIGHT_PAREN] = "TOKEN_RIGHT_PAREN",
    [TOKEN_LEFT_BRACE] = "TOKEN_LEFT_BRACE",
    [TOKEN_RIGHT_BRACE] = "TOKEN_RIGHT_BRACE",

    // Special tokens
    [TOKEN_ASM_BLOCK] = "TOKEN_ASM_BLOCK",
    [TOKEN_EOF] = "TOKEN_EOF",
};

// void print_tokens(FILE *stream, const Vector tokens) {
//     for (size_t i = 0; i < tokens.size; i++) {
//         const Token *token = vector_at_const(&tokens, i);

//         fprintf(
//             stream,
//             "%-20s \"%.*s\"",
//             token_type_name(token->type),
//             (int)token->lexeme.len,
//             token->lexeme.start
//         );

//         switch (token->type) {
//             case TOKEN_NUMBER:
//                 fprintf(stream, "  %" PRId64, token->data.number);
//                 break;

//             default:
//                 break;
//         }

//         fprintf(stream, "\n");
//     }
// }

const char *token_type_name(TokenType type) {
    return TOKEN_TYPE_NAMES[type];
}

void print_tokens(FILE *stream, const Vector tokens) {
    for (size_t i = 0; i < tokens.size; i++) {
        const Token *token = vector_at_const(&tokens, i);

        size_t length = token->lexeme.len;

        fprintf(
            stream,
            "TOKEN: %-12s | Lexeme = %.*s",
            token_type_name(token->type),
            (int)length,
            token->lexeme.start
        );

        fputc('\n', stream);
    }
}

static void print_node(FILE *stream, const char *prefix, bool last, const char *label) {
    fprintf(stream, "%s%s%s\n", prefix, last ? "`-- " : "|-- ", label);
}

static void print_expr(FILE *stream, Expr *expr, const char *prefix, bool last) {
    if (!expr) {
        print_node(stream, prefix, last, "<null>");
        return;
    }

    char label[128];

    switch (expr->type) {
        case EXPR_BINARY:
            snprintf(label, sizeof(label), "BinaryExpr: %s",
                expr->binary.op ? token_type_lexeme(expr->binary.op->type) : "<null>");

            print_node(stream, prefix, last, label);

            {
                char child_prefix[1024];
                snprintf(child_prefix, sizeof(child_prefix),
                    "%s%s", prefix, last ? "    " : "|   ");

                print_node(stream, child_prefix, false, "Left");
                char left_prefix[2048];
                snprintf(left_prefix, sizeof(left_prefix),
                    "%s|   ", child_prefix);
                print_expr(stream, expr->binary.left, left_prefix, true);

                print_node(stream, child_prefix, true, "Right");
                char right_prefix[2048];
                snprintf(right_prefix, sizeof(right_prefix),
                    "%s    ", child_prefix);
                print_expr(stream, expr->binary.right, right_prefix, true);
            }
            break;

        case EXPR_UNARY:
            snprintf(label, sizeof(label), "UnaryExpr: %s",
                token_type_lexeme(expr->unary.op->type));

            print_node(stream, prefix, last, label);

            {
                char child_prefix[1024];
                snprintf(child_prefix, sizeof(child_prefix),
                    "%s%s", prefix, last ? "    " : "|   ");

                print_expr(stream, expr->unary.operand, child_prefix, true);
            }
            break;

        case EXPR_GROUPING:
            print_node(stream, prefix, last, "Grouping");

            {
                char child_prefix[1024];
                snprintf(child_prefix, sizeof(child_prefix),
                    "%s%s", prefix, last ? "    " : "|   ");

                print_expr(stream, expr->grouping.expr, child_prefix, true);
            }
            break;

        case EXPR_ASSIGNMENT:
            if (expr->assignment.id) {
                snprintf(label, sizeof(label), "Assignment: %.*s",
                    (int)expr->assignment.id->lexeme.len,
                    expr->assignment.id->lexeme.start);
            } else {
                snprintf(label, sizeof(label), "Assignment: <null>");
            }

            print_node(stream, prefix, last, label);

            {
                char child_prefix[1024];
                snprintf(child_prefix, sizeof(child_prefix),
                    "%s%s", prefix, last ? "    " : "|   ");

                print_expr(stream, expr->assignment.value, child_prefix, true);
            }
            break;

        case EXPR_IDENTIFIER:
            if (expr->identifier.id) {
                snprintf(label, sizeof(label), "Identifier: %.*s",
                    (int)expr->identifier.id->lexeme.len,
                    expr->identifier.id->lexeme.start);
            } else {
                snprintf(label, sizeof(label), "Identifier: <null>");
            }

            print_node(stream, prefix, last, label);
            break;

        case EXPR_LITERAL:
            switch (expr->literal.type) {
                case LIT_NUMBER:
                    snprintf(label, sizeof(label), "Number: %d",
                        expr->literal.num);
                    break;
            }

            print_node(stream, prefix, last, label);
            break;

        case EXPR_CALL:
            if (expr->call.callee) {
                snprintf(label, sizeof(label), "Call: %.*s",
                    (int)expr->call.callee->lexeme.len,
                    expr->call.callee->lexeme.start);
            } else {
                snprintf(label, sizeof(label), "Call: <null>");
            }

            print_node(stream, prefix, last, label);

            if (expr->call.arguments.size > 0) {
                char child_prefix[1024];
                snprintf(child_prefix, sizeof(child_prefix),
                    "%s%s", prefix, last ? "    " : "|   ");

                for (size_t i = 0; i < expr->call.arguments.size; i++) {
                    Expr *arg = vector_at(&expr->call.arguments, i);
                    print_expr(stream, arg, child_prefix, i == expr->call.arguments.size - 1);
                }
            }
            break;
    }
}

static void print_stmt(FILE *stream, Stmt *stmt, const char *prefix, bool last) {
    if (!stmt) {
        print_node(stream, prefix, last, "<null>");
        return;
    }

    char label[128];

    switch (stmt->type) {
        case STMT_VARDECL: {
            snprintf(label, sizeof(label), "VarDecl");

            print_node(stream, prefix, last, label);

            char child_prefix[1024];
            snprintf(child_prefix, sizeof(child_prefix),
                "%s%s", prefix, last ? "    " : "|   ");

            char value[256];

            if (stmt->var_decl.identifier) {
                Token *id = stmt->var_decl.identifier;
                snprintf(value, sizeof(value),
                    "Identifier: %.*s",
                    (int)id->lexeme.len, id->lexeme.start);
            } else {
                snprintf(value, sizeof(value), "Identifier: <null>");
            }

            print_node(stream, child_prefix, false, value);

            if (stmt->var_decl.type_identifier) {
                Token *type = stmt->var_decl.type_identifier;
                snprintf(value, sizeof(value),
                    "Type: %.*s",
                    (int)type->lexeme.len, type->lexeme.start);
            } else {
                snprintf(value, sizeof(value), "Type: <null>");
            }

            print_node(stream, child_prefix,
                stmt->var_decl.initializer == NULL, value);

            if (stmt->var_decl.initializer) {
                print_node(stream, child_prefix, true, "Initializer");

                char init_prefix[2048];
                snprintf(init_prefix, sizeof(init_prefix),
                    "%s    ", child_prefix);

                print_expr(stream, stmt->var_decl.initializer,
                    init_prefix, true);
            }

            break;
        }

        case STMT_FUNDECL: {
            snprintf(label, sizeof(label), "FunDecl");
            print_node(stream, prefix, last, label);

            char child_prefix[1024];
            snprintf(child_prefix, sizeof(child_prefix),
                "%s%s", prefix, last ? "    " : "|   ");

            char value[256];
            if (stmt->fun_decl.identifier) {
                Token *id = stmt->fun_decl.identifier;
                snprintf(value, sizeof(value),
                    "Identifier: %.*s",
                    (int)id->lexeme.len, id->lexeme.start);
            } else {
                snprintf(value, sizeof(value), "Identifier: <null>");
            }
            print_node(stream, child_prefix, false, value);

            if (stmt->fun_decl.parameters.size > 0) {
                print_node(stream, child_prefix, false, "Parameters");
                char params_prefix[2048];
                snprintf(params_prefix, sizeof(params_prefix), "%s|   ", child_prefix);

                for (size_t i = 0; i < stmt->fun_decl.parameters.size; i++) {
                    FunParameter *param = vector_at(&stmt->fun_decl.parameters, i);
                    char p_buf[256];
                    snprintf(p_buf, sizeof(p_buf), "%.*s: %.*s",
                        param->identifier ? (int)param->identifier->lexeme.len : 6,
                        param->identifier ? param->identifier->lexeme.start : "<null>",
                        param->type_identifier ? (int)param->type_identifier->lexeme.len : 6,
                        param->type_identifier ? param->type_identifier->lexeme.start : "<null>");
                    print_node(stream, params_prefix, i == stmt->fun_decl.parameters.size - 1, p_buf);
                }
            }

            if (stmt->fun_decl.output != NULL && stmt->fun_decl.output->identifier != NULL) {
                print_node(stream, child_prefix, false, "Return");
                char ret_prefix[2048];
                snprintf(ret_prefix, sizeof(ret_prefix), "%s|   ", child_prefix);

                char id_val[256], type_val[256];
                snprintf(id_val, sizeof(id_val), "Identifier: %.*s",
                    stmt->fun_decl.output->identifier ? (int)stmt->fun_decl.output->identifier->lexeme.len : 6,
                    stmt->fun_decl.output->identifier ? stmt->fun_decl.output->identifier->lexeme.start : "<null>");
                print_node(stream, ret_prefix, false, id_val);

                snprintf(type_val, sizeof(type_val), "Type: %.*s",
                    stmt->fun_decl.output->type_identifier ? (int)stmt->fun_decl.output->type_identifier->lexeme.len : 6,
                    stmt->fun_decl.output->type_identifier ? stmt->fun_decl.output->type_identifier->lexeme.start : "<null>");
                print_node(stream, ret_prefix, true, type_val);
            }

            if (stmt->fun_decl.body) {
                Stmt body_stmt = {
                    .type = STMT_BLOCK,
                    .block = *stmt->fun_decl.body,
                };
                print_stmt(stream, &body_stmt, child_prefix, true);
            } else {
                print_node(stream, child_prefix, true, "Body: <null>");
            }

            break;
        }

        case STMT_EXPRESSION: {
            print_node(stream, prefix, last, "ExpressionStatement");

            char child_prefix[1024];
            snprintf(child_prefix, sizeof(child_prefix),
                "%s%s", prefix, last ? "    " : "|   ");

            print_expr(stream, stmt->expr.expr, child_prefix, true);
            break;
        }

        case STMT_BLOCK: {
            print_node(stream, prefix, last, "Block");

            char child_prefix[1024];
            snprintf(child_prefix, sizeof(child_prefix),
                "%s%s", prefix, last ? "    " : "|   ");

            for (size_t i = 0; i < stmt->block.stmts.size; i++) {
                Stmt *child = vector_at(&stmt->block.stmts, i);
                print_stmt(stream, child, child_prefix, i == stmt->block.stmts.size - 1);
            }
            break;
        }

        case STMT_IF: {
            print_node(stream, prefix, last, "IfStmt");

            char child_prefix[1024];
            snprintf(child_prefix, sizeof(child_prefix),
                "%s%s", prefix, last ? "    " : "|   ");

            print_node(stream, child_prefix, false, "Condition");
            char cond_prefix[2048];
            snprintf(cond_prefix, sizeof(cond_prefix),
                "%s|   ", child_prefix);
            print_expr(stream, stmt->if_stmt.condition, cond_prefix, true);

            bool has_else = stmt->if_stmt.else_arm != NULL;

            print_node(stream, child_prefix, !has_else, "Then");
            char then_prefix[2048];
            snprintf(then_prefix, sizeof(then_prefix),
                "%s%s", child_prefix, !has_else ? "    " : "|   ");
            print_stmt(stream, stmt->if_stmt.then_arm, then_prefix, true);

            if (has_else) {
                print_node(stream, child_prefix, true, "Else");
                char else_prefix[2048];
                snprintf(else_prefix, sizeof(else_prefix),
                    "%s    ", child_prefix);
                print_stmt(stream, stmt->if_stmt.else_arm, else_prefix, true);
            }
            break;
        }

        case STMT_ASM: {
            print_node(stream, prefix, last, "AsmStmt");

            char child_prefix[1024];
            snprintf(child_prefix, sizeof(child_prefix),
                "%s%s", prefix, last ? "    " : "|   ");

            if (stmt->asm.asm) {
                const char *src = stmt->asm.asm->lexeme.start;
                size_t len = stmt->asm.asm->lexeme.len;

                size_t i = 0;
                while (i < len) {
                    while (i < len && (src[i] == '\r' || src[i] == '\n')) i++;
                    if (i >= len) break;

                    size_t line_start = i;
                    while (i < len && src[i] != '\r' && src[i] != '\n') i++;
                    size_t line_end = i;

                    while (line_start < line_end && (src[line_start] == ' ' || src[line_start] == '\t')) line_start++;
                    while (line_end > line_start && (src[line_end - 1] == ' ' || src[line_end - 1] == '\t')) line_end--;

                    if (line_end > line_start) {
                        size_t next = i;
                        while (next < len && (src[next] == ' ' || src[next] == '\t' || src[next] == '\r' || src[next] == '\n')) next++;
                        bool is_last = (next >= len);

                        char line_buf[256];
                        snprintf(line_buf, sizeof(line_buf), "%.*s", (int)(line_end - line_start), src + line_start);
                        print_node(stream, child_prefix, is_last, line_buf);
                    }
                }
            } else {
                print_node(stream, child_prefix, true, "<null>");
            }
            break;
        }
    }
}

void print_ast(Program *program, FILE *stream) {
    fprintf(stream, "Program\n");

    for (size_t i = 0; i < program->statements.size; i++) {
        Stmt *stmt = vector_at(&program->statements, i);

        print_stmt(stream, stmt, "", i == program->statements.size - 1);
    }
}