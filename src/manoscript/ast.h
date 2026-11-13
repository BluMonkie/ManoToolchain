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
    EXPR_CALL,
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

typedef struct {
    Token *callee;
    Vector arguments;
} CallExpr;

struct Expr {
    ExprType type;
    union {
        UnaryExpr unary;
        BinaryExpr binary;
        GroupingExpr grouping;
        IdentifierExpr identifier;
        LiteralExpr literal;
        AssignmentExpr assignment;
        CallExpr call;
    };
};

typedef struct Stmt Stmt;

typedef enum {
    STMT_VARDECL,
    STMT_FUNDECL,
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
} ExprStmt;

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

typedef struct {
    Token *identifier;
    Token *type_identifier;
    const char *mangled;
} FunParameter;

typedef struct {
    Token *identifier;
    Vector parameters;
    FunParameter *output;
    BlockStmt *body;
} FunDeclStmt;

struct Stmt {
    StmtType type;
    union {
        VarDeclStmt var_decl;
        FunDeclStmt fun_decl;
        ExprStmt expr;
        BlockStmt block;
        IfStmt if_stmt;
        AsmStmt asm;
    };
};

typedef struct {
    Vector statements;
} Program;