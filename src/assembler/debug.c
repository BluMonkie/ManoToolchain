#include <stdio.h>

#include "debug.h"
#include "defs.h"
#include "lexer.h"
#include "parser.h"

static const char *TOKEN_TYPE_NAMES[] = {
    [TOKEN_AND]         = "AND",
    [TOKEN_ADD]         = "ADD",
    [TOKEN_LDA]         = "LDA",
    [TOKEN_STA]         = "STA",
    [TOKEN_BUN]         = "BUN",
    [TOKEN_BSA]         = "BSA",
    [TOKEN_ISZ]         = "ISZ",

    [TOKEN_CLA]         = "CLA",
    [TOKEN_CLE]         = "CLE",
    [TOKEN_CMA]         = "CMA",
    [TOKEN_CME]         = "CME",
    [TOKEN_CIR]         = "CIR",
    [TOKEN_CIL]         = "CIL",
    [TOKEN_INC]         = "INC",
    [TOKEN_SPA]         = "SPA",
    [TOKEN_SNA]         = "SNA",
    [TOKEN_SZA]         = "SZA",
    [TOKEN_SZE]         = "SZE",
    [TOKEN_HLT]         = "HLT",

    [TOKEN_INP]         = "INP",
    [TOKEN_OUT]         = "OUT",
    [TOKEN_SKI]         = "SKI",
    [TOKEN_SKO]         = "SKO",
    [TOKEN_ION]         = "ION",
    [TOKEN_IOF]         = "IOF",

    [TOKEN_ORG]         = "ORG",
    [TOKEN_END]         = "END",
    [TOKEN_DEC]         = "DEC",
    [TOKEN_HEX]         = "HEX",

    [TOKEN_IDENTIFIER]  = "IDENTIFIER",
    [TOKEN_COMMA]       = "COMMA",
    [TOKEN_NEWLINE]     = "NEWLINE",
    [TOKEN_DEC_NUMBER]  = "DEC_NUMBER",
    [TOKEN_HEX_NUMBER]  = "HEX_NUMBER",
    [TOKEN_EOF]         = "EOF",
};

const char *token_type_name(TokenType type) {
    return TOKEN_TYPE_NAMES[type];
}

const char *instruction_name(Instruction instruction) {
    switch (instruction) {
        case INST_MEM_AND: return "AND";
        case INST_MEM_ADD: return "ADD";
        case INST_MEM_LDA: return "LDA";
        case INST_MEM_STA: return "STA";
        case INST_MEM_BUN: return "BUN";
        case INST_MEM_BSA: return "BSA";
        case INST_MEM_ISZ: return "ISZ";

        case INST_REG_CLA: return "CLA";
        case INST_REG_CLE: return "CLE";
        case INST_REG_CMA: return "CMA";
        case INST_REG_CME: return "CME";
        case INST_REG_CIR: return "CIR";
        case INST_REG_CIL: return "CIL";
        case INST_REG_INC: return "INC";
        case INST_REG_SPA: return "SPA";
        case INST_REG_SNA: return "SNA";
        case INST_REG_SZA: return "SZA";
        case INST_REG_SZE: return "SZE";
        case INST_REG_HLT: return "HLT";

        case INST_IO_INP: return "INP";
        case INST_IO_OUT: return "OUT";
        case INST_IO_SKI: return "SKI";
        case INST_IO_SKO: return "SKO";
        case INST_IO_ION: return "ION";
        case INST_IO_IOF: return "IOF";

        case INST_PSI_ORG: return "ORG";
        case INST_PSI_DEC: return "DEC";
        case INST_PSI_HEX: return "HEX";
        case INST_PSI_END: return "END";

        default: return "INVALID";
    }
}

void print_tokens(const LexerState *state) {
    for (size_t i = 0; i < state->tokens.size; i++) {
        Token *token = vector_at(
            (Vector *)&state->tokens,
            i
        );

        size_t length = token->lexeme_end - token->lexeme_start;

        printf(
            "TOKEN: %-12s | Lexeme = %.*s",
            token_type_name(token->type),
            (int)length,
            state->src + token->lexeme_start
        );

        if (token->type == TOKEN_DEC_NUMBER ||
            token->type == TOKEN_HEX_NUMBER) {
            printf(" | Number = %d", token->number);
        }

        putchar('\n');
    }
}

void print_symbols(const SymbolTable *table) {
    for (size_t i = 0; i < table->symbols.size; i++) {
        const SymbolLocation *symbol = vector_at_const(&table->symbols, i);
        size_t length = symbol->identifier.lexeme_end - symbol->identifier.lexeme_start;

        printf(
            "SYMBOL: %-8.*s | Location = 0x%03X\n",
            (int)length,
            table->src + symbol->identifier.lexeme_start,
            symbol->location
        );
    }
}

void print_token_lexeme(const char *src, const Token *token) {
    if (token == NULL) {
        printf("<null>");
        return;
    }

    size_t length = token->lexeme_end - token->lexeme_start;

    printf(
        "%.*s",
        (int)length,
        src + token->lexeme_start
    );
}

void print_ast(const ParserState *state) {
    for (size_t i = 0; i < state->statements.size; i++) {
        const Statement *stmt = vector_at_const(&state->statements, i);

        printf("STATEMENT: %-8s | Location = ", instruction_name(stmt->instruction));

        if (stmt->location == INVALID_ADDRESS) {
            printf("INVALID");
        } else {
            printf("0x%03X", stmt->location);
        }

        if (stmt->instruction >= INST_MEM_AND &&
            stmt->instruction <= INST_MEM_ISZ) {
            printf(" | Operand = ");

            if (stmt->operand_pending) {
                print_token_lexeme(state->src, stmt->symbolic_address);
                printf(" (pending)");
            } else {
                printf("0x%03X", stmt->operand);
            }

            if (stmt->indirect) {
                printf(" | Indirect");
            }
        } else if (stmt->instruction == INST_PSI_DEC) {
            printf(" | Operand = %d", (int16_t)stmt->operand);
        } else if (stmt->instruction == INST_PSI_HEX ||
                   stmt->instruction == INST_PSI_ORG) {
            printf(" | Operand = 0x%03X", stmt->operand);
        }

        putchar('\n');
    }
}