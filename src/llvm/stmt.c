#include "llvm.h"

LLVMValueRef codegen_stmt_program(CodeGenContext *ctx, AstNode *node) {
  for (size_t i = 0; i < node->stmt.program.stmt_count; i++) {
    codegen_stmt(ctx, node->stmt.program.statements[i]);
  }
  return NULL;
}

LLVMValueRef codegen_stmt_expression(CodeGenContext *ctx, AstNode *node) {
  return codegen_expr(ctx, node->stmt.expr_stmt.expression);
}

LLVMValueRef codegen_stmt_var_decl(CodeGenContext *ctx, AstNode *node) {
  LLVMTypeRef var_type = codegen_type(ctx, node->stmt.var_decl.var_type);
  if (!var_type)
    return NULL;

  // Create alloca for the variable
  LLVMValueRef alloca =
      LLVMBuildAlloca(ctx->builder, var_type, node->stmt.var_decl.name);

  // Add to symbol table
  add_symbol(ctx, node->stmt.var_decl.name, alloca, var_type, false);

  // Initialize if there's an initializer
  if (node->stmt.var_decl.initializer) {
    LLVMValueRef init_val = codegen_expr(ctx, node->stmt.var_decl.initializer);
    if (init_val) {
      LLVMBuildStore(ctx->builder, init_val, alloca);
    }
  }

  return alloca;
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

  LLVMValueRef function =
      LLVMAddFunction(ctx->module, node->stmt.func_decl.name, func_type);

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
  if (node->stmt.return_stmt.value) {
    LLVMValueRef ret_val = codegen_expr(ctx, node->stmt.return_stmt.value);
    if (ret_val) {
      return LLVMBuildRet(ctx->builder, ret_val);
    }
  } else {
    return LLVMBuildRetVoid(ctx->builder);
  }
  return NULL;
}

LLVMValueRef codegen_stmt_block(CodeGenContext *ctx, AstNode *node) {
  for (size_t i = 0; i < node->stmt.block.stmt_count; i++) {
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
  LLVMBuildBr(ctx->builder, merge_block);

  // Generate else block if it exists
  if (else_block) {
    LLVMPositionBuilderAtEnd(ctx->builder, else_block);
    codegen_stmt(ctx, node->stmt.if_stmt.else_stmt);
    LLVMBuildBr(ctx->builder, merge_block);
  }

  // Continue with merge block
  LLVMPositionBuilderAtEnd(ctx->builder, merge_block);
  return NULL;
}
