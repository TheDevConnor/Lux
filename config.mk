# config.mk

CC      = cc
CFLAGS  = -Wall -Wextra -std=c99 -O2
LDFLAGS = 
INCLUDES = -Isrc

SRC_DIR = src
OBJ_DIR = build
BIN     = crust

SRC_FILES := $(shell find $(SRC_DIR) -name '*.c')
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

