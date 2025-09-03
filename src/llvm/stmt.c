#include "llvm.h"
#include <llvm-c/Core.h>
#include <llvm-c/Types.h>
#include <stdlib.h>

// Legacy program handler (now redirects to multi-module handler)
LLVMValueRef codegen_stmt_program(CodeGenContext *ctx, AstNode *node) {
  return codegen_stmt_program_multi_module(ctx, node);
}

LLVMValueRef codegen_stmt_expression(CodeGenContext *ctx, AstNode *node) {
  return codegen_expr(ctx, node->stmt.expr_stmt.expression);
}

LLVMValueRef codegen_stmt_var_decl(CodeGenContext *ctx, AstNode *node) {
  LLVMTypeRef var_type = codegen_type(ctx, node->stmt.var_decl.var_type);
  if (!var_type)
    return NULL;

  LLVMValueRef var_ref;

  // Use current module instead of legacy ctx->module
  LLVMModuleRef current_llvm_module =
      ctx->current_module ? ctx->current_module->module : ctx->module;

  if (ctx->current_function == NULL) {
    var_ref =
        LLVMAddGlobal(current_llvm_module, var_type, node->stmt.var_decl.name);

    // Set linkage based on whether this is a public declaration
    if (node->stmt.var_decl.is_public) {
      LLVMSetLinkage(var_ref, LLVMExternalLinkage);
    } else {
      LLVMSetLinkage(var_ref, LLVMInternalLinkage);
    }
  } else {
    var_ref = LLVMBuildAlloca(ctx->builder, var_type, node->stmt.var_decl.name);
  }

  if (node->stmt.var_decl.initializer) {
    LLVMValueRef init_val = codegen_expr(ctx, node->stmt.var_decl.initializer);
    if (init_val && ctx->current_function == NULL) {
      LLVMSetInitializer(var_ref, init_val);
    } else if (!init_val && ctx->current_function == NULL) {
      LLVMSetInitializer(var_ref, LLVMConstNull(var_type));
    } else {
      LLVMBuildStore(ctx->builder, init_val, var_ref);
    }
  }

  add_symbol(ctx, node->stmt.var_decl.name, var_ref, var_type, false);
  return var_ref;
}

LLVMValueRef codegen_stmt_function(CodeGenContext *ctx, AstNode *node) {
  LLVMTypeRef *param_types = (LLVMTypeRef *)arena_alloc(
      ctx->arena, sizeof(LLVMTypeRef) * node->stmt.func_decl.param_count,
      alignof(LLVMTypeRef));

  for (size_t i = 0; i < node->stmt.func_decl.param_count; i++) {
    param_types[i] = codegen_type(ctx, node->stmt.func_decl.param_types[i]);
    if (!param_types[i])
      return NULL;
  }

  LLVMTypeRef return_type = codegen_type(ctx, node->stmt.func_decl.return_type);
  if (!return_type)
    return NULL;

  LLVMTypeRef func_type = LLVMFunctionType(
      return_type, param_types, node->stmt.func_decl.param_count, false);

  // Use current module instead of legacy ctx->module
  LLVMModuleRef current_llvm_module =
      ctx->current_module ? ctx->current_module->module : ctx->module;

  LLVMValueRef function = LLVMAddFunction(current_llvm_module,
                                          node->stmt.func_decl.name, func_type);

  LLVMSetLinkage(function, get_function_linkage(node));

  // Add function to symbol table
  add_symbol(ctx, node->stmt.func_decl.name, function, func_type, true);

  // Set parameter names
  for (size_t i = 0; i < node->stmt.func_decl.param_count; i++) {
    LLVMValueRef param = LLVMGetParam(function, i);
    LLVMSetValueName2(param, node->stmt.func_decl.param_names[i],
                      strlen(node->stmt.func_decl.param_names[i]));
  }

  // Generate function body
  LLVMBasicBlockRef entry_block =
      LLVMAppendBasicBlockInContext(ctx->context, function, "entry");
  LLVMPositionBuilderAtEnd(ctx->builder, entry_block);

  LLVMValueRef old_function = ctx->current_function;
  ctx->current_function = function;

  // Add parameters to symbol table as allocas
  for (size_t i = 0; i < node->stmt.func_decl.param_count; i++) {
    LLVMValueRef param = LLVMGetParam(function, i);
    LLVMValueRef alloca = LLVMBuildAlloca(ctx->builder, param_types[i],
                                          node->stmt.func_decl.param_names[i]);
    LLVMBuildStore(ctx->builder, param, alloca);
    add_symbol(ctx, node->stmt.func_decl.param_names[i], alloca, param_types[i],
               false);
  }

  // Generate body
  codegen_stmt(ctx, node->stmt.func_decl.body);

  ctx->current_function = old_function;

  return function;
}

LLVMValueRef codegen_stmt_return(CodeGenContext *ctx, AstNode *node) {
  LLVMValueRef ret_val = NULL;
  if (node->stmt.return_stmt.value) {
    ret_val = codegen_expr(ctx, node->stmt.return_stmt.value);
    if (!ret_val)
      return NULL;
  }
  return LLVMBuildRet(ctx->builder, ret_val);
}

