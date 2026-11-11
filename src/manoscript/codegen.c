#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "codegen.h"
#include "ast.h"
#include "defs.h"
#include "symbol_map.h"
#include "common/vector.h"

static void emit_stmt_inst(CodegenState *state, Stmt *stmt);
static void emit_var_decl_inst(CodegenState *state, VarDeclStmt *stmt);

static void emit_expr_inst(CodegenState *state, Expr *expr);
static void emit_assignment_expr(CodegenState *state, AssignmentExpr *expr);
static void emit_binary_expr(CodegenState *state, BinaryExpr *expr);
static void emit_binary_add_expr(CodegenState *state, BinaryExpr *expr, const char *temp);
static void emit_binary_sub_expr(CodegenState *state, BinaryExpr *expr, const char *temp);
static void emit_binary_and_expr(CodegenState *state, BinaryExpr *expr, const char *temp);
static void emit_binary_or_expr(CodegenState *state, BinaryExpr *expr, const char *temp);
static void emit_binary_xor_expr(CodegenState *state, BinaryExpr *expr, const char *temp);
static void emit_unary_expr(CodegenState *state, UnaryExpr *expr);
static void emit_identifier_expr(CodegenState *state, IdentifierExpr *expr);
static void emit_literal_expr(CodegenState *state, LiteralExpr *expr);

static void emit_data_section(CodegenState *state);
static void emit_var_data_section(CodegenState *state);
static void emit_const_data_section(CodegenState *state);
static void emit_temp_data_section(CodegenState *state);

static void emit(CodegenState *state, ManoInst inst);

static const char *get_next_temporary(CodegenState *state);
static void pop_temporary(CodegenState *state);
static char get_base36_char(unsigned int i);

CodegenState codegen_new(Program *program) {
    return (CodegenState) {
        .program = program,
        .instructions = vector_new(sizeof(ManoInst)),
        .symbols = symbol_map_new(),
        .temporaries = vector_new_with_capacity(sizeof(char) * 4, 32),
    };
}

void emit_instructions(CodegenState *state) {
    emit(state, (ManoInst) {
        .type = INST_PSI_ORG,
        .argument = "100"
    });

    for (size_t i = 0; i < state->program->statements.size; i++) {
        Stmt *stmt = vector_at(&state->program->statements, i);
        emit_stmt_inst(state, stmt);
    }

    emit(state, (ManoInst) {
        .type = INST_REG_HLT,
    });
    emit_data_section(state);
    emit(state, (ManoInst) { .type = INST_PSI_END });
}

static void emit_stmt_inst(CodegenState *state, Stmt *stmt) {
    switch (stmt->type) {
        case STMT_VARDECL:
            emit_var_decl_inst(state, &stmt->var_decl);
            break;
        case STMT_EXPRESSION:
            emit_expr_inst(state, stmt->expr.expr);
            break;
    }
}

static void emit_var_decl_inst(CodegenState *state, VarDeclStmt *stmt) {
    // TODO: Add check for if var is predefined
    symbol_define_var(&state->symbols, stmt->identifier->lexeme);
    if (stmt->initializer != nullptr) {
        emit_expr_inst(state, stmt->initializer);
        emit(state, (ManoInst) {
            .type = INST_MEM_STA,
            .argument = symbol_get_map_var(&state->symbols, stmt->identifier->lexeme),
        });
    }
}

static void emit_expr_inst(CodegenState *state, Expr *expr) {
    switch (expr->type) {
        case EXPR_BINARY:
            emit_binary_expr(state, &expr->binary);
            break;
        case EXPR_ASSIGNMENT:
            emit_assignment_expr(state, &expr->assignment);
            break;
        case EXPR_LITERAL:
            emit_literal_expr(state, &expr->literal);
            break;
        case EXPR_GROUPING:
            emit_expr_inst(state, expr->grouping.expr);
            break;
        case EXPR_IDENTIFIER:
            emit_identifier_expr(state, &expr->identifier);
            break;
        case EXPR_UNARY:
            emit_unary_expr(state, &expr->unary);
            break;
    }
}

