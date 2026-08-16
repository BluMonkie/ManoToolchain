# Registers

Registers are described using alphanumeric identifiers.
Example: `AC, PC, MAR, etc.`

## Registers in Mano Basic Computer
| Register symbol | Number of bits | Register name        | Function                     |
| --------------- | -------------: | -------------------- | ---------------------------- |
| `DR`            |             16 | Data register        | Holds memory operand         |
| `AR`            |             12 | Address register     | Holds address for memory     |
| `AC`            |             16 | Accumulator          | Processor register           |
| `IR`            |             16 | Instruction register | Holds instruction code       |
| `PC`            |             12 | Program counter      | Holds address of instruction |
| `TR`            |             16 | Temporary register   | Holds temporary data         |
| `INPR`          |              8 | Input register       | Holds input character        |
| `OUTR`          |              8 | Output register      | Holds output character       |


## Register transfer language
|Symbol|Meaning|
|------|-------|
|Alphanumeric Identifier|Name of a register|
|Paranthesis|Part of a register|
|Arrow|Transfer operation (dest <- source)|
|Arithmetic/Logical symbol|Arithmetic/Logical operation|
|Comma|Separates microinstructions|
|Colon|Executes if control signal = 1|


Apart from this, transfers may also involve the common bus:
Example: `BUS <- R1 , R1 <- R1`
But this notation is rarely used. If the bus is known to exist in the system, the transfer can be written directly between the registers.
