/**
 * @file run.c
 * @brief Implements the main build run logic: reading source, lexing,
 *        parsing, error reporting, and printing AST.
 */

#include "../ast/ast_utils.h"
#include "../c_libs/error/error.h"
#include "../llvm/llvm.h"
#include "../parser/parser.h"
#include "../typechecker/type.h"
#include "help.h"

// Helper to save all output files
void save_output_files(CodeGenContext *ctx, const char *base_name) {
  char filename[256];
  
  // Save readable LLVM IR
  snprintf(filename, sizeof(filename), "%s.ll", base_name);
  char *ir = print_llvm_ir(ctx);
  if (ir) {
    FILE *f = fopen(filename, "w");
    if (f) {
      fprintf(f, "%s", ir);
      fclose(f);
      printf("✓ LLVM IR saved to %s\n", filename);
    }
    LLVMDisposeMessage(ir);
  }

  printf("✓ LLVM bitcode saved to %s.bc\n", base_name);

  // Generate object file
  printf("\n=== Object File Generation ===\n");
  snprintf(filename, sizeof(filename), "%s.o", base_name);
  if (generate_object_file(ctx, filename)) {
    printf("✓ Object file saved to %s\n", filename);
    
    // Generate assembly file
    snprintf(filename, sizeof(filename), "%s.s", base_name);
    if (generate_assembly_file(ctx, filename)) {
      printf("✓ Assembly file saved to %s\n", filename);
    }
    
    // Link executable
    printf("\n=== Linking ===\n");
    snprintf(filename, sizeof(filename), "%s.o", base_name);
    char exe_name[256];
    snprintf(exe_name, sizeof(exe_name), "%s", base_name);
    
    if (link_with_ld_simple(filename, exe_name)) {
      printf("✓ Executable created: %s\n", exe_name);
      printf("Run with: ./%s\n", exe_name);
    } else {
      printf("✗ Linking failed! Manual linking options:\n");
      printf("  gcc %s -o %s\n", filename, base_name);
      printf("  clang %s -o %s\n", filename, base_name);
    }
  } else {
    printf("✗ Object file generation failed!\n");
  }
}

// Simplified LLVM code generation helper
bool generate_llvm_code(AstNode *root, BuildConfig config, ArenaAllocator *allocator) {
  printf("\n=== LLVM Code Generation ===\n");

  // Initialize LLVM context
  CodeGenContext *ctx = init_codegen_context(allocator, "main_module");
  if (!ctx) {
    fprintf(stderr, "Failed to initialize LLVM codegen context\n");
    return false;
  }

  const char *base_name = config.name ? config.name : "output";
  
  // Generate IR
  char bc_file[256];
  if (config.save) {
    snprintf(bc_file, sizeof(bc_file), "%s.bc", base_name);
  }
  
  bool success = generate_llvm_ir(ctx, root, config.save ? bc_file : NULL);
  if (!success) {
    printf("✗ LLVM IR generation failed!\n");
    cleanup_codegen_context(ctx);
    return false;
  }

  printf("✓ LLVM IR generation successful!\n");

  // Save files if requested
  if (config.save) {
    save_output_files(ctx, base_name);
  }

  cleanup_codegen_context(ctx);
  return true;
}

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
  bool success = false;
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
    goto cleanup;
  }

  Token tk;
  while ((tk = next_token(&lexer)).type_ != TOK_EOF) {
    Token *slot = (Token *)growable_array_push(&tokens);
    if (!slot) {
      fprintf(stderr, "Out of memory while growing token array\n");
      goto cleanup;
    }
    *slot = tk;
  }
  if (error_report())
    goto cleanup;

  AstNode *root = parse(&tokens, allocator);
  if (error_report())
    goto cleanup;
  print_ast(root, "", true, true);

  Scope root_scope;
  init_scope(&root_scope, NULL, "global", allocator);

  bool tc = typecheck(root, &root_scope, allocator);
  debug_print_scope(&root_scope, 0);

  if (tc) {
    success = generate_llvm_code(root, config, allocator);
  } else {
    printf("Skipping LLVM code generation due to type checking errors.\n");
  }

  if (config.name)
    printf("Building target: %s\n", config.name);
  if (config.clean)
    printf("Cleaning build artifacts.\n");

cleanup:
  free((void *)source);
  return success;
}