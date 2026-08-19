#include <assert.h>
#include <stdio.h>

#include "parser.h"
#include "debug.h"

#define INVALID_STATEMENT (Statement) {.instruction = INST_INVALID}

ParserState parser_new(char *src, Vector tokens);

void parse_tokens(ParserState *state);
void parse_statements(ParserState *state);

void resolve_symbolic_addresses(ParserState *state);
void verify_addresses(ParserState *state);

Statement parse_statement(ParserState *state);
Statement parse_instruction(ParserState *state, Instruction inst);
Statement parse_mr_instruction(ParserState *state, Instruction inst);
Statement parse_nmr_instruction(ParserState *state, Instruction inst);
Statement parse_ps_instruction(ParserState *state, Instruction inst);
Statement parse_label(ParserState *state);

Token *peek(ParserState *state);
Token *advance(ParserState *state);
Token *previous(ParserState *state);
bool check(ParserState *state, TokenType type);
bool match(ParserState *state, TokenType type);
Token *consume(ParserState *state, TokenType type);

bool is_at_end(ParserState *state);

Instruction instruction_from_token(TokenType type);

ParserState parser_new(char *src, Vector tokens) {
    Vector vec = vector_new(sizeof(Statement));
    SymbolTable table = symbol_table_new(src);

    return (ParserState) {
        .src = src,
        .had_error = false,
        .current = 0,
        .current_address = 0,
        .tokens = tokens,
        .statements = vec,
        .symbols = table,
    };
}

void parse_tokens(ParserState *state) {
    parse_statements(state);
    
    if (state->symbols.had_error) {
        state->had_error = true;
    }

    if (!state->had_error) {
        verify_addresses(state);
    }

    if (!state->had_error) {
        resolve_symbolic_addresses(state);

        if (state->symbols.had_error) {
            state->had_error = true;
        }
    }
}

void parse_statements(ParserState *state) {
    while (!is_at_end(state)) {
        while (peek(state)->type == TOKEN_NEWLINE) {
                advance(state);
        }
        Statement stmt = parse_statement(state);
        
        if (stmt.instruction == INST_PSI_END) {
            while (peek(state)->type == TOKEN_NEWLINE) {
                advance(state);
            }
            if (!is_at_end(state)) {
                state->had_error = true;
                fprintf(stderr, "ERROR while parsing: END instruction must be present at the end of the program.\n");
            }
            break;
        }
        consume(state, TOKEN_NEWLINE);
        if (stmt.instruction != INST_INVALID) {
            vector_push(&state->statements, &stmt);
        }
    }
}

void resolve_symbolic_addresses(ParserState *state) {
    size_t len = state->statements.size;

    for (size_t i = 0; i < len; i++) {
        Statement *stmt = vector_at(&state->statements, i);
        if (!stmt->operand_pending) continue;

        Address address = symbol_locate(&state->symbols, *stmt->symbolic_address);
        stmt->operand = address;
        stmt->operand_pending = false;
    }
}

void verify_addresses(ParserState *state) {
    for (size_t i = 0; i < state->statements.size; i++) {
        const Statement *stmt = vector_at_const(&state->statements, i);

        if (stmt->location == INVALID_ADDRESS) {
            continue;
        }

        if (stmt->location > 0xFFF) {
            state->had_error = true;
            fprintf(stderr, "ERROR: Statement at address 0x%04X exceeds Mano's 12-bit address space.\n", stmt->location);
        }
    }
}

Statement parse_statement(ParserState *state) {
    Token *token = advance(state);
    Instruction inst = instruction_from_token(token->type);

    if (inst == INST_INVALID) {
        if (token->type == TOKEN_IDENTIFIER) {
            return parse_label(state);
        } else {
            state->had_error = true;
            fprintf(stderr, "ERROR while parsing: Expected instruction or label identifier, got %s.\n", token_type_name(token->type));
            return INVALID_STATEMENT;
        }
    } else {
        return parse_instruction(state, inst);
    }
}

Statement parse_instruction(ParserState *state, Instruction inst) {
    assert(inst != INST_INVALID && "ERROR: Cannot parse invalid instruction.");

    switch (inst) {
        case INST_PSI_ORG:
        case INST_PSI_DEC:
        case INST_PSI_HEX:
        case INST_PSI_END:
            return parse_ps_instruction(state, inst);
        default:
            break;
    }

    if (inst >= 0 && inst <= 6) {
        return parse_mr_instruction(state, inst);
    } else if (inst >> 12 == 0x7 || inst >> 12 == 0xF) {
        return parse_nmr_instruction(state, inst);
    }

    assert(false && "Unknown instruction");
    return INVALID_STATEMENT;
}

Statement parse_mr_instruction(ParserState *state, Instruction inst) {
    Token *symbolic_address = consume(state, TOKEN_IDENTIFIER);
    bool is_indirect = match(state, TOKEN_I);

    return (Statement) {
        .instruction = inst,
        .operand_pending = true,
        .location = state->current_address++,
        .indirect = is_indirect,
        .symbolic_address = symbolic_address
    };
}

