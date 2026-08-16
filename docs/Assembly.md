# Assembly
Mano basic computer uses its own flavour of assembly.

Each line of an assembly language program is arranged in three fields. The fields specify the following information.

1. The label field may be empty or it may specify a symbolic address.
2. The instruction field specifies a machine instruction or a pseudoinstruction.
3. The comment field may be empty or it may include a comment.

The instruction field may specify one of the following:
1. Memory Reference Instruction (MRI)
2. Register Reference or I/O Instruction (non-MRI)
3. Pseudo Instruction with or without operand

### MR-Instructions
An MRI contains an instruction mnemonic along with a symbolic address.
Additionally, an MRI operation may be followed by `I` to indicate indirect addressing mode.

Example: `ADD MIN`, `STA DIF I`, etc.

### Non MR-Instructions
These instructions do not have an operand.
Example: `INC`, `CMA`, etc.

### Pseudo-Instructions
Pseudo instructions are not real machine instructions but rather instructions to the assembler giving information about some phase of translation.
The mano assembly contains 4 pseudo-instructions.
| Symbol | Information for the Assembler |
|:-------|:-------------------------------|
| `ORG N` | Hexadecimal number `N` is the memory location for the instruction or operand listed in the following line. |
| `END` | Denotes the end of the symbolic program. |
| `DEC N` | Signed decimal number `N` to be converted to binary. |
| `HEX N` | Hexadecimal number `N` to be converted to binary. |

### Labels
Labels are used to create symbolic addresses. These addresses are then used to perform MR-Instructions. 
Labels in Mano assembly
- Must start with a letter
- May contain letters and numbers (except the first letter)
- Must have no more than 3 characters

Labels must be followed by a comma(`,`) and then an instruction.

### Comments
Comments in mano assembly start with `/` and continue till the end of the line.

### Whitespace
Mano assembly does not treat whitespace as significant, except newlines.
Newlines(`\n`) are used to terminate statements. Thus, one line can only contain a single statement.

## Grammar
This grammar defines the assmebly syntax in EBNF:
```ebnf
program = { statement , NEWLINE } , end , [NEWLINE] ;
statement = label | instruction;
label = IDENTIFIER , "," , instruction ;
instruction = mri | nmri | psi;
mri = mri_instruction , IDENTIFIER , [ "I" ];
nmri = nmri_instruction ;
psi = psi_instruction ;

end = "END" ;
mri_instruction = "AND" | "ADD" | "LDA" | "STA" | "BUN" | "BSA" | "ISZ" ;
nmri_instruction = "CLA" | "CLE" | "CMA" | "CME" | "CIR" | "CIL" | "INC" | "SPA" | "SNA" | "SZA" | "SZE" | "HLT" | "INP" | "OUT" | "SKI" | "SKO" | "ION" | "IOF" ;
psi_instruction = ("ORG" , HEX_NUMBER) | ("DEC" , DEC_NUMBER) | ("HEX" , HEX_NUMBER) ;
```

### Lexical Definitions
The following lexical elements are defined using regular expressions:

IDENTIFIER    = `[A-Za-z][A-Za-z0-9]{0,2}`
DEC_NUMBER    = `-?[0-9]+`
HEX_NUMBER    = `[0-9A-Fa-f]+`