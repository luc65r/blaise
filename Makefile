TARGET_EXEC := blaise

CC := gcc
CFLAGS := -std=gnu11
CFLAGS += -Og -march=native
CFLAGS += -ggdb
CFLAGS += -Wall -Wextra
CFLAGS += -Werror=switch
CFLAGS += -MMD -MP
LDFLAGS := -lunistring -lgmp -ljansson
FLEX := flex
FFLAGS :=
BISON := bison
BFLAGS := -t -v

SRC_DIR := src
BUILD_DIR := build

vpath %.c $(SRC_DIR)
vpath %.h $(SRC_DIR)
vpath scan.% $(BUILD_DIR)
vpath parse.% $(BUILD_DIR)

CFLAGS += -I$(SRC_DIR) -I$(BUILD_DIR)

SRCS := $(wildcard $(SRC_DIR)/*.c) $(BUILD_DIR)/scan.c $(BUILD_DIR)/parse.c
OBJS := $(patsubst %.c, %.o, $(patsubst $(SRC_DIR)/%, $(BUILD_DIR)/%, $(SRCS)))
DEPS := $(OBJS:.o=.d)

all: $(BUILD_DIR)/$(TARGET_EXEC)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(CFLAGS)

$(BUILD_DIR)/main.o: $(BUILD_DIR)/scan.h $(BUILD_DIR)/parse.h

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/scan.c $(BUILD_DIR)/scan.h &: $(SRC_DIR)/scan.l $(BUILD_DIR)/parse.h
	@mkdir -p $(dir $@)
	$(FLEX) $(FFLAGS) -o $(BUILD_DIR)/scan.c --header-file=$(BUILD_DIR)/scan.h $<

$(BUILD_DIR)/parse.c $(BUILD_DIR)/parse.h &: $(SRC_DIR)/parse.y
	@mkdir -p $(dir $@)
	$(BISON) $(BFLAGS) -o $(BUILD_DIR)/parse.c --defines=$(BUILD_DIR)/parse.h $<

test_ast: $(BUILD_DIR)/$(TARGET_EXEC)
	cd tests && ./ast.sh $(abspath $<)

test_eval: $(BUILD_DIR)/$(TARGET_EXEC)
	cd tests && ./eval.sh $(abspath $<)

test: test_ast test_eval

clean:
	rm -rf $(BUILD_DIR)

.SUFFIXES:

.PHONY: clean test_ast test_eval test

-include $(DEPS)