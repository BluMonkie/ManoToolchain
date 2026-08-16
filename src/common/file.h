#pragma once

char *read_file_string(const char* file_path);
void write_file_buffer(const char* file_path, const void* buffer, const size_t length);
char *replace_or_add_extension(const char *filename, const char *old_extension, const char *new_extension);