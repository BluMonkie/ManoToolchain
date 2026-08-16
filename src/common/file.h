#pragma once

char *read_file_string(const char* file_path);
void write_file_buffer(const char* file_path, const void* buffer, const size_t length);