static void emit_assignment_expr(CodegenState *state, AssignmentExpr *expr) {
    // TODO: check the variable actually exists
    emit_expr_inst(state, expr->value);
    emit(state, (ManoInst) {
        .type = INST_MEM_STA,
        .argument = symbol_get_map_var(&state->symbols, expr->id->lexeme),
    });
}

static void emit_binary_expr(CodegenState *state, BinaryExpr *expr) {
    emit_expr_inst(state, expr->left);
    
    const char *temp = get_next_temporary(state);
    emit(state, (ManoInst) {
        .type = INST_MEM_STA,
        .argument = temp,
    });

    emit_expr_inst(state, expr->right);
    
    if (expr->op->type == TOKEN_PLUS) {
        emit_binary_add_expr(state, expr, temp);
    } else if (expr->op->type == TOKEN_MINUS) {
        emit_binary_sub_expr(state, expr, temp);
    }

    switch (expr->op->type) {
        case TOKEN_PLUS:
            emit_binary_add_expr(state, expr, temp);
            break;
        case TOKEN_MINUS:
            emit_binary_sub_expr(state, expr, temp);
            break;
        case TOKEN_AMP:
            emit_binary_and_expr(state, expr, temp);
            break;
        case TOKEN_PIPE:
            emit_binary_or_expr(state, expr, temp);
            break;
        case TOKEN_CARET:
            emit_binary_xor_expr(state, expr, temp);
            break;
        default:
            assert(0 && "unreachable code");
    }

    pop_temporary(state);
}

static void emit_binary_add_expr(CodegenState *state, BinaryExpr *expr, const char *temp) {
    emit(state, (ManoInst) {
        .type = INST_MEM_ADD,
        .argument = temp,
    });
}

static void emit_binary_sub_expr(CodegenState *state, BinaryExpr *expr, const char *temp) {
    emit(state, (ManoInst) {
        .type = INST_REG_CMA,
    });
    emit(state, (ManoInst) {
        .type = INST_REG_INC,
    });

    emit(state, (ManoInst) {
        .type = INST_MEM_ADD,
        .argument = temp,
    });
}

static void emit_binary_and_expr(CodegenState *state, BinaryExpr *expr, const char *temp) {
    emit(state, (ManoInst) {
        .type = INST_MEM_AND,
        .argument = temp,
    });
}

static void emit_binary_or_expr(CodegenState *state, BinaryExpr *expr, const char *temp) {
    emit(state, (ManoInst) {
        .type = INST_REG_CMA,
    });
    
    const char *t1 = get_next_temporary(state);
    emit(state, (ManoInst) {
        .type = INST_MEM_STA,
        .argument = t1,
    });
    
    emit(state, (ManoInst) {
        .type = INST_MEM_LDA,
        .argument = temp,
    });
    emit(state, (ManoInst) {
        .type = INST_REG_CMA,
    });
    
    emit(state, (ManoInst) {
        .type = INST_MEM_AND,
        .argument = t1,
    });
    emit(state, (ManoInst) {
        .type = INST_REG_CMA,
    });

    pop_temporary(state);
}

static void emit_binary_xor_expr(CodegenState *state, BinaryExpr *expr, const char *temp) {
    const char *t1 = get_next_temporary(state);
    const char *t2 = get_next_temporary(state);

    // Preserve B
    emit(state, (ManoInst) {
        .type = INST_MEM_STA,
        .argument = t1,
    });

    // Compute ~(A & ~B)
    emit(state, (ManoInst) {
        .type = INST_REG_CMA,
    });
    emit(state, (ManoInst) {
        .type = INST_MEM_AND,
        .argument = temp,
    });
    emit(state, (ManoInst) {
        .type = INST_REG_CMA,
    });
    emit(state, (ManoInst) {
        .type = INST_MEM_STA,
        .argument = t2,
    });
    
    // Compute ~(~A & B)
    emit(state, (ManoInst) {
        .type = INST_MEM_LDA,
        .argument = temp,
    });
    emit(state, (ManoInst) {
        .type = INST_REG_CMA,
    });
    emit(state, (ManoInst) {
        .type = INST_MEM_AND,
        .argument = t1,
    });
    emit(state, (ManoInst) {
        .type = INST_REG_CMA,
    });
    
    // Compute ~(A & ~B) & ~(~A & B)
    emit(state, (ManoInst) {
        .type = INST_MEM_AND,
        .argument = t2,
    });

    // Compute ~(~(A & ~B) & ~(~A & B)) = A ^ B
    emit(state, (ManoInst) {
        .type = INST_REG_CMA,
    });
    
    pop_temporary(state);
    pop_temporary(state);
}

