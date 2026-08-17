#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "common/file.h"
#include "common/xmalloc.h"

char *read_file_string(const char *file_path) {
    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }

    long file_length = ftell(file);
    if (file_length < 0) {
        fclose(file);
        return NULL;
    }

    rewind(file);

    char *buffer = xmalloc((size_t)file_length + 1);

    size_t bytes_read = fread(buffer, 1, (size_t)file_length, file);

    if (bytes_read != (size_t)file_length) {
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[bytes_read] = '\0';

    fclose(file);
    return buffer;
}

bool read_file_buffer(const char *file_path, void **out, size_t *len) {
    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        return false;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return false;
    }

    long file_length = ftell(file);
    if (file_length < 0) {
        fclose(file);
        return false;
    }

    rewind(file);

    size_t size = (size_t)file_length;
    void *buffer = xmalloc(size);

    size_t bytes_read = fread(buffer, 1, size, file);

    if (bytes_read != size) {
        free(buffer);
        fclose(file);
        return false;
    }

    fclose(file);

    *out = buffer;
    *len = bytes_read;

    return true;
}

bool write_file_buffer(const char* file_path, const void* buffer, const size_t length) {
    FILE *file = fopen(file_path, "wb");
    if (file == NULL) {
        fprintf(stderr, "Error: Failed to open file '%s'.\n", file_path);
        return false;
    }

    size_t elements_written = fwrite(buffer, sizeof(char), length, file);

    if (elements_written != length) {
        fprintf(stderr, "Error: Only wrote %zu elements out of %zu.\n", elements_written, length);
        return false;
    }
    fclose(file);
    return true;
}

char *replace_or_add_extension(const char *filename, const char *old_extension, const char *new_extension) {
    size_t filename_len = strlen(filename);
    size_t old_len = strlen(old_extension);
    size_t new_len = strlen(new_extension);

    if (filename_len >= old_len &&
        strcmp(filename + filename_len - old_len, old_extension) == 0) {
        filename_len -= old_len;
    }

    char *result = xmalloc(filename_len + new_len + 1);

    memcpy(result, filename, filename_len);
    strcpy(result + filename_len, new_extension);

    return result;
}

void write_buffer16_be(const uint16_t *buf, const size_t len, const char *output_file_name) {
    uint8_t *buffer = xmalloc(len * 2 * sizeof(uint8_t));

    for (size_t i = 0; i < len; i++) {
        uint16_t word = buf[i];

        buffer[i * 2]     = (uint8_t)(word >> 8);
        buffer[i * 2 + 1] = (uint8_t)word;
    }

    write_file_buffer(output_file_name, buffer, len * 2 * sizeof(uint8_t));
    free(buffer);
}

bool read_buffer16_be(const char *file_path, uint16_t **out, size_t *length) {
    void *raw_buffer;
    size_t len;
    bool read = read_file_buffer(file_path, &raw_buffer, &len);
    if (!read) {
        return false;
    }
    
    uint8_t *buffer = raw_buffer;
    size_t new_len = (len + 1) / 2;
    uint16_t *buf16 = xmalloc(sizeof(uint16_t) * new_len);

    for (int i = 0; i < new_len; i++) {
        uint16_t hi = buffer[2 * i];
        uint16_t lo = (2 * i + 1 < len) ? buffer[2 * i + 1] : 0;

        buf16[i] = (hi << 8) | lo;
    }

    *length = new_len;
    *out = buf16;
    free(buffer);
    return true;
}
