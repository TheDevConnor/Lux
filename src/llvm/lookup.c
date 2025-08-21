#include "llvm.h"

LLVMValueRef codegen_expr(CodeGenContext *ctx, AstNode *node) {
  if (!node || node->category != Node_Category_EXPR) {
    return NULL;
  }

  switch (node->type) {
  case AST_EXPR_LITERAL:
    return codegen_expr_literal(ctx, node);
  case AST_EXPR_IDENTIFIER:
    return codegen_expr_identifier(ctx, node);
  case AST_EXPR_BINARY:
    return codegen_expr_binary(ctx, node);
  case AST_EXPR_UNARY:
    return codegen_expr_unary(ctx, node);
  case AST_EXPR_CALL:
    return codegen_expr_call(ctx, node);
  case AST_EXPR_ASSIGNMENT:
    return codegen_expr_assignment(ctx, node);
  default:
    return NULL;
  }
}

// Generate code for statements
LLVMValueRef codegen_stmt(CodeGenContext *ctx, AstNode *node) {
  if (!node || node->category != Node_Category_STMT) {
    return NULL;
  }

  switch (node->type) {
  case AST_PROGRAM:
    return codegen_stmt_program(ctx, node);
  case AST_STMT_EXPRESSION:
    return codegen_stmt_expression(ctx, node);
  case AST_STMT_VAR_DECL:
    return codegen_stmt_var_decl(ctx, node);
  case AST_STMT_FUNCTION:
    return codegen_stmt_function(ctx, node);
  case AST_STMT_RETURN:
    return codegen_stmt_return(ctx, node);
  case AST_STMT_BLOCK:
    return codegen_stmt_block(ctx, node);
  case AST_STMT_IF:
    return codegen_stmt_if(ctx, node);
  default:
    return NULL;
  }
}

// Generate code for types
LLVMTypeRef codegen_type(CodeGenContext *ctx, AstNode *node) {
  if (!node || node->category != Node_Category_TYPE) {
    return NULL;
  }

  switch (node->type) {
  case AST_TYPE_BASIC:
    return codegen_type_basic(ctx, node);
  case AST_TYPE_POINTER:
    return codegen_type_pointer(ctx, node);
  case AST_TYPE_ARRAY:
    return codegen_type_array(ctx, node);
  case AST_TYPE_FUNCTION:
    return codegen_type_function(ctx, node);
  default:
    return NULL;
  }
}
