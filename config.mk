# config.mk

CC       = gcc
CFLAGS   = -Wall -Wextra -std=c17 -O2
LDFLAGS  =
INCLUDES = -Isrc

# LLVM configuration
LLVM_CFLAGS := $(shell llvm-config --cflags)
LLVM_LDFLAGS := $(shell llvm-config --ldflags --system-libs --libs core analysis bitwriter target)

# Add LLVM flags to existing flags
override CFLAGS += $(LLVM_CFLAGS)
override LDFLAGS += $(LLVM_LDFLAGS)

SRC_DIR  = src
OBJ_DIR  = build

define find_c_sources
$(wildcard $(1)/*.c) \
$(foreach d,$(wildcard $(1)/*),$(call find_c_sources,$(d)))
endef

SRC_FILES := $(call find_c_sources,$(SRC_DIR))
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))
