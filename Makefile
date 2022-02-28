SRC_DIR := .
OBJ_DIR := .
BIN_DIR := .

EXE := $(BIN_DIR)/harkka
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CPPFLAGS := -Iinclude -MMD -MP
CFLAGS   := -std=c99 -g -Wall -Wextra -Wno-missing-field-initializers
LDLIBS   := -lm

.PHONY: all clean run

all: 
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDLIBS) -o harkka harkka.c

clean:
	@$(RM) -rv $(EXE)
