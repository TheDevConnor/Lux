#include "ast.h"

AstNode *create_ast_node(ArenaAllocator *arena, NodeType type, NodeCategory category, size_t line, size_t column) {
  AstNode *node = arena_alloc(arena, sizeof(AstNode), alignof(AstNode));
  if (!node) return NULL;

  node->type = type;
  node->line = line;
  node->column = column;
  node->category = category;

  return node;
}

AstNode *create_expr_node(ArenaAllocator *arena, NodeType type, size_t line, size_t column) {
    AstNode *node = arena_alloc(arena, sizeof(AstNode), alignof(AstNode));
    if (!node) return NULL;
    node->type = type;
    node->line = line;
    node->column = column;
    node->category = Node_Category_EXPR;
    return node;
}

AstNode *create_stmt_node(ArenaAllocator *arena, NodeType type, size_t line, size_t column) {
    AstNode *node = arena_alloc(arena, sizeof(AstNode), alignof(AstNode));
    if (!node) return NULL;
    node->type = type;
    node->line = line;
    node->column = column;
    node->category = Node_Category_STMT;
    return node;
}

AstNode *create_type_node(ArenaAllocator *arena, NodeType type, size_t line, size_t column) {
    AstNode *node = arena_alloc(arena, sizeof(AstNode), alignof(AstNode));
    if (!node) return NULL;
    node->type = type;
    node->line = line;
    node->column = column;
    node->category = Node_Category_TYPE;
    return node;
}