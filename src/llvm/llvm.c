#include "llvm.h"
#include <llvm-c/TargetMachine.h>

// Initialize code generation context with target machine support
CodeGenContext *init_codegen_context(ArenaAllocator *arena,
                                     const char *module_name) {
  CodeGenContext *ctx = (CodeGenContext *)arena_alloc(
      arena, sizeof(CodeGenContext), alignof(CodeGenContext));

  // Initialize LLVM targets
  LLVMInitializeAllTargetInfos();
  LLVMInitializeAllTargets();
  LLVMInitializeAllTargetMCs();
  LLVMInitializeAllAsmParsers();
  LLVMInitializeAllAsmPrinters();

  ctx->context = LLVMContextCreate();
  ctx->module = LLVMModuleCreateWithNameInContext(module_name, ctx->context);
  ctx->builder = LLVMCreateBuilderInContext(ctx->context);
  ctx->symbols = NULL;
  ctx->current_function = NULL;
  ctx->loop_continue_block = NULL;
  ctx->loop_break_block = NULL;
  ctx->arena = arena;

  return ctx;
}

// Generate object file from LLVM IR
bool generate_object_file(CodeGenContext *ctx, const char *object_filename) {
  char *error = NULL;

  // Get the target triple for the current machine
  char *target_triple = LLVMGetDefaultTargetTriple();
  LLVMSetTarget(ctx->module, target_triple);

  // Get the target from the triple
  LLVMTargetRef target;
  if (LLVMGetTargetFromTriple(target_triple, &target, &error)) {
    fprintf(stderr, "Failed to get target: %s\n", error);
    LLVMDisposeMessage(error);
    LLVMDisposeMessage(target_triple);
    return false;
  }

  // Create target machine
  LLVMTargetMachineRef target_machine = LLVMCreateTargetMachine(
      target, target_triple,
      "generic", // CPU
      "",        // Features
      LLVMCodeGenLevelDefault, LLVMRelocDefault, LLVMCodeModelDefault);

  if (!target_machine) {
    fprintf(stderr, "Failed to create target machine\n");
    LLVMDisposeMessage(target_triple);
    return false;
  }

  // Set the data layout and target triple on the module
  LLVMTargetDataRef target_data = LLVMCreateTargetDataLayout(target_machine);
  char *data_layout = LLVMCopyStringRepOfTargetData(target_data);
  LLVMSetDataLayout(ctx->module, data_layout);

  // Generate object file
  if (LLVMTargetMachineEmitToFile(target_machine, ctx->module,
                                  (char *)object_filename, LLVMObjectFile,
                                  &error)) {
    fprintf(stderr, "Failed to emit object file: %s\n", error);
    LLVMDisposeMessage(error);
    LLVMDisposeTargetMachine(target_machine);
    LLVMDisposeTargetData(target_data);
    LLVMDisposeMessage(data_layout);
    LLVMDisposeMessage(target_triple);
    return false;
  }

  // Cleanup
  LLVMDisposeTargetMachine(target_machine);
  LLVMDisposeTargetData(target_data);
  LLVMDisposeMessage(data_layout);
  LLVMDisposeMessage(target_triple);

  return true;
}

// Generate assembly file from LLVM IR (optional, for debugging)
bool generate_assembly_file(CodeGenContext *ctx, const char *asm_filename) {
  char *error = NULL;

  // Get the target triple for the current machine
  char *target_triple = LLVMGetDefaultTargetTriple();
  LLVMSetTarget(ctx->module, target_triple);

  // Get the target from the triple
  LLVMTargetRef target;
  if (LLVMGetTargetFromTriple(target_triple, &target, &error)) {
    fprintf(stderr, "Failed to get target: %s\n", error);
    LLVMDisposeMessage(error);
    LLVMDisposeMessage(target_triple);
    return false;
  }

  // Create target machine
  LLVMTargetMachineRef target_machine = LLVMCreateTargetMachine(
      target, target_triple, "generic", "", LLVMCodeGenLevelDefault,
      LLVMRelocDefault, LLVMCodeModelDefault);

  if (!target_machine) {
    fprintf(stderr, "Failed to create target machine\n");
    LLVMDisposeMessage(target_triple);
    return false;
  }

  // Set the data layout on the module
  LLVMTargetDataRef target_data = LLVMCreateTargetDataLayout(target_machine);
  char *data_layout = LLVMCopyStringRepOfTargetData(target_data);
  LLVMSetDataLayout(ctx->module, data_layout);

  // Generate assembly file
  if (LLVMTargetMachineEmitToFile(target_machine, ctx->module,
                                  (char *)asm_filename, LLVMAssemblyFile,
                                  &error)) {
    fprintf(stderr, "Failed to emit assembly file: %s\n", error);
    LLVMDisposeMessage(error);
    LLVMDisposeTargetMachine(target_machine);
    LLVMDisposeTargetData(target_data);
    LLVMDisposeMessage(data_layout);
    LLVMDisposeMessage(target_triple);
    return false;
  }

  // Cleanup
  LLVMDisposeTargetMachine(target_machine);
  LLVMDisposeTargetData(target_data);
  LLVMDisposeMessage(data_layout);
  LLVMDisposeMessage(target_triple);

  return true;
}

// Symbol table operations (unchanged)
void add_symbol(CodeGenContext *ctx, const char *name, LLVMValueRef value,
                LLVMTypeRef type, bool is_function) {
  LLVM_Symbol *sym = (LLVM_Symbol *)arena_alloc(ctx->arena, sizeof(LLVM_Symbol),
                                                alignof(LLVM_Symbol));
  sym->name = arena_strdup(ctx->arena, name);
  sym->value = value;
  sym->type = type;
  sym->is_function = is_function;
  sym->next = ctx->symbols;
  ctx->symbols = sym;
}

LLVM_Symbol *find_symbol(CodeGenContext *ctx, const char *name) {
  for (LLVM_Symbol *sym = ctx->symbols; sym; sym = sym->next) {
    if (strcmp(sym->name, name) == 0) {
      return sym;
    }
  }
  return NULL;
}

char *print_llvm_ir(CodeGenContext *ctx) {
  return LLVMPrintModuleToString(ctx->module);
}

// Cleanup (unchanged)
void cleanup_codegen_context(CodeGenContext *ctx) {
  if (ctx) {
    LLVMDisposeBuilder(ctx->builder);
    LLVMDisposeModule(ctx->module);
    LLVMContextDispose(ctx->context);
  }
}

// Enhanced IR generation with optional object file output
bool generate_llvm_ir(CodeGenContext *ctx, AstNode *ast_root,
                      const char *output_file) {
  if (!ast_root)
    return false;

  // Generate code for the entire program
  codegen_stmt(ctx, ast_root);

  // Verify the module
  char *error = NULL;
  if (LLVMVerifyModule(ctx->module, LLVMAbortProcessAction, &error)) {
    fprintf(stderr, "Module verification failed: %s\n", error);
    LLVMDisposeMessage(error);
    return false;
  }

  // Write bitcode file if specified
  if (output_file) {
    if (LLVMWriteBitcodeToFile(ctx->module, output_file) != 0) {
      fprintf(stderr, "Failed to write bitcode to %s\n", output_file);
      return false;
    }
  }

  return true;
}
