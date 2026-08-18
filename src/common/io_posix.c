#if defined(__unix__) || defined(__APPLE__)

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>

#include "io.h"

static struct termios original_termios;
static bool terminal_initialized = false;

static void restore_terminal(void) {
    if (!terminal_initialized) {
        return;
    }

    if (tcsetattr(STDIN_FILENO, TCSANOW, &original_termios) == -1) {
        perror("Failed to restore terminal settings");
    }

    terminal_initialized = false;
}

void io_init(void) {
    if (terminal_initialized) {
        return;
    }

    if (tcgetattr(STDIN_FILENO, &original_termios) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    struct termios terminal = original_termios;

    // Disable line buffering and automatic echo.
    terminal.c_lflag &= ~(ICANON | ECHO);

    // Return from read() as soon as at least one character is available.
    terminal.c_cc[VMIN] = 1;
    terminal.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &terminal) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }

    terminal_initialized = true;

    if (atexit(restore_terminal) != 0) {
        restore_terminal();
        fprintf(stderr, "Failed to register terminal cleanup.\n");
        exit(EXIT_FAILURE);
    }
}

void io_cleanup(void) {
    restore_terminal();
}

bool is_input_available(void) {
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    struct timeval timeout = {
        .tv_sec = 0,
        .tv_usec = 0
    };

    int result;

    do {
        result = select(
            STDIN_FILENO + 1,
            &fds,
            NULL,
            NULL,
            &timeout
        );
    } while (result == -1 && errno == EINTR);

    if (result == -1) {
        perror("select");
        return false;
    }

    return result > 0 && FD_ISSET(STDIN_FILENO, &fds);
}

char get_input_char(void) {
    char c;

    ssize_t result;

    do {
        result = read(STDIN_FILENO, &c, 1);
    } while (result == -1 && errno == EINTR);

    if (result != 1) {
        return '\0';
    }

    return c;
}

void put_output_char(char c) {
    if (putchar((unsigned char)c) == EOF) {
        perror("putchar");
        return;
    }

    if (fflush(stdout) == EOF) {
        perror("fflush");
    }
}

#endif