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

  // {
  //   // Create root scope

  //   printf("=== Enhanced Type Checker Test ===\n\n");

  //   // Test 1: Basic type creation and symbol addition
  //   printf("1. Testing basic types and symbols:\n");
  //   AstNode *int_type = create_basic_type(allocator, "int", 1, 1);
  //   AstNode *float_type = create_basic_type(allocator, "float", 1, 1);
  //   AstNode *string_type = create_basic_type(allocator, "string", 1, 1);

  //   scope_add_symbol(&root_scope, "x", int_type, true, true, allocator);
  //   scope_add_symbol(&root_scope, "y", float_type, false, true, allocator);
  //   scope_add_symbol(&root_scope, "name", string_type, true, false, allocator);

  //   printf("  Added symbols: x (int), y (float), name (string)\n");

  //   // Test lookups
  //   Symbol *found = scope_lookup(&root_scope, "x");
  //   if (found) {
  //     printf("  Found x: %s, public=%d, mutable=%d\n",
  //            type_to_string(found->type, allocator), found->is_public,
  //            found->is_mutable);
  //   }

  //   // Test 2: Type matching
  //   printf("\n2. Testing type matching:\n");
  //   TypeMatchResult match1 = types_match(int_type, int_type);
  //   TypeMatchResult match2 = types_match(int_type, float_type);
  //   TypeMatchResult match3 = types_match(int_type, string_type);

  //   printf("  int vs int: %s\n",
  //          match1 == TYPE_MATCH_EXACT ? "EXACT" : "NO MATCH");
  //   printf("  int vs float: %s\n", match2 == TYPE_MATCH_COMPATIBLE
  //                                      ? "COMPATIBLE"
  //                                  : match2 == TYPE_MATCH_EXACT ? "EXACT"
  //                                                               : "NO MATCH");
  //   printf("  int vs string: %s\n",
  //          match3 == TYPE_MATCH_NONE ? "NO MATCH" : "MATCH");

  //   // Test 3: Pointer and array types
  //   printf("\n3. Testing complex types:\n");
  //   AstNode *int_ptr = create_pointer_type(allocator, int_type, 2, 1);
  //   AstNode *int_array = create_array_type(allocator, int_type, NULL, 2, 5);

  //   scope_add_symbol(&root_scope, "ptr", int_ptr, true, true, allocator);
  //   scope_add_symbol(&root_scope, "arr", int_array, true, true, allocator);

  //   printf("  Created pointer type: %s\n", type_to_string(int_ptr, allocator));
  //   printf("  Created array type: %s\n", type_to_string(int_array, allocator));
  //   printf("  Is ptr a pointer? %s\n", is_pointer_type(int_ptr) ? "YES" : "NO");
  //   printf("  Is arr an array? %s\n", is_array_type(int_array) ? "YES" : "NO");

  //   // Test 4: Expression typechecking
  //   printf("\n4. Testing expression typechecking:\n");

  //   // Create a simple identifier expression
  //   AstNode *id_expr = create_identifier_expr(allocator, "x", 3, 1);
  //   AstNode *expr_type = typecheck_expression(id_expr, &root_scope, allocator);
  //   if (expr_type) {
  //     printf("  Identifier 'x' has type: %s\n",
  //            type_to_string(expr_type, allocator));
  //   }

  //   // Create a literal expression
  //   int val = 42;
  //   AstNode *lit_expr = create_literal_expr(allocator, LITERAL_INT, &val, 3, 5);
  //   AstNode *lit_type = typecheck_expression(lit_expr, &root_scope, allocator);
  //   if (lit_type) {
  //     printf("  Integer literal has type: %s\n",
  //            type_to_string(lit_type, allocator));
  //   }

  //   // Test 5: Variable declaration
  //   printf("\n5. Testing variable declaration:\n");
  //   AstNode *var_decl = create_var_decl_stmt(allocator, "z", int_type, lit_expr,
  //                                            true, true, 4, 1);
  //   bool success = typecheck_var_decl(var_decl, &root_scope, allocator);
  //   printf("  Variable declaration 'z' typechecked: %s\n",
  //          success ? "SUCCESS" : "FAILED");

  //   // Test 6: Scoped lookups
  //   printf("\n6. Testing nested scopes:\n");
  //   Scope *child_scope =
  //       create_child_scope(&root_scope, "function_scope", allocator);
  //   scope_add_symbol(child_scope, "local_var", float_type, false, true,
  //                    allocator);

  //   Symbol *local = scope_lookup(child_scope, "local_var");
  //   Symbol *global = scope_lookup(child_scope, "x"); // Should find in parent
  //   Symbol *missing = scope_lookup(child_scope, "nonexistent");

  //   printf("  Found local_var in child scope: %s\n", local ? "YES" : "NO");
  //   printf("  Found global x from child scope: %s\n", global ? "YES" : "NO");
  //   printf("  Found nonexistent variable: %s\n", missing ? "YES" : "NO");

  //   // Test 7: Debug output
  //   printf("\n7. Scope hierarchy:\n");
  //   debug_print_scope(&root_scope, 0);
  //   debug_print_scope(child_scope, 1);

  //   printf("\n=== Test Complete ===\n");
  // }

  if (config.name)
    printf("Building target: %s\n", config.name);
  if (config.save)
    printf("Saving output LLVM file.\n");
  if (config.clean)
    printf("Cleaning build artifacts.\n");

  free((void *)source);
  return true;
}