LLVMValueRef codegen_stmt_block(CodeGenContext *ctx, AstNode *node) {
  for (size_t i = 0; i < node->stmt.block.stmt_count; i++) {
    // Stop processing if we hit a terminator
    if (LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(ctx->builder))) {
      break;
    }
    codegen_stmt(ctx, node->stmt.block.statements[i]);
  }
  return NULL;
}

LLVMValueRef codegen_stmt_if(CodeGenContext *ctx, AstNode *node) {
  LLVMValueRef condition = codegen_expr(ctx, node->stmt.if_stmt.condition);
  if (!condition)
    return NULL;

  LLVMBasicBlockRef then_block = LLVMAppendBasicBlockInContext(
      ctx->context, ctx->current_function, "then");
  LLVMBasicBlockRef else_block =
      node->stmt.if_stmt.else_stmt
          ? LLVMAppendBasicBlockInContext(ctx->context, ctx->current_function,
                                          "else")
          : NULL;
  LLVMBasicBlockRef merge_block = LLVMAppendBasicBlockInContext(
      ctx->context, ctx->current_function, "merge");

  LLVMBuildCondBr(ctx->builder, condition, then_block,
                  else_block ? else_block : merge_block);

  // Generate then block
  LLVMPositionBuilderAtEnd(ctx->builder, then_block);
  codegen_stmt(ctx, node->stmt.if_stmt.then_stmt);

  // Only add branch if block isn't already terminated
  if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(ctx->builder))) {
    LLVMBuildBr(ctx->builder, merge_block);
  }

  // Generate else block if it exists
  if (else_block) {
    LLVMPositionBuilderAtEnd(ctx->builder, else_block);
    codegen_stmt(ctx, node->stmt.if_stmt.else_stmt);

    // Only add branch if block isn't already terminated
    if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(ctx->builder))) {
      LLVMBuildBr(ctx->builder, merge_block);
    }
  }

  // Continue with merge block
  LLVMPositionBuilderAtEnd(ctx->builder, merge_block);
  return NULL;
}

LLVMValueRef codegen_stmt_print(CodeGenContext *ctx, AstNode *node) {
  // Use current module instead of legacy ctx->module
  LLVMModuleRef current_llvm_module =
      ctx->current_module ? ctx->current_module->module : ctx->module;

  // Declare printf once (check if it already exists first)
  LLVMValueRef printf_func =
      LLVMGetNamedFunction(current_llvm_module, "printf");
  LLVMTypeRef printf_type = NULL;

  if (!printf_func) {
    LLVMTypeRef printf_arg_types[] = {
        LLVMPointerType(LLVMInt8TypeInContext(ctx->context), 0)};
    printf_type = LLVMFunctionType(LLVMInt32TypeInContext(ctx->context),
                                   printf_arg_types, 1, true);
    printf_func = LLVMAddFunction(current_llvm_module, "printf", printf_type);
    add_symbol(ctx, "printf", printf_func, printf_type, true);
  } else {
    printf_type = LLVMGlobalGetValueType(printf_func);
  }

  for (size_t i = 0; i < node->stmt.print_stmt.expr_count; i++) {
    AstNode *expr = node->stmt.print_stmt.expressions[i];
    LLVMValueRef value = NULL;
    const char *format_str = NULL;

    if (expr->type == AST_EXPR_LITERAL &&
        expr->expr.literal.lit_type == LITERAL_STRING) {

      // Process escape sequences in the string literal
      char *processed_str =
          process_escape_sequences(expr->expr.literal.value.string_val);

      // Create the string value directly (not using %s format)
      value = LLVMBuildGlobalStringPtr(ctx->builder, processed_str, "str");
      format_str = "%s";

      free(processed_str);
    } else {
      // Handle non-string expressions as before
      value = codegen_expr(ctx, expr);
      if (!value)
        return NULL;

      LLVMTypeRef value_type = LLVMTypeOf(value);
      if (LLVMGetTypeKind(value_type) == LLVMIntegerTypeKind) {
        unsigned int bits = LLVMGetIntTypeWidth(value_type);
        if (bits == 8 || bits == 16 || bits == 32) {
          format_str = "%d";
        } else if (bits == 64) {
          format_str = "%lld";
        } else {
          format_str = "%d";
        }
      } else if (LLVMGetTypeKind(value_type) == LLVMDoubleTypeKind) {
        format_str = "%f";
      } else {
        format_str = "%p";
      }
    }

    // Create format string and call printf
    LLVMValueRef format_str_val =
        LLVMBuildGlobalStringPtr(ctx->builder, format_str, "fmt");
    LLVMValueRef args[] = {format_str_val, value};
    LLVMBuildCall2(ctx->builder, printf_type, printf_func, args, 2, "");
  }

  return LLVMConstNull(LLVMVoidTypeInContext(ctx->context));
}
