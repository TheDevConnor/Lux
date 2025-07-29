#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../c_libs/memory/memory.h"
#include "../lexer/lexer.h"

#define TOKEN_AT(i) (((Token *)tokens.data)[(i)])
#define MAX_TOKENS 100

#define DEBUG_ARENA_ALLOC 1  // Comment this line to disable debug logs

typedef enum {
  ARGC_ERROR = 1,
  FILE_ERROR = 2,
  MEMORY_ERROR = 3,
  LEXER_ERROR = 4,
  PARSER_ERROR = 5,
  RUNTIME_ERROR = 6,
  UNKNOWN_ERROR = 99
} ErrorCode;

typedef struct {
  const char *filepath;
  const char *source;
  const char *name;
  bool save, clean;
} BuildConfig;

bool check_argc(int argc, int expected);
const char *read_file(const char *filename);

int print_help();
int print_version();
int print_license();

bool parse_args(int argc, char *argv[], BuildConfig *config);
bool run_build(BuildConfig config, ArenaAllocator *allocator);

void print_token(const Token *t);
