#include <stdlib.h>
#include <stdio.h>
#include <common/file.h>

char *read_file_string(const char* file_path) {
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_length = ftell(file);
    rewind(file);

    char *buffer = malloc(sizeof(char) * (file_length + 1));
    if (buffer == NULL) {
        fprintf(stderr, "ERROR: Out of memory!\n");
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, sizeof(char), file_length, file);

    buffer[bytes_read] = '\0';

    fclose(file);
    return buffer;
}

void write_file_buffer(const char* file_path, const void* buffer, const size_t length) {
    FILE *file = fopen(file_path, "wb");
    if (file == NULL) {
        fprintf(stderr, "Error: Failed to open file '%s'.\n", file_path);
        return;
    }

    size_t elements_written = fwrite(buffer, sizeof(char), length, file);

    if (elements_written < length) {
        fprintf(stderr, "Error: Only wrote %zu elements out of %zu.\n", elements_written, length);
    }
    fclose(file);
}