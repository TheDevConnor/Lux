/**
 * @file run.c
 * @brief Implements the main build run logic: reading source, lexing,
 *        parsing, error reporting, and printing AST.
 */

#include "../ast/ast_utils.h"
#include "../c_libs/error/error.h"
#include "../parser/parser.h"
#include "../typechecker/type.h"
#include "help.h"

/**
 * @brief Runs the build process using given configuration and allocator.
 *
 * Reads the source file, lexes tokens into a growable array,
 * reports errors if any, parses AST, reports errors again,
 * prints AST, and outputs build status.
 *
 * @param config BuildConfig structure with user options.
 * @param allocator ArenaAllocator for memory allocation.
 * @return true if build succeeded, false if errors or failures occurred.
 */
bool run_build(BuildConfig config, ArenaAllocator *allocator) {
  const char *source = read_file(config.filepath);
  if (!source) {
    fprintf(stderr, "Failed to read source file: %s\n", config.filepath);
    return false;
  }

  Lexer lexer;
  init_lexer(&lexer, source, allocator);

  GrowableArray tokens;
  if (!growable_array_init(&tokens, allocator, MAX_TOKENS, sizeof(Token))) {
    fprintf(stderr, "Failed to initialize token array.\n");
    free((void *)source);
    return false;
  }

  Token tk;
  while ((tk = next_token(&lexer)).type_ != TOK_EOF) {
    Token *slot = (Token *)growable_array_push(&tokens);
    if (!slot) {
      fprintf(stderr, "Out of memory while growing token array\n");
      free((void *)source);
      return false;
    }
    *slot = tk;
  }
  if (error_report()) {
    free((void *)source);
    return false;
  }

  AstNode *root = parse(&tokens, allocator);
  if (error_report()) {
    free((void *)source);
    return false;
  }
  // print_ast(root, "", true, true);

  Scope root_scope;
  init_scope(&root_scope, NULL, "global", allocator);

  bool tc = typecheck(root, &root_scope, allocator);
  debug_print_scope(&root_scope, 0);

  if (config.name)
    printf("Building target: %s\n", config.name);
  if (config.save)
    printf("Saving output LLVM file.\n");
  if (config.clean)
    printf("Cleaning build artifacts.\n");

  free((void *)source);
  return true;
}
