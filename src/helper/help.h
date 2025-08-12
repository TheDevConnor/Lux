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

/**
 * @brief Check if argc is at least expected number.
 *
 * @param argc Actual argc
 * @param expected Expected argc
 * @return true if argc >= expected, false otherwise
 */
bool check_argc(int argc, int expected);

/**
 * @brief Reads entire file content into memory.
 *
 * @param filename Path to the file
 * @return Pointer to file content or NULL if error
 */
const char *read_file(const char *filename);

/**
 * @brief Prints the help message describing usage and options.
 *
 * @return Always returns 0
 */
int print_help();

/**
 * @brief Prints the version string.
 *
 * @return Always returns 0
 */
int print_version();

/**
 * @brief Prints license information.
 *
 * @return Always returns 0
 */
int print_license();

/**
 * @brief Parses command-line arguments into a BuildConfig.
 *
 * @param argc Argument count
 * @param argv Argument vector
 * @param config Output config struct to fill
 * @return false if help/version/license printed or error, true otherwise
 */
bool parse_args(int argc, char *argv[], BuildConfig *config);

/**
 * @brief Runs the build given the configuration and memory allocator.
 *
 * @param config Build configuration options
 * @param allocator Arena allocator for memory management
 * @return true on success, false on failure
 */
bool run_build(BuildConfig config, ArenaAllocator *allocator);

/**
 * @brief Prints a token's textual representation and token type.
 *
 * @param t Pointer to token to print
 */
void print_token(const Token *t);
