#include <stdio.h>
#include <stdlib.h>

#include "executor.h"
#include "decoder.h"

static void execute_instruction(System *sys, Instruction inst);
static void execute_mr_instruction(System *sys, Instruction inst);
static void execute_rr_instruction(System *sys, Instruction inst);
static void execute_io_instruction(System *sys, Instruction inst);

void execute_instructions(System *sys) {
    while (!sys->halted) {
        Word inst_word = sys->mem.data[sys->cpu.PC];
        
        Instruction inst = decode_instruction(inst_word);
        if (inst.opcode == INST_INVALID) {
            fprintf(stderr, "Error: Invalid instruction %x at %x.\n", inst_word, sys->cpu.PC);
            exit(EXIT_FAILURE);
        }
        sys->cpu.PC++;

        execute_instruction(sys, inst);
    }
}

void execute_instruction(System *sys, Instruction inst) {
    InstOpcode opcode = inst.opcode;
    if (opcode <= INST_ISZ) {
        execute_mr_instruction(sys, inst);
    } else if (opcode <= INST_HLT) {
        execute_rr_instruction(sys, inst);
    } else if (opcode <= INST_IOF) {
        execute_io_instruction(sys, inst);
    } else {
        fprintf(stderr, "Error: Invalid instruction\n");
        exit(EXIT_FAILURE);
    }
}

void execute_mr_instruction(System *sys, Instruction inst) {
    Address effective_address = inst.operand;
    if (inst.is_indirect) {
        effective_address = sys->mem.data[effective_address] & 0x0FFF;
    }

    switch (inst.opcode) {
        case INST_AND:
            sys->cpu.AC &= sys->mem.data[effective_address];
            break;
        case INST_ADD: {
            Word ac = sys->cpu.AC;
            Word mem = sys->mem.data[effective_address];

            uint32_t sum = (uint32_t) ac + (uint32_t) mem;
            sys->cpu.AC = (Word) sum;
            sys->cpu.E = (sum >> 16) & 1;
            break;
        }
        case INST_LDA:
            sys->cpu.AC = sys->mem.data[effective_address];
            break;
        case INST_STA:
            sys->mem.data[effective_address] = sys->cpu.AC;
            break;
        case INST_BUN:
            sys->cpu.PC = effective_address;
            break;
        case INST_BSA:
            sys->mem.data[effective_address] = sys->cpu.PC;
            sys->cpu.PC = effective_address + 1;
            break;
        case INST_ISZ:
            sys->mem.data[effective_address]++;

            if (sys->mem.data[effective_address] == 0) {
                sys->cpu.PC++;
            }
            break;
        default:
            break;
    }
}

void execute_rr_instruction(System *sys, Instruction inst) {
    switch (inst.opcode) {
        case INST_CLA:
            sys->cpu.AC = 0;
            break;
        case INST_CLE:
            sys->cpu.E = 0;
            break;
        case INST_CMA:
            sys->cpu.AC = ~sys->cpu.AC;
            break;
        case INST_CME:
            sys->cpu.E = !sys->cpu.E;
            break;
        case INST_CIR:
            sys->cpu.E = sys->cpu.AC & 1u;
            sys->cpu.AC = (sys->cpu.AC >> 1) | (sys->cpu.E << 15);
            break;
        case INST_CIL:
            sys->cpu.E = (sys->cpu.AC >> 15) & 1u;
            sys->cpu.AC = (sys->cpu.AC << 1) | sys->cpu.E;
            break;
        case INST_INC:
            sys->cpu.AC++;
            break;
        case INST_SPA:
            if ((int16_t) sys->cpu.AC > 0) {
                sys->cpu.PC++;
            } 
            break;
        case INST_SNA:
            if ((int16_t) sys->cpu.AC < 0) {
                sys->cpu.PC++;
            } 
            break;
        case INST_SZA:
            if (sys->cpu.AC == 0) {
                sys->cpu.PC++;
            } 
            break;
        case INST_SZE:
            if (!sys->cpu.E) {
                sys->cpu.PC++;
            } 
            break;
        case INST_HLT:
            sys->halted = true;
            break;
        default:
            break;
    }
}

// TODO: Document and Implement async I/O behaviour
void execute_io_instruction(System *sys, Instruction inst) {
    (void)sys;
    (void)inst;

    fprintf(stderr, "ERROR: I/O instructions aren't implemented yet.\n");
    exit(EXIT_FAILURE);
}