#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"
#include "common/arena.h"
#include "ast.h"
#include "defs.h"
#include "intrinsics.h"
#include "symbol_map.h"
#include "common/vector.h"
#include "intrinsics.inc"

static void emit_stmt_inst(CodegenState *state, Stmt *stmt);
static void emit_var_decl_inst(CodegenState *state, VarDeclStmt *stmt);
static void emit_fun_decl_inst(CodegenState *state, FunDeclStmt *stmt);
static void emit_asm_inst(CodegenState *state, AsmStmt *stmt);
static void emit_block_inst(CodegenState *state, BlockStmt *stmt);
static void emit_if_inst(CodegenState *state, IfStmt *stmt);
static void emit_while_inst(CodegenState *state, WhileStmt *stmt);

static void emit_expr_inst(CodegenState *state, Expr *expr);
static void emit_assignment_expr(CodegenState *state, AssignmentExpr *expr);
static void emit_binary_expr(CodegenState *state, BinaryExpr *expr);
static void emit_binary_add_expr(CodegenState *state, BinaryExpr *expr, const char *temp);
static void emit_binary_sub_expr(CodegenState *state, BinaryExpr *expr, const char *temp);
static void emit_binary_and_expr(CodegenState *state, BinaryExpr *expr, const char *temp);
static void emit_binary_comp_expr(CodegenState *state, BinaryExpr *expr, const char *temp);
static void emit_binary_arg_intr_expr(CodegenState *state, BinaryExpr *expr, const char *temp);
static void emit_unary_expr(CodegenState *state, UnaryExpr *expr);
static void emit_identifier_expr(CodegenState *state, IdentifierExpr *expr);
static void emit_literal_expr(CodegenState *state, LiteralExpr *expr);
static void emit_call_expr(CodegenState *state, CallExpr *expr);

static void emit_data_section(CodegenState *state);
static void emit_var_data_section(CodegenState *state);
static void emit_const_data_section(CodegenState *state);
static void emit_temp_data_section(CodegenState *state);
static void emit_branches_section(CodegenState *state);
static void emit_function_section(CodegenState *state);

static void emit(CodegenState *state, ManoInst inst);

static const char *get_next_temporary(CodegenState *state);
static const char *pop_temporary(CodegenState *state);

static char get_base36_char(unsigned int i);
static bool is_alphanumeric(char c);

CodegenState codegen_new(Program *program) {
    return (CodegenState) {
        .program = program,
        .instructions = vector_new(sizeof(ManoInst)),
        .symbols = symbol_map_new(),
        .temporaries = vector_new_with_capacity(sizeof(char) * 4, 32),
        .arena = arena_new(),
    };
}

void codegen_free(CodegenState *state) {
    vector_free(&state->instructions);  
    vector_free(&state->temporaries);
    
    symbol_map_free(&state->symbols);
    arena_drop(&state->arena);
} 

void emit_instructions(CodegenState *state) {
    emit(state, (ManoInst) {
        .type = INST_PSI_ORG,
        .argument = "100"
    });

    for (size_t i = 0; i < state->program->statements.size; i++) {
        Stmt *stmt = vector_at(&state->program->statements, i);
        if (stmt->type == STMT_PSEUDO_EN_BUILTIN) {
            state->is_builtin = true;
            continue;
        }
        if (stmt->type == STMT_PSEUDO_DIS_BUILTIN) {
            state->is_builtin = false;
            continue;
        }
        emit_stmt_inst(state, stmt);
    }

    emit(state, (ManoInst) {
        .type = INST_REG_HLT,
    });
    emit_function_section(state);
    emit_branches_section(state);
    emit_intrinsics(state);
    emit_data_section(state);
    emit(state, (ManoInst) { .type = INST_PSI_END });
}

