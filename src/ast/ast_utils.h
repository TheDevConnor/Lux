#pragma once

#include "ast.h"
#include <stdio.h>

#define IS_EXPR(node) ((node)->type >= AST_EXPR_LITERAL && (node)->type <= AST_EXPR_GROUPING)
#define IS_STMT(node) ((node)->type >= AST_STMT_EXPRESSION && (node)->type <= AST_STMT_STRUCT)
#define IS_TYPE(node) ((node)->type >= AST_TYPE_BASIC && (node)->type <= AST_TYPE_ENUM)

#define IS_LITERAL(node) ((node)->type == AST_EXPR_LITERAL)
#define IS_BINARY(node)  ((node)->type == AST_EXPR_BINARY)
#define IS_UNARY(node)   ((node)->type == AST_EXPR_UNARY)
#define IS_CALL(node)    ((node)->type == AST_EXPR_CALL)
#define IS_VAR_DECL(node) ((node)->type == AST_STMT_VAR_DECL)

const char *node_type_to_string(NodeType type);
const char *binop_to_string(BinaryOp op);
const char *unop_to_string(UnaryOp op);
const char *literal_type_to_string(LiteralType type);

void print_ast(const AstNode *node, int indent);
