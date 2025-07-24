#include "../ast.h"
#include <stdio.h>
#include <string.h>

AstNode *create_program_node(ArenaAllocator *arena, AstNode **statements, size_t stmt_count, size_t line, size_t column) {
  AstNode *node = create_stmt_node(arena, AST_PROGRAM, line, column);
  node->stmt.program.statements = statements;
  node->stmt.program.stmt_count = stmt_count;
  return node;
}

AstNode *create_expr_stmt(ArenaAllocator *arena, Expr *expression, size_t line, size_t column) {
  AstNode *node = create_stmt_node(arena, AST_STMT_EXPRESSION, line, column);
  node->stmt.expr_stmt.expression = expression;
  return node;
}

AstNode *create_var_decl_stmt(ArenaAllocator *arena, const char *name, AstNode *var_type, Expr *initializer, bool is_mutable, size_t line, size_t column) {
  AstNode *node = create_stmt_node(arena, AST_STMT_VAR_DECL, line, column);
  node->stmt.var_decl.name = name;
  node->stmt.var_decl.var_type = var_type;
  node->stmt.var_decl.initializer = initializer;
  node->stmt.var_decl.is_mutable = is_mutable;
  return node;
}

AstNode *create_func_decl_stmt(ArenaAllocator *arena, const char *name, char **param_names, AstNode **param_types, size_t param_count, AstNode *return_type, AstNode *body, size_t line, size_t column) {
  AstNode *node = create_stmt_node(arena, AST_STMT_FUNCTION, line, column);
  node->stmt.func_decl.name = name;
  node->stmt.func_decl.param_names = param_names;
  node->stmt.func_decl.param_types = param_types;
  node->stmt.func_decl.param_count = param_count;
  node->stmt.func_decl.return_type = return_type;
  node->stmt.func_decl.body = body;
  return node;
}

AstNode *create_if_stmt(ArenaAllocator *arena, Expr *condition, AstNode *then_stmt, AstNode *else_stmt, size_t line, size_t column) {
  AstNode *node = create_stmt_node(arena, AST_STMT_IF, line, column);
  node->stmt.if_stmt.condition = condition;
  node->stmt.if_stmt.then_stmt = then_stmt;
  node->stmt.if_stmt.else_stmt = else_stmt;
  return node;
}

AstNode *create_loop_stmt(ArenaAllocator *arena, Expr *condition, AstNode *body, AstNode *initializer, AstNode *increment, size_t line, size_t column) {
  AstNode *node = create_stmt_node(arena, AST_STMT_LOOP, line, column);
  node->stmt.loop_stmt.condition = condition;
  node->stmt.loop_stmt.body = body;
  node->stmt.loop_stmt.initializer = initializer;
  node->stmt.loop_stmt.increment = increment;
  return node;
}

AstNode *create_return_stmt(ArenaAllocator *arena, Expr *value, size_t line, size_t column) {
  AstNode *node = create_stmt_node(arena, AST_STMT_RETURN, line, column);
  node->stmt.return_stmt.value = value;
  return node;
}

AstNode *create_block_stmt(ArenaAllocator *arena, AstNode **statements, size_t stmt_count, size_t line, size_t column) {
  AstNode *node = create_stmt_node(arena, AST_STMT_BLOCK, line, column);
  node->stmt.block.statements = statements;
  node->stmt.block.stmt_count = stmt_count;
  return node;
}

AstNode *create_print_stmt(ArenaAllocator *arena, Expr **expressions, size_t expr_count, size_t line, size_t column) {
  AstNode *node = create_stmt_node(arena, AST_STMT_PRINT, line, column);
  node->stmt.print_stmt.expressions = expressions;
  node->stmt.print_stmt.expr_count = expr_count;
  return node;
}