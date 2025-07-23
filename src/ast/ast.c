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

void free_ast_node(AstNode *node) {
    if (!node) return;

    // Free child nodes based on type
    switch (node->type) {
        case AST_PROGRAM:
            for (size_t i = 0; i < node->stmt.program.stmt_count; ++i) {
                free_ast_node(node->stmt.program.statements[i]);
            }
            break;
        case AST_STMT_EXPRESSION:
            free_ast_node(node->stmt.expr_stmt.expression);
            break;
        case AST_STMT_VAR_DECL:
            free_ast_node(node->stmt.var_decl.var_type);
            free_ast_node(node->stmt.var_decl.initializer);
            break;
        case AST_STMT_FUNCTION:
            for (size_t i = 0; i < node->stmt.func_decl.param_count; ++i) {
                free_ast_node(node->stmt.func_decl.param_types[i]);
            }
            free_ast_node(node->stmt.func_decl.return_type);
            free_ast_node(node->stmt.func_decl.body);
            break;
        case AST_STMT_IF:
            free_ast_node(node->stmt.if_stmt.condition);
            free_ast_node(node->stmt.if_stmt.then_stmt);
            free_ast_node(node->stmt.if_stmt.else_stmt);
            break;
        case AST_STMT_LOOP:
            free_ast_node(node->stmt.loop_stmt.condition);
            free_ast_node(node->stmt.loop_stmt.body);
            free_ast_node(node->stmt.loop_stmt.initializer);
            free_ast_node(node->stmt.loop_stmt.increment);
            break;
        case AST_STMT_RETURN:
            free_ast_node(node->stmt.return_stmt.value);
            break;
        case AST_STMT_BLOCK:
            for (size_t i = 0; i < node->stmt.block.stmt_count; ++i) {
                free_ast_node(node->stmt.block.statements[i]);
            }
            break;
        default:
            // Handle other node types as needed
            break;
    }
}