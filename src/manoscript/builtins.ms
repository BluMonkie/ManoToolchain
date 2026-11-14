fun print_char(c: i16) {
    asm {
        LDA $c
        OUT
    }
}