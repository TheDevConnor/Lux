#pragma once
// LLVM C API Headers
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h> // Added for object file generation

// Standard Library Headers
#include <stdalign.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// Project Headers
#include "../ast/ast.h"
#include "../c_libs/memory/memory.h"

typedef struct LLVM_Symbol LLVM_Symbol;
typedef struct CodeGenContext CodeGenContext;

// Symbol table entry for variables and functions
struct LLVM_Symbol {
  char *name;
  LLVMValueRef value;
  LLVMTypeRef type;
  bool is_function;
  struct LLVM_Symbol *next;
};

// Code generation context
struct CodeGenContext {
  // LLVM Core Components
  LLVMContextRef context;
  LLVMModuleRef module;
  LLVMBuilderRef builder;

  // Symbol Management
  LLVM_Symbol *symbols;

  // Code Generation State
  LLVMValueRef current_function;
  LLVMBasicBlockRef loop_continue_block;
  LLVMBasicBlockRef loop_break_block;

  // Memory Management
  ArenaAllocator *arena;
};

// =============================================================================
// CORE API FUNCTIONS
// =============================================================================

// Context Management
CodeGenContext *init_codegen_context(ArenaAllocator *arena,
                                     const char *module_name);
void cleanup_codegen_context(CodeGenContext *ctx);

// Symbol Table Operations
void add_symbol(CodeGenContext *ctx, const char *name, LLVMValueRef value,
                LLVMTypeRef type, bool is_function);
LLVM_Symbol *find_symbol(CodeGenContext *ctx, const char *name);

// Main Code Generation
bool generate_llvm_ir(CodeGenContext *ctx, AstNode *ast_root,
                      const char *output_file);
char *print_llvm_ir(CodeGenContext *ctx);

// Object File Generation
bool generate_object_file(CodeGenContext *ctx, const char *object_filename);
bool generate_assembly_file(CodeGenContext *ctx, const char *asm_filename);

// =============================================================================
// AST NODE HANDLERS - PUBLIC INTERFACE
// =============================================================================

// Primary Dispatch Functions
LLVMValueRef codegen_expr(CodeGenContext *ctx, AstNode *node);
LLVMValueRef codegen_stmt(CodeGenContext *ctx, AstNode *node);
LLVMTypeRef codegen_type(CodeGenContext *ctx, AstNode *node);

// =============================================================================
// AST NODE HANDLERS - EXPRESSION TYPES
// =============================================================================

LLVMValueRef codegen_expr_literal(CodeGenContext *ctx, AstNode *node);
LLVMValueRef codegen_expr_identifier(CodeGenContext *ctx, AstNode *node);
LLVMValueRef codegen_expr_binary(CodeGenContext *ctx, AstNode *node);
LLVMValueRef codegen_expr_unary(CodeGenContext *ctx, AstNode *node);
LLVMValueRef codegen_expr_call(CodeGenContext *ctx, AstNode *node);
LLVMValueRef codegen_expr_assignment(CodeGenContext *ctx, AstNode *node);

// =============================================================================
// AST NODE HANDLERS - STATEMENT TYPES
// =============================================================================

LLVMValueRef codegen_stmt_program(CodeGenContext *ctx, AstNode *node);
LLVMValueRef codegen_stmt_expression(CodeGenContext *ctx, AstNode *node);
LLVMValueRef codegen_stmt_var_decl(CodeGenContext *ctx, AstNode *node);
LLVMValueRef codegen_stmt_function(CodeGenContext *ctx, AstNode *node);
LLVMValueRef codegen_stmt_return(CodeGenContext *ctx, AstNode *node);
LLVMValueRef codegen_stmt_block(CodeGenContext *ctx, AstNode *node);
LLVMValueRef codegen_stmt_if(CodeGenContext *ctx, AstNode *node);

// =============================================================================
// AST NODE HANDLERS - TYPE SYSTEM
// =============================================================================

LLVMTypeRef codegen_type_basic(CodeGenContext *ctx, AstNode *node);
LLVMTypeRef codegen_type_pointer(CodeGenContext *ctx, AstNode *node);
LLVMTypeRef codegen_type_array(CodeGenContext *ctx, AstNode *node);
LLVMTypeRef codegen_type_function(CodeGenContext *ctx, AstNode *node);
