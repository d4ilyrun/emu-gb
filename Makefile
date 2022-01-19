CC = gcc
EXE = emu-gb

INCLUDE_DIRS = include
SRC_DIR = src
BIN_DIR = bin

CFLAGS = -Wall -Wextra -Wno-unused-result -Wno-missing-field-initializers -Wno-unknown-pragmas
CPPFLAGS = $(patsubst %,-I%,$(INCLUDE_DIRS))
LDFLAGS =

DEBUG_FLAGS = -DNDEBUG -Og -g
OPTI_FLAGS = -O3

SRC_FILES = $(wildcard $(SRC_DIR)/*/*.c) $(wildcard $(SRC_DIR)/*.c)
BIN_FILES = $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%.o, $(SRC_FILES))

all: $(EXE)
$(EXE): build
	$(CC) $(BIN_FILES) $(LDFLAGS) -o $@

build:	CFLAGS += $(OPTI_FLAGS)
build: $(BIN_FILES)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean $(BIN_FILES)
	$(CC) $(BIN_FILES) $(LDFLAGS) -o $(EXE)

test: CPPFLAGS += -DTEST_ROM
test: clean $(BIN_FILES)
	$(CC) $(BIN_FILES) $(LDFLAGS) -o $(EXE)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@[ -d `dirname $@` ] || mkdir -p `dirname $@`
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -r $(BIN_DIR) $(EXE)

clang-format:
	@find . -type f -name '*.[ch]' -exec clang-format --style=file -i {} ';'

.PHONY: all build clean debug clang-format
