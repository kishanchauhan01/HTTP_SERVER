CC = gcc
CFLAGS = -Wall -g

SRC_DIR = src
BUILD_DIR = build

SRC = $(wildcard $(SRC_DIR)/*.c)
OUT = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.out, $(SRC))

all: $(BUILD_DIR) $(OUT)

# Create build folder if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile source files into build folder
$(BUILD_DIR)/%.out: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(BUILD_DIR)
