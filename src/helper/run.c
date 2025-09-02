// #include "../ast/ast_utils.h"
#include "../c_libs/error/error.h"
#include "../llvm/llvm.h"
#include "../parser/parser.h"
#include "../typechecker/type.h"
#include "help.h"

#include <stdbool.h>

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
    }
    LLVMDisposeMessage(ir);
  }

  // Generate object file
  snprintf(filename, sizeof(filename), "%s.o", base_name);
  generate_object_file(ctx, filename);

  // Generate assembly file
  snprintf(filename, sizeof(filename), "%s.s", base_name);
  generate_assembly_file(ctx, filename);

  // Link executable
  snprintf(filename, sizeof(filename), "%s.o", base_name);
  char exe_name[256];
  snprintf(exe_name, sizeof(exe_name), "%s", base_name);
  link_with_ld_simple(filename, exe_name);
}

bool generate_llvm_code(AstNode *root, BuildConfig config,
                        ArenaAllocator *allocator, int *step) {
  CodeGenContext *ctx = init_codegen_context(allocator, "main_module");
  if (!ctx) {
    return false;
  }

  const char *base_name = config.name ? config.name : "output";
  char bc_file[256];

  if (config.save) {
    snprintf(bc_file, sizeof(bc_file), "%s.bc", base_name);
  }

  bool success = generate_llvm_ir(ctx, root, config.save ? bc_file : NULL);
  if (!success) {
    cleanup_codegen_context(ctx);
    return false;
  }

  print_progress(++(*step), 7, "LLVM IR Generation");
  if (config.save) {
    save_output_files(ctx, base_name);
  } else {
    // Directly generate executable without saving intermediate files
    char obj_file[256];
    snprintf(obj_file, sizeof(obj_file), "%s.o", base_name);
    generate_object_file(ctx, obj_file);

    char exe_file[256];
    snprintf(exe_file, sizeof(exe_file), "%s", base_name);
    link_with_ld_simple(obj_file, exe_file);

    // delete the object file after linking
    remove(obj_file);
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

bool run_build(BuildConfig config, ArenaAllocator *allocator) {
  bool success = false;
  int total_stages = 8;
  int step = 0;

  GrowableArray modules;
  if (!growable_array_init(&modules, allocator, 16, sizeof(AstNode *))) {
    return false;
  }

  // Stage 1: Lexing
  print_progress(++step, total_stages, "Lexing");

  // Parse additional files
  for (size_t i = 0; i < config.file_count; i++) {
    char **files_array = (char **)config.files.data;
    Stmt *module = parse_file_to_module(files_array[i], i, allocator);
    if (!module || error_report())
      goto cleanup;

    AstNode **slot = (AstNode **)growable_array_push(&modules);
    if (!slot)
      goto cleanup;
    *slot = (AstNode *)module;
  }

  // Stage 2: Parsing
  print_progress(++step, total_stages, "Parsing");

  Stmt *main_module =
      parse_file_to_module(config.filepath, config.file_count, allocator);
  if (!main_module || error_report())
    goto cleanup;

  AstNode **main_slot = (AstNode **)growable_array_push(&modules);
  if (!main_slot)
    goto cleanup;
  *main_slot = (AstNode *)main_module;

  // Stage 3: Combining modules
  print_progress(++step, total_stages, "Module Combination");

  AstNode *combined_program = create_program_node(
      allocator, (AstNode **)modules.data, modules.count, 0, 0);
  if (!combined_program)
    goto cleanup;

  // print_ast(combined_program, false, false, 0);

  // Stage 4: Typechecking
  print_progress(++step, total_stages, "Typechecker");

  Scope root_scope;
  init_scope(&root_scope, NULL, "global", allocator);
  bool tc = typecheck(combined_program, &root_scope, allocator);
  // debug_print_scope(&root_scope, 0);

  if (tc) {
    // Stage 5: LLVM IR
    print_progress(++step, total_stages, "LLVM IR");

    success = generate_llvm_code(combined_program, config, allocator, &step);
  }

  // Stage 6: Finalizing
  print_progress(++step, total_stages, "Finalizing");
  print_progress(++step, total_stages, "Completed");
  printf("Build succeeded! Written to '%s'\n",
         config.name ? config.name : "output");

cleanup:
  return success;
}
