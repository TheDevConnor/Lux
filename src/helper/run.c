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
bool generate_llvm_code(AstNode *root, BuildConfig config,
                        ArenaAllocator *allocator) {
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

// Helper function to parse a single file and extract its module
Stmt *parse_file_to_module(const char *path, size_t position,
                           ArenaAllocator *allocator) {
  const char *source = read_file(path);
  if (!source) {
    fprintf(stderr, "Failed to read source file: %s\n", path);
    return NULL;
  }

  Lexer lexer;
  init_lexer(&lexer, source, allocator);

  GrowableArray tokens;
  if (!growable_array_init(&tokens, allocator, MAX_TOKENS, sizeof(Token))) {
    fprintf(stderr, "Failed to initialize token array for %s.\n", path);
    free((void *)source);
    return NULL;
  }

  Token tk;
  while ((tk = next_token(&lexer)).type_ != TOK_EOF) {
    Token *slot = (Token *)growable_array_push(&tokens);
    if (!slot) {
      fprintf(stderr, "Out of memory while growing token array\n");
      free((void *)source);
      return NULL;
    }
    *slot = tk;
  }

  if (error_report()) {
    free((void *)source);
    return NULL;
  }

  // Parse and extract the module from the program
  AstNode *program_root = parse(&tokens, allocator);
  free((void *)source);

  if (!program_root) {
    return NULL;
  }

  // Extract the first (and should be only) module from the program
  if (program_root->type == AST_PROGRAM &&
      program_root->stmt.program.module_count > 0) {
    Stmt *module = (Stmt *)program_root->stmt.program.modules[0];

    // Update the module's position if it's actually a module
    if (module && module->type == AST_PREPROCESSOR_MODULE) {
      // Assuming your module structure has a position field - adjust based on
      // your actual AST structure This might be module->data.module.position =
      // position; depending on your struct layout
      module->preprocessor.module.potions = position;
    }

    return module;
  }

  return NULL;
}

AstNode *lex_and_parse_file(const char *path, ArenaAllocator *allocator) {
  const char *source = read_file(path);
  if (!source) {
    fprintf(stderr, "Failed to read source file: %s\n", path);
    return NULL;
  }

  Lexer lexer;
  init_lexer(&lexer, source, allocator);

  GrowableArray tokens;
  if (!growable_array_init(&tokens, allocator, MAX_TOKENS, sizeof(Token))) {
    fprintf(stderr, "Failed to initialize token array for %s.\n", path);
    free((void *)source);
    return NULL;
  }

  Token tk;
  while ((tk = next_token(&lexer)).type_ != TOK_EOF) {
    Token *slot = (Token *)growable_array_push(&tokens);
    if (!slot) {
      fprintf(stderr, "Out of memory while growing token array\n");
      free((void *)source);
      return NULL;
    }
    *slot = tk;
  }

  if (error_report()) {
    free((void *)source);
    return NULL;
  }

  AstNode *root = parse(&tokens, allocator);

  free((void *)source);
  return root;
}

/**
 * @brief Runs the build process using given configuration and allocator.
 *
 * Reads the source files, lexes tokens, parses into modules,
 * combines them into a single Program node, and runs type checking and code
 * generation.
 *
 * @param config BuildConfig structure with user options.
 * @param allocator ArenaAllocator for memory allocation.
 * @return true if build succeeded, false if errors or failures occurred.
 */
void debug_ast_module_structure(AstNode *program_node) {
  if (!program_node || program_node->type != AST_PROGRAM) {
    printf("DEBUG: Not a program node\n");
    return;
  }

  printf("DEBUG: Program has %zu modules\n",
         program_node->stmt.program.module_count);

  for (size_t i = 0; i < program_node->stmt.program.module_count; i++) {
    AstNode *module = program_node->stmt.program.modules[i];
    if (!module) {
      printf("DEBUG: Module %zu is NULL\n", i);
      continue;
    }

    printf("DEBUG: Module %zu:\n", i);
    printf("  - Type: %d\n", module->type);
    printf("  - Name: %s\n", module->preprocessor.module.name);
    printf("  - Body pointer: %p\n", (void *)module->preprocessor.module.body);
    printf("  - Potions (position): %d\n", module->preprocessor.module.potions);

    // Check what's actually in the body
    if (module->preprocessor.module.body) {
      printf("  - Body contents:\n");
      for (int j = 0; j < 10; j++) { // Check first 10 elements
        AstNode *body_item = module->preprocessor.module.body[j];
        printf("    body[%d]: %p", j, (void *)body_item);
        if (body_item) {
          printf(" (type: %d)", body_item->type);
        }
        printf("\n");
        if (!body_item)
          break; // Stop at first null
      }
    }
  }
}

bool run_build(BuildConfig config, ArenaAllocator *allocator) {
  bool success = false;

  GrowableArray modules;
  if (!growable_array_init(&modules, allocator, 16, sizeof(AstNode *))) {
    fprintf(stderr, "Failed to initialize modules list.\n");
    return false;
  }

  // Parse additional files (-l) and extract their modules
  for (size_t i = 0; i < config.file_count; i++) {
    char **files_array = (char **)config.files.data;
    Stmt *module = parse_file_to_module(files_array[i], i, allocator);
    if (!module || error_report()) {
      goto cleanup;
    }

    AstNode **slot = (AstNode **)growable_array_push(&modules);
    if (!slot) {
      fprintf(stderr, "Out of memory while growing modules array\n");
      goto cleanup;
    }
    *slot = (AstNode *)module;
  }

  // Parse the main file and extract its module
  Stmt *main_module =
      parse_file_to_module(config.filepath, config.file_count, allocator);
  if (!main_module || error_report()) {
    goto cleanup;
  }

  AstNode **main_slot = (AstNode **)growable_array_push(&modules);
  if (!main_slot) {
    fprintf(stderr, "Out of memory while growing modules array\n");
    goto cleanup;
  }
  *main_slot = (AstNode *)main_module;

  // Create a single Program node containing all modules
  AstNode *combined_program = create_program_node(
      allocator, (AstNode **)modules.data, modules.count, 0, 0);
  if (!combined_program) {
    fprintf(stderr, "Failed to create combined program node\n");
    goto cleanup;
  }

  // Print the combined AST for debugging
  // print_ast(combined_program, "", true, true);

  // Typechecking with global scope
  Scope root_scope;
  init_scope(&root_scope, NULL, "global", allocator);

  bool tc = typecheck(combined_program, &root_scope, allocator);
  // debug_ast_module_structure(combined_program);
  debug_print_scope(&root_scope, 0);

  if (tc) {
    success = generate_llvm_code(combined_program, config, allocator);
  } else {
    printf("Skipping LLVM code generation due to type checking errors.\n");
  }

  if (config.name)
    printf("Building target: %s\n", config.name);
  if (config.clean)
    printf("Cleaning build artifacts.\n");

cleanup:
  return success;
}
