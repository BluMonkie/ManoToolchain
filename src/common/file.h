#pragma once

#include <stdint.h>

char *read_file_string(const char* file_path);
bool read_file_buffer(const char *file_path, void **out, size_t *len);
void write_file_buffer(const char* file_path, const void* buffer, const size_t length);
void write_buffer16_be(uint16_t *buf, size_t len, char *output_file_name);
void read_buffer16_be(char *file_name);
char *replace_or_add_extension(const char *filename, const char *old_extension, const char *new_extension);