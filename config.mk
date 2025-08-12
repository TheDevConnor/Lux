# config.mk

CC       = gcc
CFLAGS   = -Wall -Wextra -std=c17 -O2
LDFLAGS  =
INCLUDES = -Isrc

SRC_DIR  = src
OBJ_DIR  = build

define find_c_sources
$(wildcard $(1)/*.c) \
$(foreach d,$(wildcard $(1)/*),$(call find_c_sources,$(d)))
endef

SRC_FILES := $(call find_c_sources,$(SRC_DIR))
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))
