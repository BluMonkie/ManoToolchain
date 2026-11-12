#pragma once

#include <stdio.h>

#include <common/vector.h>
#include "ast.h"

void print_ast(Program *program, FILE *stream);
void print_tokens(FILE *stream, const Vector tokens);