Statement parse_nmr_instruction(ParserState *state, Instruction inst) {
    return (Statement) {
        .instruction = inst,
        .location = state->current_address++
    };
}

Statement parse_ps_instruction(ParserState *state, Instruction inst) {
    if (inst == INST_PSI_ORG) {
        Token *address = consume(state, TOKEN_HEX_NUMBER);
        if (address == NULL) {
            return INVALID_STATEMENT;
        }
        if (address->number > 0xFFF) {
            state->had_error = true;
            fprintf(stderr, "ERROR while parsing: Address exceeds 12-bit address space.\n");
            return INVALID_STATEMENT;
        }

        state->current_address = (uint16_t) address->number;
        return (Statement) {
            .instruction = inst,
            .location = INVALID_ADDRESS,
            .operand = address->number,
        };
    } else if (inst == INST_PSI_DEC) {
        Token *num = consume(state, TOKEN_DEC_NUMBER);
        if (num == NULL) {
            return INVALID_STATEMENT;
        }

        return (Statement) {
            .instruction = inst,
            .location = state->current_address++,
            .operand = num->number
        };
    } else if (inst == INST_PSI_HEX) {
        Token *num = consume(state, TOKEN_HEX_NUMBER);
        if (num == NULL) {
            return INVALID_STATEMENT;
        }

        return (Statement) {
            .instruction = inst,
            .location = state->current_address++,
            .operand = num->number
        };
    }

    return (Statement) {
        .instruction = INST_PSI_END,
        .location = INVALID_ADDRESS
    };
}

Statement parse_label(ParserState *state) {
    Token *identifier = previous(state);
    symbol_define(&state->symbols, *identifier, state->current_address);
    
    consume(state, TOKEN_COMMA);
    
    Token *token = advance(state);
    Instruction inst = instruction_from_token(token->type);

    if (inst == INST_INVALID) {
        state->had_error = true;
        fprintf(stderr, "ERROR while parsing: Expected instruction, got %s.\n", token_type_name(token->type));
        return INVALID_STATEMENT;
    }

    return parse_instruction(state, inst);
}

Token *peek(ParserState *state) {
    return vector_at(&state->tokens, state->current);
}

Token *advance(ParserState *state) {
    return vector_at(&state->tokens, state->current++);
}

Token *previous(ParserState *state) {
    return vector_at(&state->tokens, state->current - 1);
}

bool check(ParserState *state, TokenType type) {
    return peek(state)->type == type;
}

bool match(ParserState *state, TokenType type) {
    if (check(state, type)) {
        advance(state);
        return true;
    }
    return false;
}

Token *consume(ParserState *state, TokenType type) {
    if (match(state, type)) {
        return previous(state);
    }
    state->had_error = true;
    fprintf(stderr, "ERROR while parsing: Expected %s, got %s.\n", token_type_name(type), token_type_name(peek(state)->type));   
    return NULL;
}

bool is_at_end(ParserState *state) {
    return peek(state)->type == TOKEN_EOF;
}

Instruction instruction_from_token(TokenType type) {
    switch (type) {
        case TOKEN_AND: return INST_MEM_AND;
        case TOKEN_ADD: return INST_MEM_ADD;
        case TOKEN_LDA: return INST_MEM_LDA;
        case TOKEN_STA: return INST_MEM_STA;
        case TOKEN_BUN: return INST_MEM_BUN;
        case TOKEN_BSA: return INST_MEM_BSA;
        case TOKEN_ISZ: return INST_MEM_ISZ;

        case TOKEN_CLA: return INST_REG_CLA;
        case TOKEN_CLE: return INST_REG_CLE;
        case TOKEN_CMA: return INST_REG_CMA;
        case TOKEN_CME: return INST_REG_CME;
        case TOKEN_CIR: return INST_REG_CIR;
        case TOKEN_CIL: return INST_REG_CIL;
        case TOKEN_INC: return INST_REG_INC;
        case TOKEN_SPA: return INST_REG_SPA;
        case TOKEN_SNA: return INST_REG_SNA;
        case TOKEN_SZA: return INST_REG_SZA;
        case TOKEN_SZE: return INST_REG_SZE;
        case TOKEN_HLT: return INST_REG_HLT;

        case TOKEN_INP: return INST_IO_INP;
        case TOKEN_OUT: return INST_IO_OUT;
        case TOKEN_SKI: return INST_IO_SKI;
        case TOKEN_SKO: return INST_IO_SKO;
        case TOKEN_ION: return INST_IO_ION;
        case TOKEN_IOF: return INST_IO_IOF;

        case TOKEN_ORG: return INST_PSI_ORG;
        case TOKEN_DEC: return INST_PSI_DEC;
        case TOKEN_HEX: return INST_PSI_HEX;
        case TOKEN_END: return INST_PSI_END;

        default:
            return INST_INVALID;
    }
}