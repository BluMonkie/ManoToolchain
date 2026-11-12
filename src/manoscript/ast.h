#pragma once

#include "defs.h"
#include "common/vector.h"
#include <stddef.h>

typedef enum {
    EXPR_BINARY,
    EXPR_UNARY,
    EXPR_GROUPING,
    EXPR_ASSIGNMENT,
    EXPR_IDENTIFIER,
    EXPR_LITERAL,
} ExprType;

typedef struct Expr Expr;

typedef struct {
    Expr *left;
    Token *op;
    Expr *right;
} BinaryExpr;

typedef struct {
    Token *op;
    Expr *operand;
} UnaryExpr;

typedef struct {
    Expr *expr;
} GroupingExpr;

typedef struct {
    Token *id;
} IdentifierExpr;

typedef struct {
    Token *id;
    Expr *value;
} AssignmentExpr;

typedef enum {
    LIT_NUMBER
} LiteralType;

typedef struct {
    LiteralType type;
    union {
        int num;
    };
} LiteralExpr;

struct Expr {
    ExprType type;
    union {
        UnaryExpr unary;
        BinaryExpr binary;
        GroupingExpr grouping;
        IdentifierExpr identifier;
        LiteralExpr literal;
        AssignmentExpr assignment;
    };
};

typedef struct Stmt Stmt;

typedef enum {
    STMT_VARDECL,
    STMT_EXPRESSION,
    STMT_BLOCK,
    STMT_IF,
    STMT_ASM,
} StmtType;

typedef struct {
    Token *identifier;
    Token *type_identifier;
    Expr *initializer;
} VarDeclStmt;

typedef struct {
    Expr *expr;
} ExprStatement;

typedef struct {
    Vector stmts;
} BlockStmt;

typedef struct {
    Expr *condition;
    Stmt *then_arm;
    Stmt *else_arm;
} IfStmt;

typedef struct {
    Token *asm;
} AsmStmt;

struct Stmt {
    StmtType type;
    union {
        VarDeclStmt var_decl;
        ExprStatement expr;
        BlockStmt block;
        IfStmt if_stmt;
        AsmStmt asm;
    };
};

typedef struct {
    Vector statements;
} Program;