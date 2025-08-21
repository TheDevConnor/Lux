/**
 * @file help.h
 * @brief Declarations for command-line parsing, file reading, and build configuration.
 *
 * Provides functions to parse arguments, read files, print help/version/license,
 * and print token debug info.
 */

#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../c_libs/memory/memory.h"
#include "../lexer/lexer.h"

/** Macro to access token at index in a token growable array */
#define TOKEN_AT(i) (((Token *)tokens.data)[(i)])
#define MAX_TOKENS 100

/** Enable debug logs for arena allocator (comment to disable) */
#define DEBUG_ARENA_ALLOC 1

/** Error codes returned by the compiler */
typedef enum {
  ARGC_ERROR = 1,
  FILE_ERROR = 2,
  MEMORY_ERROR = 3,
  LEXER_ERROR = 4,
  PARSER_ERROR = 5,
  RUNTIME_ERROR = 6,
  UNKNOWN_ERROR = 99
} ErrorCode;

/**
 * @brief Configuration structure to hold build options parsed from CLI.
 */
typedef struct {
  const char *filepath; /**< Path to the source file */
  const char *source;   /**< Source code content (optional) */
  const char *name;     /**< Build target name */
  bool save;            /**< Flag to save output */
  bool clean;           /**< Flag to clean build artifacts */
} BuildConfig;

bool check_argc(int argc, int expected);
const char *read_file(const char *filename);

int print_help();
int print_version();
int print_license();

bool parse_args(int argc, char *argv[], BuildConfig *config);
bool run_build(BuildConfig config, ArenaAllocator *allocator);

void print_token(const Token *t);

bool link_with_ld(const char *obj_filename, const char *exe_filename);
bool get_gcc_file_path(const char *filename, char *buffer, size_t buffer_size);
bool get_lib_paths(char *buffer, size_t buffer_size);
bool link_with_ld_simple(const char *obj_filename, const char *exe_filename);
