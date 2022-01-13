CC = gcc
EXE = emu-gb

INCLUDE_DIRS = include
SRC_DIR = src
BIN_DIR = bin

CFLAGS = -g
CPPFLAGS = $(patsubst %,-I%,$(INCLUDE_DIRS))
LDFLAGS =

SRC_FILES = $(wildcard $(SRC_DIR)/*/*.c) $(wildcard $(SRC_DIR)/*.c)
BIN_FILES = $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%.o, $(SRC_FILES))

all: $(EXE)
$(EXE): build
	$(CC) $(BIN_FILES) $(LDFLAGS) -o $@

build: $(BIN_FILES)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@[ -d `dirname $@` ] || mkdir -p `dirname $@`
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -r $(BIN_DIR)

.PHONY: all build clean
