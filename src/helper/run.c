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
  print_ast(root, "", true, true);

  Scope root_scope;
  init_scope(&root_scope, NULL, "global", allocator);

  bool tc = typecheck(root, &root_scope, allocator);
  debug_print_scope(&root_scope, 0);

  // Add LLVM IR generation if typechecking passed
  if (tc) {
    printf("\n=== LLVM Code Generation ===\n");

    // Initialize code generation context
    CodeGenContext *codegen_ctx =
        init_codegen_context(allocator, "main_module");
    if (!codegen_ctx) {
      fprintf(stderr, "Failed to initialize LLVM codegen context\n");
      free((void *)source);
      return false;
    }

    // Generate LLVM IR with name-based bitcode file
    char bc_filename[256];
    const char *base_name = config.name ? config.name : "output";
    if (config.save) {
      snprintf(bc_filename, sizeof(bc_filename), "%s.bc", base_name);
    }
    const char *output_file = config.save ? bc_filename : NULL;
    bool codegen_success = generate_llvm_ir(codegen_ctx, root, output_file);

    if (codegen_success) {
      printf("✓ LLVM IR generation successful!\n");

      // Print the generated IR
      char *ir = print_llvm_ir(codegen_ctx);
      if (ir) {
        printf("\nGenerated LLVM IR:\n");
        printf("==================\n");
        printf("%s\n", ir);
        LLVMDisposeMessage(ir);
      }

      if (config.save) {
        printf("✓ LLVM bitcode saved to %s\n", bc_filename);

        // Determine base name for output files
        const char *base_name = config.name ? config.name : "output";

        // Create filenames with proper extension
        char ir_filename[256];
        char obj_filename[256];
        char asm_filename[256];
        snprintf(ir_filename, sizeof(ir_filename), "%s.ll", base_name);
        snprintf(obj_filename, sizeof(obj_filename), "%s.o", base_name);
        snprintf(asm_filename, sizeof(asm_filename), "%s.s", base_name);

        // Save human-readable IR
        char *ir_readable = print_llvm_ir(codegen_ctx);
        if (ir_readable) {
          FILE *ir_file = fopen(ir_filename, "w");
          if (ir_file) {
            fprintf(ir_file, "%s", ir_readable);
            fclose(ir_file);
            printf("✓ Human-readable LLVM IR saved to %s\n", ir_filename);
          }
          LLVMDisposeMessage(ir_readable);
        }

        // Generate object file
        printf("\n=== Object File Generation ===\n");
        if (generate_object_file(codegen_ctx, obj_filename)) {
          printf("✓ Object file saved to %s\n", obj_filename);

          // Also generate assembly for debugging (optional)
          if (generate_assembly_file(codegen_ctx, asm_filename)) {
            printf("✓ Assembly file saved to %s\n", asm_filename);
          }

          // Show how to create executable
          printf("\n=== Creating Executable ===\n");
          printf("To create an executable, run:\n");
          printf("  gcc %s -o %s\n", obj_filename, base_name);
          printf("or\n");
          printf("  clang %s -o %s\n", obj_filename, base_name);

        } else {
          printf("✗ Object file generation failed!\n");
        }
      }
    } else {
      printf("✗ LLVM IR generation failed!\n");
    }

    // Cleanup codegen context
    cleanup_codegen_context(codegen_ctx);
  } else {
    printf("Skipping LLVM code generation due to type checking errors.\n");
  }

  if (config.name)
    printf("Building target: %s\n", config.name);
  if (config.clean)
    printf("Cleaning build artifacts.\n");

  free((void *)source);
  return tc; // Return true only if everything succeeded including typecheck
}
