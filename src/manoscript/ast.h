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
    bool builtin;
} IdentifierExpr;

typedef struct {
    Token *id;
    Expr *value;
    bool builtin;
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
    bool builtin;
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
    STMT_WHILE,
    STMT_ASM,
    STMT_PSEUDO_EN_BUILTIN,
    STMT_PSEUDO_DIS_BUILTIN,
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
    Expr *condition;
    Stmt *body;
} WhileStmt;

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
    bool is_builtin;
} FunDeclStmt;

struct Stmt {
    StmtType type;
    union {
        VarDeclStmt var_decl;
        FunDeclStmt fun_decl;
        ExprStmt expr;
        BlockStmt block;
        IfStmt if_stmt;
        WhileStmt while_stmt;
        AsmStmt asm;
    };
};

typedef struct {
    Vector statements;
} Program;