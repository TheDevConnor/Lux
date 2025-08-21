#include "llvm.h"

LLVMValueRef codegen_expr_literal(CodeGenContext *ctx, AstNode *node) {
  switch (node->expr.literal.lit_type) {
  case LITERAL_INT:
    return LLVMConstInt(LLVMInt32TypeInContext(ctx->context),
                        node->expr.literal.value.int_val, false);
  case LITERAL_FLOAT:
    return LLVMConstReal(LLVMFloatTypeInContext(ctx->context),
                         node->expr.literal.value.float_val);
  case LITERAL_BOOL:
    return LLVMConstInt(LLVMInt1TypeInContext(ctx->context),
                        node->expr.literal.value.bool_val ? 1 : 0, false);
  case LITERAL_STRING:
    return LLVMBuildGlobalStringPtr(ctx->builder,
                                    node->expr.literal.value.string_val, "str");
  case LITERAL_NULL:
    return LLVMConstNull(
        LLVMPointerType(LLVMInt8TypeInContext(ctx->context), 0));
  default:
    return NULL;
  }
}

// Expression identifier handler
LLVMValueRef codegen_expr_identifier(CodeGenContext *ctx, AstNode *node) {
  LLVM_Symbol *sym = find_symbol(ctx, node->expr.identifier.name);
  if (sym) {
    if (sym->is_function) {
      return sym->value;
    } else {
      // Load variable value
      return LLVMBuildLoad2(ctx->builder, sym->type, sym->value, "load");
    }
  }
  return NULL;
}

// Expression binary operation handler
LLVMValueRef codegen_expr_binary(CodeGenContext *ctx, AstNode *node) {
  LLVMValueRef left = codegen_expr(ctx, node->expr.binary.left);
  LLVMValueRef right = codegen_expr(ctx, node->expr.binary.right);

  if (!left || !right)
    return NULL;

  switch (node->expr.binary.op) {
  case BINOP_ADD:
    return LLVMBuildAdd(ctx->builder, left, right, "add");
  case BINOP_SUB:
    return LLVMBuildSub(ctx->builder, left, right, "sub");
  case BINOP_MUL:
    return LLVMBuildMul(ctx->builder, left, right, "mul");
  case BINOP_DIV:
    return LLVMBuildSDiv(ctx->builder, left, right, "div");
  case BINOP_MOD:
    return LLVMBuildSRem(ctx->builder, left, right, "mod");
  case BINOP_EQ:
    return LLVMBuildICmp(ctx->builder, LLVMIntEQ, left, right, "eq");
  case BINOP_NE:
    return LLVMBuildICmp(ctx->builder, LLVMIntNE, left, right, "ne");
  case BINOP_LT:
    return LLVMBuildICmp(ctx->builder, LLVMIntSLT, left, right, "lt");
  case BINOP_LE:
    return LLVMBuildICmp(ctx->builder, LLVMIntSLE, left, right, "le");
  case BINOP_GT:
    return LLVMBuildICmp(ctx->builder, LLVMIntSGT, left, right, "gt");
  case BINOP_GE:
    return LLVMBuildICmp(ctx->builder, LLVMIntSGE, left, right, "ge");
  case BINOP_AND:
    return LLVMBuildAnd(ctx->builder, left, right, "and");
  case BINOP_OR:
    return LLVMBuildOr(ctx->builder, left, right, "or");
  default:
    return NULL;
  }
}

// Expression unary operation handler
LLVMValueRef codegen_expr_unary(CodeGenContext *ctx, AstNode *node) {
  LLVMValueRef operand = codegen_expr(ctx, node->expr.unary.operand);
  if (!operand)
    return NULL;

  switch (node->expr.unary.op) {
  case UNOP_NEG:
    return LLVMBuildNeg(ctx->builder, operand, "neg");
  case UNOP_NOT:
    return LLVMBuildNot(ctx->builder, operand, "not");
  default:
    return NULL;
  }
}

// Expression function call handler
LLVMValueRef codegen_expr_call(CodeGenContext *ctx, AstNode *node) {
  LLVMValueRef callee = codegen_expr(ctx, node->expr.call.callee);
  if (!callee)
    return NULL;

  LLVMValueRef *args = (LLVMValueRef *)arena_alloc(
      ctx->arena, sizeof(LLVMValueRef) * node->expr.call.arg_count,
      alignof(LLVMValueRef));

  for (size_t i = 0; i < node->expr.call.arg_count; i++) {
    args[i] = codegen_expr(ctx, node->expr.call.args[i]);
    if (!args[i])
      return NULL;
  }

  return LLVMBuildCall2(ctx->builder, LLVMGlobalGetValueType(callee), callee,
                        args, node->expr.call.arg_count, "call");
}

// Expression assignment handler
LLVMValueRef codegen_expr_assignment(CodeGenContext *ctx, AstNode *node) {
  LLVMValueRef value = codegen_expr(ctx, node->expr.assignment.value);
  if (!value)
    return NULL;

  // For simplicity, assume target is an identifier
  if (node->expr.assignment.target->type == AST_EXPR_IDENTIFIER) {
    LLVM_Symbol *sym =
        find_symbol(ctx, node->expr.assignment.target->expr.identifier.name);
    if (sym && !sym->is_function) {
      LLVMBuildStore(ctx->builder, value, sym->value);
      return value;
    }
  }
  return NULL;
}
