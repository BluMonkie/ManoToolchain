
#include <stdio.h>

#include "ast.h"
#include "defs.h"
#include "common/vector.h"

static void print_expr(Expr *expr, const char *prefix, bool last);
static void print_stmt(Stmt *stmt, const char *prefix, bool last);

static void print_node(const char *prefix, bool last, const char *label) {
    printf("%s%s%s\n", prefix, last ? "`-- " : "|-- ", label);
}

static void print_expr(Expr *expr, const char *prefix, bool last) {
    if (!expr) {
        print_node(prefix, last, "<null>");
        return;
    }

    char label[128];

    switch (expr->type) {
        case EXPR_BINARY:
            snprintf(label, sizeof(label), "BinaryExpr: %s",
                expr->binary.op ? token_type_name(expr->binary.op->type) : "<null>");

            print_node(prefix, last, label);

            {
                char child_prefix[1024];
                snprintf(child_prefix, sizeof(child_prefix),
                    "%s%s", prefix, last ? "    " : "|   ");

                print_node(child_prefix, false, "Left");
                char left_prefix[2048];
                snprintf(left_prefix, sizeof(left_prefix),
                    "%s|   ", child_prefix);
                print_expr(expr->binary.left, left_prefix, true);

                print_node(child_prefix, true, "Right");
                char right_prefix[2048];
                snprintf(right_prefix, sizeof(right_prefix),
                    "%s    ", child_prefix);
                print_expr(expr->binary.right, right_prefix, true);
            }
            break;

        case EXPR_UNARY:
            snprintf(label, sizeof(label), "UnaryExpr: %s",
                token_type_name(expr->unary.op->type));

            print_node(prefix, last, label);

            {
                char child_prefix[1024];
                snprintf(child_prefix, sizeof(child_prefix),
                    "%s%s", prefix, last ? "    " : "|   ");

                print_expr(expr->unary.operand, child_prefix, true);
            }
            break;

        case EXPR_GROUPING:
            print_node(prefix, last, "Grouping");

            {
                char child_prefix[1024];
                snprintf(child_prefix, sizeof(child_prefix),
                    "%s%s", prefix, last ? "    " : "|   ");

                print_expr(expr->grouping.expr, child_prefix, true);
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

            print_node(prefix, last, label);

            {
                char child_prefix[1024];
                snprintf(child_prefix, sizeof(child_prefix),
                    "%s%s", prefix, last ? "    " : "|   ");

                print_expr(expr->assignment.value, child_prefix, true);
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

            print_node(prefix, last, label);
            break;

        case EXPR_LITERAL:
            switch (expr->literal.type) {
                case LIT_NUMBER:
                    snprintf(label, sizeof(label), "Number: %d",
                        expr->literal.num);
                    break;
            }

            print_node(prefix, last, label);
            break;
    }
}

static void print_stmt(Stmt *stmt, const char *prefix, bool last) {
    if (!stmt) {
        print_node(prefix, last, "<null>");
        return;
    }

    char label[128];

    switch (stmt->type) {
        case STMT_VARDECL: {
            snprintf(label, sizeof(label), "VarDecl");

            print_node(prefix, last, label);

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

            print_node(child_prefix, false, value);

            if (stmt->var_decl.type_identifier) {
                Token *type = stmt->var_decl.type_identifier;
                snprintf(value, sizeof(value),
                    "Type: %.*s",
                    (int)type->lexeme.len, type->lexeme.start);
            } else {
                snprintf(value, sizeof(value), "Type: <null>");
            }

            print_node(child_prefix,
                stmt->var_decl.initializer == NULL, value);

            if (stmt->var_decl.initializer) {
                print_node(child_prefix, true, "Initializer");

                char init_prefix[2048];
                snprintf(init_prefix, sizeof(init_prefix),
                    "%s    ", child_prefix);

                print_expr(stmt->var_decl.initializer,
                    init_prefix, true);
            }

            break;
        }

        case STMT_EXPRESSION: {
            print_node(prefix, last, "ExpressionStatement");

            char child_prefix[1024];
            snprintf(child_prefix, sizeof(child_prefix),
                "%s%s", prefix, last ? "    " : "|   ");

            print_expr(stmt->expr.expr, child_prefix, true);
            break;
        }

        case STMT_BLOCK: {
            print_node(prefix, last, "Block");

            char child_prefix[1024];
            snprintf(child_prefix, sizeof(child_prefix),
                "%s%s", prefix, last ? "    " : "|   ");

            for (size_t i = 0; i < stmt->block.stmts.size; i++) {
                Stmt *child = vector_at(&stmt->block.stmts, i);
                print_stmt(child, child_prefix, i == stmt->block.stmts.size - 1);
            }
            break;
        }

        case STMT_IF: {
            print_node(prefix, last, "IfStmt");

            char child_prefix[1024];
            snprintf(child_prefix, sizeof(child_prefix),
                "%s%s", prefix, last ? "    " : "|   ");

            print_node(child_prefix, false, "Condition");
            char cond_prefix[2048];
            snprintf(cond_prefix, sizeof(cond_prefix),
                "%s|   ", child_prefix);
            print_expr(stmt->if_stmt.condition, cond_prefix, true);

            bool has_else = stmt->if_stmt.else_arm != NULL;

            print_node(child_prefix, !has_else, "Then");
            char then_prefix[2048];
            snprintf(then_prefix, sizeof(then_prefix),
                "%s%s", child_prefix, !has_else ? "    " : "|   ");
            print_stmt(stmt->if_stmt.then_arm, then_prefix, true);

            if (has_else) {
                print_node(child_prefix, true, "Else");
                char else_prefix[2048];
                snprintf(else_prefix, sizeof(else_prefix),
                    "%s    ", child_prefix);
                print_stmt(stmt->if_stmt.else_arm, else_prefix, true);
            }
            break;
        }

        case STMT_ASM: {
            print_node(prefix, last, "AsmStmt");

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
                        print_node(child_prefix, is_last, line_buf);
                    }
                }
            } else {
                print_node(child_prefix, true, "<null>");
            }
            break;
        }
    }
}

void print_ast(Program *program) {
    printf("Program\n");

    for (size_t i = 0; i < program->statements.size; i++) {
        Stmt *stmt = vector_at(&program->statements, i);

        print_stmt(stmt, "", i == program->statements.size - 1);
    }
}