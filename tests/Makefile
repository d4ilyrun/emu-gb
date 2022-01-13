TEST_SRC_DIR = src
TEST_SRC_FILES = $(wildcard $(TEST_SRC_DIR)/*/*.c)

TEST_BIN_DIR = bin
TESTS = $(patsubst $(TEST_SRC_DIR)/%.c, %, $(TEST_SRC_FILES))

TEST_INCLUDE_DIRS = ../include

MAKE += --no-print-directory

CPPFLAGS += $(patsubst %, -I%, $(TEST_INCLUDE_DIRS))
CFLAGS += -g

all: $(TESTS)

prebuild:
	@echo 'Compiling project source files...'
	@$(MAKE) -C ../ build

$(TESTS): prebuild
	@$(MAKE) $(TEST_BIN_DIR)/$@

$(TEST_BIN_DIR)/%: $(TEST_SRC_DIR)/%.c
	@[ -d `dirname $@` ] || mkdir -p `dirname $@`
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(wildcard ../bin/*/*.o) -o $@ $(LDFLAGS)

list:
	@for test in $(TESTS); do echo "> $$test"; done

clean:
	$(RM) -r $(TEST_BIN_DIR)