static void emit_unary_expr(CodegenState *state, UnaryExpr *expr) {
    emit_expr_inst(state, expr->operand);
    switch (expr->op->type) {
        case TOKEN_PLUS:
            break;
        case TOKEN_MINUS:
            emit(state, (ManoInst) {
                .type = INST_REG_CMA,
            });
            emit(state, (ManoInst) {
                .type = INST_REG_INC,
            });
            break;
        case TOKEN_TILDE:
            emit(state, (ManoInst) {
                .type = INST_REG_CMA,
            });
            break;
        default:
            assert(0 && "Unreachable code\n");
    }
}

static void emit_identifier_expr(CodegenState *state, IdentifierExpr *expr) {
    const char *label = symbol_get_map_var(&state->symbols, expr->id->lexeme);
    emit(state, (ManoInst) {
        .type = INST_MEM_LDA,
        .argument = label,
    });
}

static void emit_literal_expr(CodegenState *state, LiteralExpr *expr) {
    const char *label = symbol_get_map_const(&state->symbols, expr->num);
    emit(state, (ManoInst) {
        .type = INST_MEM_LDA,
        .argument = label,
    });
}

static void emit_data_section(CodegenState *state) {
    emit_var_data_section(state);
    emit_const_data_section(state);
    emit_temp_data_section(state);
}

static void emit_var_data_section(CodegenState *state) {
    for (size_t i = 0; i < state->symbols.var_map.size; i++) {
        VarPair *pair = vector_at(&state->symbols.var_map, i);

        emit(state, (ManoInst) {
            .type = INST_PSI_DEC,
            .label = pair->mangled,
            .argument = "0",
        });
    }
}


static void emit_const_data_section(CodegenState *state) {
    for (size_t i = 0; i < state->symbols.const_map.size; i++) {
        ConstPair *pair = vector_at(&state->symbols.const_map, i);
        snprintf(pair->value_str, 16, "%d", pair->value);

        emit(state, (ManoInst) {
            .type = INST_PSI_DEC,
            .label = pair->mangled,
            .argument = pair->value_str,
        });
    }
}

static void emit_temp_data_section(CodegenState *state) {
    for (size_t i = 0; i < state->temporaries.size; i++) {
        char *label = vector_at(&state->temporaries, i);

        emit(state, (ManoInst) {
            .type = INST_PSI_DEC,
            .label = label,
            .argument = "0",
        });
    }
}


static const char *get_next_temporary(CodegenState *state) {
    size_t slot = state->temp_depth++;
    if (slot < state->temporaries.size) {
        return vector_at(&state->temporaries, slot);
    }

    char label[4];
    label[0] = 't';
    label[1] = get_base36_char(slot / 36);
    label[2] = get_base36_char(slot % 36);
    label[3] = '\0';
    
    vector_push(&state->temporaries, label);
    return vector_at(&state->temporaries, state->temporaries.size - 1);
}

static void pop_temporary(CodegenState *state) {
    if (state->temp_depth == 0) return;
    state->temp_depth--;
}

static void emit(CodegenState *state, ManoInst inst) {
    vector_push(&state->instructions, &inst);
}

static char get_base36_char(unsigned int i) {
    static const char alphanum[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    
    return alphanum[i % 36];
}