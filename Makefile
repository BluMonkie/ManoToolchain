CC := gcc

CFLAGS := -Wall -Wextra -std=c23
CPPFLAGS := -Isrc

ASSEMBLER_CPPFLAGS := $(CPPFLAGS) -Isrc/assembler
EMULATOR_CPPFLAGS := $(CPPFLAGS) -Isrc/emulator

COMMON_SRC := $(wildcard src/common/*.c)
ASSEMBLER_SRC := $(wildcard src/assembler/*.c)
EMULATOR_SRC := $(wildcard src/emulator/*.c)

COMMON_OBJ := $(COMMON_SRC:src/%.c=build/%.o)
ASSEMBLER_OBJ := $(ASSEMBLER_SRC:src/%.c=build/%.o)
EMULATOR_OBJ := $(EMULATOR_SRC:src/%.c=build/%.o)

.PHONY: all assembler emulator clean

all: assembler emulator

assembler: bin/assembler

emulator: bin/emulator

bin/assembler: $(COMMON_OBJ) $(ASSEMBLER_OBJ)
	@mkdir -p $(@D)
	$(CC) $^ -o $@

bin/emulator: $(COMMON_OBJ) $(EMULATOR_OBJ)
	@mkdir -p $(@D)
	$(CC) $^ -o $@

build/common/%.o: src/common/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/assembler/%.o: src/assembler/%.c
	@mkdir -p $(@D)
	$(CC) $(ASSEMBLER_CPPFLAGS) $(CFLAGS) -c $< -o $@

build/emulator/%.o: src/emulator/%.c
	@mkdir -p $(@D)
	$(CC) $(EMULATOR_CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build bin