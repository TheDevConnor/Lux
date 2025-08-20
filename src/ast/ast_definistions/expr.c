#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../ast.h"

AstNode *create_literal_expr(ArenaAllocator *arena, LiteralType lit_type, void *value, size_t line, size_t column) {
    AstNode *node = create_expr(arena, AST_EXPR_LITERAL, line, column);
    node->expr.literal.lit_type = lit_type;

    if (value == NULL && lit_type != LITERAL_NULL) {
        fprintf(stderr, "Error: NULL value passed to create_literal_expr with non-null literal type.\n");
        exit(1);
    }

    switch (lit_type) {
    case LITERAL_INT:
      node->expr.literal.value.int_val = *(long long *)value;
      break;
    case LITERAL_FLOAT:
      node->expr.literal.value.float_val = *(double *)value;
      break;
    case LITERAL_STRING:
      node->expr.literal.value.string_val = arena_strdup(arena, (char *)value);
      break;
    case LITERAL_CHAR:
      node->expr.literal.value.char_val = *(char *)value;
      break;
    case LITERAL_BOOL:
      node->expr.literal.value.bool_val = *(bool *)value;
      break;
    case LITERAL_NULL:
      break;
    case LITERAL_IDENT:
      node->expr.literal.value.string_val = arena_strdup(arena, (char *)value);
      break;
    default:
      fprintf(stderr, "Error: Unsupported literal type %d in create_literal_expr.\n", lit_type);
      exit(1);
      break;
    }

    return node;
}

AstNode *create_identifier_expr(ArenaAllocator *arena, const char *name, size_t line, size_t column) {
    AstNode *node = create_expr(arena, AST_EXPR_IDENTIFIER, line, column);
    node->expr.identifier.name = arena_strdup(arena, name);
    return node;
}

AstNode *create_binary_expr(ArenaAllocator *arena, BinaryOp op, AstNode *left, AstNode *right, size_t line, size_t column) {
    AstNode *node = create_expr(arena, AST_EXPR_BINARY, line, column);
    node->expr.binary.op = op;
    node->expr.binary.left = left;
    node->expr.binary.right = right;
    return node;
}

AstNode *create_unary_expr(ArenaAllocator *arena, UnaryOp op, AstNode *operand, size_t line, size_t column) {
    AstNode *node = create_expr(arena, AST_EXPR_UNARY, line, column);
    node->expr.unary.op = op;
    node->expr.unary.operand = operand;
    return node;
}

AstNode *create_call_expr(ArenaAllocator *arena, AstNode *callee, AstNode **args, size_t arg_count, size_t line, size_t column) {
    AstNode *node = create_expr(arena, AST_EXPR_CALL, line, column);
    node->expr.call.callee = callee;
    node->expr.call.args = args;
    node->expr.call.arg_count = arg_count;
    return node;
}

AstNode *create_assignment_expr(ArenaAllocator *arena, AstNode *target, AstNode *value, size_t line, size_t column) {
    AstNode *node = create_expr(arena, AST_EXPR_ASSIGNMENT, line, column);
    node->expr.assignment.target = target;
    node->expr.assignment.value = value;
    return node;
}

AstNode *create_ternary_expr(ArenaAllocator *arena, AstNode *condition, AstNode *then_expr, AstNode *else_expr, size_t line, size_t column) {
    AstNode *node = create_expr(arena, AST_EXPR_TERNARY, line, column);
    node->expr.ternary.condition = condition;
    node->expr.ternary.then_expr = then_expr;
    node->expr.ternary.else_expr = else_expr;
    return node;
}

AstNode *create_member_expr(ArenaAllocator *arena, AstNode *object, const char *member, size_t line, size_t column) {
    AstNode *node = create_expr(arena, AST_EXPR_MEMBER, line, column);
    node->expr.member.object = object;
    node->expr.member.member = strdup(member);
    return node;
}

AstNode *create_index_expr(ArenaAllocator *arena, AstNode *object, AstNode *index, size_t line, size_t column) {
    AstNode *node = create_expr(arena, AST_EXPR_INDEX, line, column);
    node->expr.index.object = object;
    node->expr.index.index = index;
    return node;
}

AstNode *create_grouping_expr(ArenaAllocator *arena, AstNode *expr, size_t line, size_t column) {
    AstNode *node = create_expr(arena, AST_EXPR_GROUPING, line, column);
    node->expr.grouping.expr = expr;
    return node;
}

AstNode *create_array_expr(ArenaAllocator *arena, AstNode **elements, size_t element_count, size_t line, size_t column) {
    AstNode *node = create_expr(arena, AST_EXPR_ARRAY, line, column);
    node->expr.array.elements = elements;
    node->expr.array.element_count = element_count;
    return node;
}

AstNode *create_deref_expr(ArenaAllocator *arena, Expr *object, size_t line, size_t col) {
  AstNode *node = create_expr(arena, AST_EXPR_DEREF, line, col);
  node->expr.deref.object = object;
  return node;
}

AstNode *create_addr_expr(ArenaAllocator *arena, Expr *object, size_t line, size_t col) {
  AstNode *node = create_expr(arena, AST_EXPR_ADDR, line, col);
  node->expr.addr.object = object;
  return node;
}
