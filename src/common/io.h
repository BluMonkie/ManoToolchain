#pragma once

#include <stdbool.h>

void io_init();
void io_cleanup();

bool is_input_available();
char get_input_char();

void put_output_char(char c);