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

all: $(EXE)

$(EXE): $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(EXE)

run: $(EXE)
	$(EXE)

-include $(OBJ:.o=.d)
