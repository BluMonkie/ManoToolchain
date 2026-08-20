#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/file.h"
#include "common/io.h"
#include "executor.h"
#include "decoder.h"
#include "cli.h"

void print_help() {
    printf("ManoEMU - A mano basic assembly assembler\n");
    printf("Usage: manoemu [options] input_file.mano\n");
    printf("Valid options: --help, --version\n");
}

void print_version() {
    printf("ManoEMU - A mano basic instruction emulator\n");
    printf("Version v1.0.0\n");
}

int main(int argc, char **argv) {
    CliOptions options = parse_cli_arguments(argc, argv);

    if (options.had_error) {
        exit(EXIT_FAILURE);
    }

    if (options.print_help) {
        print_help();
        exit(EXIT_SUCCESS);
    }

    if (options.print_version) {
        print_version();
        exit(EXIT_SUCCESS);
    }

    if (options.input_file_path == NULL) {
        fprintf(stderr, "ERROR: Missing source file. See manoemu --help.\n");
        exit(EXIT_FAILURE);
    }

    io_init();
    System system = {0};

    uint16_t *prog;
    size_t prog_len;
    bool success = read_buffer16_be(argv[1], &prog, &prog_len);

    if (!success || prog_len > MANO_MEMORY_SIZE) {
        abort();
    }

    memcpy(system.mem.data, prog, prog_len * sizeof(Word));
    free(prog);
    execute_instructions(&system);

}