static void emit_stmt_inst(CodegenState *state, Stmt *stmt) {
    switch (stmt->type) {
        case STMT_VARDECL:
            emit_var_decl_inst(state, &stmt->var_decl);
            break;
        case STMT_FUNDECL:
            emit_fun_decl_inst(state, &stmt->fun_decl);
            break;
        case STMT_EXPRESSION:
            emit_expr_inst(state, stmt->expr.expr);
            break;
        case STMT_ASM:
            emit_asm_inst(state, &stmt->asm);
            break;
        case STMT_BLOCK:
            emit_block_inst(state, &stmt->block);
            break;
        case STMT_IF:
            emit_if_inst(state, &stmt->if_stmt);
            break;
        case STMT_WHILE:
            emit_while_inst(state, &stmt->while_stmt);
            break;
        default:
            assert(0 && "Unreachable code");
    }
}

static void emit_var_decl_inst(CodegenState *state, VarDeclStmt *stmt) {
    // TODO: Add check for if var is predefined
    symbol_define_symbol(&state->symbols, stmt->identifier->lexeme, state->is_builtin);
    if (stmt->initializer != nullptr) {
        emit_expr_inst(state, stmt->initializer);
        emit(state, (ManoInst) {
            .type = INST_MEM_STA,
            .argument = symbol_get_map_symbol(&state->symbols, stmt->identifier->lexeme, state->is_builtin),
        });
    }
}

static void emit_fun_decl_inst(CodegenState *state, FunDeclStmt *stmt) {
    // TODO: Check if symbol is available
    symbol_define_function(&state->symbols, stmt, state->is_builtin);
}

static void emit_block_inst(CodegenState *state, BlockStmt *stmt) {
    for (size_t i = 0; i < stmt->stmts.size; i++) {
        emit_stmt_inst(state, vector_at(&stmt->stmts, i));
    }
}

static void emit_if_inst(CodegenState *state, IfStmt *stmt) {
    emit_expr_inst(state, stmt->condition);
    emit(state, (ManoInst) {
        .type = INST_REG_SZA,
    });

    const char *lreturn = symbol_get_label(&state->symbols);
    const char *lthen = symbol_get_label_for_block(&state->symbols, stmt->then_arm, lreturn);
    const char *lelse = symbol_get_label_for_block(&state->symbols, stmt->else_arm, lreturn);

    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = lthen,
    });
    if (stmt->else_arm != nullptr) {
        emit(state, (ManoInst) {
            .type = INST_MEM_BUN,
            .argument = lelse,
        });
    }
    state->pending_label = lreturn;
}

static void emit_while_inst(CodegenState *state, WhileStmt *stmt) {
    const char *lreturn = symbol_get_label(&state->symbols);
    const char *lloop = symbol_get_label_for_block(&state->symbols, stmt->body, lreturn);
    
    state->pending_label = lreturn;
    emit_expr_inst(state, stmt->condition);
    emit(state, (ManoInst) {
        .type = INST_REG_SZA,
    });
    emit(state, (ManoInst) {
        .type = INST_MEM_BUN,
        .argument = lloop,
    });
}

static void emit_asm_inst(CodegenState *state, AsmStmt *stmt) {
    StringSlice asm = stmt->asm->lexeme;
    const char *end = asm.start + asm.len;
    Vector sb = vector_new_with_capacity(sizeof(char), asm.len);

    // TODO: Verify the variable actually exists
    const char *c = asm.start;
    while (c < end && (*c == ' ' || *c == '\t' || *c == '\r' || *c == '\n')) c++;

    while (c < end) {
        
        if (*c == '$') {
            const char *begin = ++c;

            while (c < end && is_alphanumeric(*c)) c++;

            const char *substitute = symbol_get_map_symbol(&state->symbols, (StringSlice) {
                .start = begin,
                .len = c - begin,
            }, false);

            for (int i = 0; i < 3; i++) {
                vector_push(&sb, &substitute[i]);
            }
        } else if (*c == '\n') {
            vector_push(&sb, c);
            c++;
            while (c < end && (*c == ' ' || *c == '\t' || *c == '\r' || *c == '\n')) c++;
        } else {
            vector_push(&sb, c);
            c++;
        }
    }
    char _;
    if (*((char *) vector_at(&sb, sb.size - 1)) == '\n') vector_pop(&sb, &_);

    char *str = arena_alloc(&state->arena, sb.size + 1);
    for (size_t i = 0; i < sb.size; i++) {
        str[i] = *((char *) vector_at(&sb, i));
    }
    str[sb.size] = '\0';

    emit(state, (ManoInst) {
        .type = INST_ASM,
        .argument = str,
    });
    vector_free(&sb);
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
        case EXPR_CALL:
            emit_call_expr(state, &expr->call);
            break;
    }
}

