#pragma once

#include <stdint.h>
#include <stdbool.h>

char *read_file_string(const char* file_path);
bool read_file_buffer(const char *file_path, void **out, size_t *len);
bool write_file_buffer(const char* file_path, const void* buffer, const size_t length);
char *replace_or_add_extension(const char *filename, const char *old_extension, const char *new_extension);
void write_buffer16_be(const uint16_t *buf, const size_t len, const char *output_file_name);
bool read_buffer16_be(const char *file_path, uint16_t **out, size_t *length);