CC := gcc

CFLAGS := -Wall -Wextra -std=c23 -MMD -MP
CPPFLAGS := -Isrc

ASSEMBLER_CPPFLAGS := $(CPPFLAGS) -Isrc/assembler
EMULATOR_CPPFLAGS := $(CPPFLAGS) -Isrc/emulator
MANOSCRIPT_CPPFLAGS := $(CPPFLAGS) -Isrc/manoscript

COMMON_SRC := $(wildcard src/common/*.c)
ASSEMBLER_SRC := $(wildcard src/assembler/*.c)
EMULATOR_SRC := $(wildcard src/emulator/*.c)
MANOSCRIPT_SRC := $(wildcard src/manoscript/*.c)

COMMON_OBJ := $(COMMON_SRC:src/%.c=build/%.o)
ASSEMBLER_OBJ := $(ASSEMBLER_SRC:src/%.c=build/%.o)
EMULATOR_OBJ := $(EMULATOR_SRC:src/%.c=build/%.o)
MANOSCRIPT_OBJ := $(MANOSCRIPT_SRC:src/%.c=build/%.o)

ALL_OBJ := $(COMMON_OBJ) $(ASSEMBLER_OBJ) $(EMULATOR_OBJ) $(MANOSCRIPT_OBJ)

ifeq ($(OS),Windows_NT)
    CLEAN_CMD := if exist build rmdir /s /q build & if exist bin rmdir /s /q bin
else
    CLEAN_CMD := rm -rf build bin
endif

.PHONY: all assembler emulator clean

all: assembler emulator manoscript

assembler: bin/manoasm

emulator: bin/manoemu

manoscript: bin/manoscript

clean:
	$(CLEAN_CMD)

bin/manoasm: $(COMMON_OBJ) $(ASSEMBLER_OBJ)
	@mkdir -p $(@D)
	$(CC) $^ -o $@

bin/manoemu: $(COMMON_OBJ) $(EMULATOR_OBJ)
	@mkdir -p $(@D)
	$(CC) $^ -o $@

bin/manoscript: $(COMMON_OBJ) $(MANOSCRIPT_OBJ)
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

build/manoscript/%.o: src/manoscript/%.c
	@mkdir -p $(@D)
	$(CC) $(MANOSCRIPT_CPPFLAGS) $(CFLAGS) -c $< -o $@

-include $(ALL_OBJ:.o=.d)