static void emit_assignment_expr(CodegenState *state, AssignmentExpr *expr) {
    // TODO: check the variable actually exists
    emit_expr_inst(state, expr->value);
    emit(state, (ManoInst) {
        .type = INST_MEM_STA,
        .argument = symbol_get_map_symbol(&state->symbols, expr->id->lexeme, expr->builtin),
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
        case TOKEN_LESS:
        case TOKEN_LESS_EQUAL:
        case TOKEN_GREATER:
        case TOKEN_GREATER_EQUAL:
        case TOKEN_EQUAL_EQUAL:
        case TOKEN_BANG_EQUAL:
            emit_binary_comp_expr(state, expr, temp);
            break;
        case TOKEN_STAR:
        case TOKEN_SLASH:
        case TOKEN_PERCENT:
        case TOKEN_LESS_LESS:
        case TOKEN_GREATER_GREATER:
        case TOKEN_CARET:
        case TOKEN_PIPE:
            emit_binary_arg_intr_expr(state, expr, temp);
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

static void emit_binary_comp_expr(CodegenState *state, BinaryExpr *expr, const char *temp) {
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
    
    const char *jmp_label;

    switch (expr->op->type) {
        case TOKEN_LESS:
            jmp_label = enable_intrinsic(state, INT_CMP_LS);
            break;
        case TOKEN_GREATER:
            jmp_label = enable_intrinsic(state, INT_CMP_GR);
            break;
        case TOKEN_LESS_EQUAL:
            jmp_label = enable_intrinsic(state, INT_CMP_LS_EQ);
            break;
        case TOKEN_GREATER_EQUAL:
            jmp_label = enable_intrinsic(state, INT_CMP_GR_EQ);
            break;
        case TOKEN_EQUAL_EQUAL:
            jmp_label = enable_intrinsic(state, INT_CMP_EQ);
            break;
        case TOKEN_BANG_EQUAL:
            jmp_label = enable_intrinsic(state, INT_CMP_NEQ);
            break;
        default:
            assert(0 && "Unreachable");
    }

    emit(state, (ManoInst) {
        .type = INST_MEM_BSA,
        .argument = jmp_label,
    });
}

static void emit_binary_arg_intr_expr(CodegenState *state, BinaryExpr *expr, const char *temp) {
    Intrinsic intr;
    switch (expr->op->type) {
        case TOKEN_STAR:
            intr = INT_ARTH_MUL;
            break;
        case TOKEN_SLASH:
            intr = INT_ARTH_DIV;
            break;
        case TOKEN_PERCENT:
            intr = INT_ARTH_REM;
            break;
        case TOKEN_LESS_LESS:
            intr = INT_BIT_LSH;
            break;
        case TOKEN_GREATER_GREATER:
            intr = INT_BIT_RSH;
            break;
        case TOKEN_PIPE:
            intr = INT_BIT_OR;
            break;
        case TOKEN_CARET:
            intr = INT_BIT_XOR;
            break;
        default:
            assert(0 && "Unreachable code");
    }

    const char *label = enable_intrinsic(state, intr);

    const char *slot = get_intrinsic_arg(state, intr);
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
        .type = INST_MEM_STA,
        .argument = slot,
    });
    emit(state, (ManoInst) {
        .type = INST_MEM_LDA,
        .argument = t1,
    });
    emit(state, (ManoInst) {
        .type = INST_MEM_BSA,
        .argument = label,
    });
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
    const char *label = symbol_get_map_symbol(&state->symbols, expr->id->lexeme, expr->builtin);
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

static void emit_call_expr(CodegenState *state, CallExpr *expr) {
    // TODO: Verify the function actually exists and parameter count
    FunDeclStmt *fun = symbol_get_function(&state->symbols, expr->callee->lexeme, expr->builtin);
    assert(expr->arguments.size == fun->parameters.size);

    for (size_t i = 0; i < expr->arguments.size; i++) {
        Expr *arg = vector_at(&expr->arguments, i);

        emit_expr_inst(state, arg);
        emit(state, (ManoInst) {
            .type = INST_MEM_STA,
            .argument = get_next_temporary(state),
        });
    }

    for (size_t i = fun->parameters.size; i > 0; i--) {
        FunParameter *param = vector_at(&fun->parameters, i - 1);

        emit(state, (ManoInst) {
            .type = INST_MEM_LDA,
            .argument = pop_temporary(state),  
        });
        emit(state, (ManoInst) {
            .type = INST_MEM_STA,
            .argument = param->mangled,
        });
    }

    emit(state, (ManoInst) {
        .type = INST_MEM_BSA,
        .argument = symbol_get_map_symbol(&state->symbols, fun->identifier->lexeme, expr->builtin),
    });
}

static void emit_data_section(CodegenState *state) {
    emit_var_data_section(state);
    emit_const_data_section(state);
    emit_temp_data_section(state);
}

static void emit_var_data_section(CodegenState *state) {
    for (size_t i = 0; i < state->symbols.symbol_map.size; i++) {
        SymbolPair *pair = vector_at(&state->symbols.symbol_map, i);
        if (pair->type != SYMBOL_VAR) continue;

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

static void emit_branches_section(CodegenState *state) {
    for (size_t i = 0; i < state->symbols.label_map.size; i++) {
        LabelPair *pair = vector_at(&state->symbols.label_map, i);
        if (pair->stmt == nullptr) continue;

        state->pending_label = pair->label;
        emit_stmt_inst(state, pair->stmt);
        emit(state, (ManoInst) {
            .type = INST_MEM_BUN,
            .argument = pair->return_label,
        });
    }
}

static void emit_function_section(CodegenState *state) {
    for (size_t i = 0; i < state->symbols.functions.size; i++) {
        FunDeclStmt *fun = vector_at(&state->symbols.functions, i);
        const char *label = symbol_get_map_symbol(&state->symbols, fun->identifier->lexeme, fun->is_builtin);
        restore_fun_scope(&state->symbols, fun);

        emit(state, (ManoInst) {
            .type = INST_PSI_HEX,
            .label = label,
            .argument = "0",
        });
        emit_block_inst(state, fun->body);
        if (fun->output != nullptr) {
            emit(state, (ManoInst) {
                .type = INST_MEM_LDA,
                .argument = fun->output->mangled,
            });
        }
        emit(state, (ManoInst) {
            .type = INST_MEM_BUN,
            .argument = label,
            .indirect = true,
        });
        symbol_pop_scope(&state->symbols);
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

static const char *pop_temporary(CodegenState *state) {
    if (state->temp_depth == 0) return nullptr;
    return vector_at(&state->temporaries, --state->temp_depth);
}

static void emit(CodegenState *state, ManoInst inst) {
    if (state->pending_label != nullptr) {
        assert(inst.label == nullptr && "Pending label collided with explicit instruction label!");

        inst.label = state->pending_label;
        state->pending_label = nullptr;
    }
    vector_push(&state->instructions, &inst);
}

static char get_base36_char(unsigned int i) {
    static const char alphanum[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    
    return alphanum[i % 36];
}

static bool is_alphanumeric(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || (c >= '0' && c <= '9');
}