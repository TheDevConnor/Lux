#include <stdalign.h>
#include <stdio.h>

#include "../c_libs/memory/memory.h"
#include "../helper/help.h"
#include "../ast/ast.h"
#include "parser.h"


Stmt *parse(GrowableArray *tks, ArenaAllocator *arena) {
  Parser parser = {
      .tks = (Token *)tks->data, // Cast to Token pointer
      .stmts =
          (Stmt *)arena_alloc(arena, sizeof(Stmt) * tks->count,
                              alignof(100)), // Allocate memory for statements
      .tk_count = tks->count,
      .stmt_count = 0,
      .capacity = tks->capacity,
      .pos = 0,
  };

  if (!parser.tks || !parser.stmts) {
    return NULL; // Memory allocation failed
  }

  printf("Parsing statements...\n");
  for (size_t i = 0; i < parser.tk_count; i++)
    print_token(parser.tks + i);

  return NULL;
}

BindingPower get_bp(TokenType kind) {
  switch (kind) {
  // Assignment
  case TOK_EQUAL:
    return BP_ASSIGN;

  // Ternary
  case TOK_QUESTION:
    return BP_TERNARY;

  // Logical
  case TOK_OR:
    return BP_LOGICAL_OR;
  case TOK_AND:
    return BP_LOGICAL_AND;

  // Bitwise
  case TOK_PIPE:
    return BP_BITWISE_OR;
  case TOK_CARET:
    return BP_BITWISE_XOR;
  case TOK_AMP:
    return BP_BITWISE_AND;

  // Equality
  case TOK_EQEQ:
  case TOK_NEQ:
    return BP_EQUALITY;

  // Relational
  case TOK_LT:
  case TOK_LE:
  case TOK_GT:
  case TOK_GE:
    return BP_RELATIONAL;

  // Arithmetic
  case TOK_PLUS:
  case TOK_MINUS:
    return BP_SUM;
  case TOK_STAR:
  case TOK_SLASH:
    return BP_PRODUCT;

  // Postfix
  case TOK_PLUSPLUS:
  case TOK_MINUSMINUS:
    return BP_POSTFIX;

  // Call/indexing/member access
  case TOK_LPAREN:
  case TOK_LBRACKET:
  case TOK_DOT:
    return BP_CALL;

  default:
    return BP_NONE;
  }
}

Expr *nud(Parser *parser) {
  switch (p_current(parser).type_) {
  case TOK_NUMBER:
  case TOK_STRING:
  case TOK_IDENTIFIER:
    return primary(parser);
  case TOK_MINUS:
  case TOK_PLUS:
  case TOK_BANG:
    return unary(parser);
  case TOK_LPAREN:
    return grouping(parser);
  default:
    p_advance(parser);
    return NULL;
  }
}

Expr *led(Parser *parser, Expr *left, BindingPower bp) {
  switch (p_current(parser).type_) {
  case TOK_PLUS:
  case TOK_MINUS:
  case TOK_STAR:
  case TOK_SLASH:
  case TOK_EQEQ:
  case TOK_NEQ:
  case TOK_LT:
  case TOK_LE:
  case TOK_GT:
  case TOK_GE:
  case TOK_AMP:
  case TOK_PIPE:
  case TOK_CARET:
  case TOK_AND:
  case TOK_OR:
    return binary(parser, left, bp);
  case TOK_LPAREN:
    return call_expr(parser, left, bp);
  case TOK_LBRACKET:
    return assign_expr(parser, left, bp);
  case TOK_DOT:
    return prefix_expr(parser, left, bp);
  default:
    p_advance(parser);
    return left; // No valid LED found, return left expression
  }
}

Type *parse_type(Parser *parser) {
  switch (p_current(parser).type_) {
  case TOK_INT:
  case TOK_UINT:
  case TOK_FLOAT:
  case TOK_BOOL:
  case TOK_STRING:
  case TOK_VOID:
  case TOK_CHAR_LITERAL:
    return tnud(parser);
  case TOK_IDENTIFIER:
    return tled(parser, NULL, BP_NONE);
  default:
    p_advance(parser);
    return NULL; // No valid type found
  }
}
