#if defined(_WIN32)

#include <conio.h>

#include "io.h"

void io_init() {}
void io_cleanup() {}

bool is_input_available() {
    return _kbhit() != 0;
}

char get_input_char() {
    return (char) _getch();
}

void put_output_char(char c) {
    _putch(c);
}

#endif