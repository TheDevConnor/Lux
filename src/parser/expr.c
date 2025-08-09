#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

Expr *primary(Parser *parser) {
  int line = p_current(parser).line;
  int col = p_current(parser).col;

  Token current = p_current(parser);
  LiteralType lit_type;
  switch (current.type_) {
  case TOK_NUMBER:
    lit_type = LITERAL_INT;
    break;
  case TOK_STRING:
    lit_type = LITERAL_STRING;
    break;
  case TOK_CHAR_LITERAL:
    lit_type = LITERAL_CHAR;
    break;
  case TOK_TRUE:
    lit_type = LITERAL_BOOL;
    break;
  case TOK_FALSE:
    lit_type = LITERAL_BOOL;
    break;
  case TOK_IDENTIFIER:
    lit_type = LITERAL_IDENT;
    break;
  default:
    lit_type = LITERAL_NULL;
    break;
  }

  if (lit_type != LITERAL_NULL) {
    void *value = NULL;
    switch (lit_type) {
    case LITERAL_INT:
      value = arena_alloc(parser->arena, sizeof(long long), alignof(long long));
      *(long long *)value = strtoll(current.value, NULL, 10);
      break;
    case LITERAL_FLOAT:
      value = arena_alloc(parser->arena, sizeof(double), alignof(double));
      *(double *)value = strtod(current.value, NULL);
      break;
    case LITERAL_STRING:
      value = arena_alloc(parser->arena, strlen(get_name(parser)) + 1,
                          alignof(char));
      strcpy((char *)value, get_name(parser));
      break;
    case LITERAL_CHAR:
      value = arena_alloc(parser->arena, sizeof(char), alignof(char));
      *(char *)value = current.value[0]; // Assume single character
      break;
    case LITERAL_BOOL:
      value = arena_alloc(parser->arena, sizeof(bool), alignof(bool));
      *(bool *)value = (strcmp(current.value, "true") == 0);
      break;
    case LITERAL_IDENT:
      value = get_name(parser); // Get the identifier name
      break;
    default:
      value = NULL; // Handle null or unsupported literal types
      break;
    }
    p_advance(parser); // Consume the token

    if (lit_type == LITERAL_IDENT) {
      return create_identifier_expr(parser->arena, (char *)value, line, col);
    }
    return create_literal_expr(parser->arena, lit_type, value, line, col);
  }

  return NULL; // Handle error or unsupported literal type
}

Expr *unary(Parser *parser) {
  int line = p_current(parser).line;
  int col = p_current(parser).col;

  Token current = p_current(parser);
  UnaryOp op;

  switch (current.type_) {
  case TOK_MINUS:
    op = UNOP_NEG;
    break;
  case TOK_PLUS:
    op = UNOP_POS;
    break;
  case TOK_BANG:
    op = UNOP_NOT;
    break;
  case TOK_PLUSPLUS:
    op = UNOP_PRE_INC;
    break;
  case TOK_MINUSMINUS:
    op = UNOP_PRE_DEC;
    break;
  default:
    op = 0;
    break; // or handle error
  }

  if (op) {
    p_advance(parser); // Consume the token
    Expr *operand = parse_expr(parser, BP_UNARY);
    return create_unary_expr(parser->arena, op, operand, line, col);
  }

  return NULL;
}

Expr *grouping(Parser *parser) {
  int line = p_current(parser).line;
  int col = p_current(parser).col;

  p_consume(parser, TOK_LPAREN, "Expected '(' for grouping");
  Expr *expr = parse_expr(parser, BP_LOWEST);
  p_consume(parser, TOK_RPAREN, "Expected ')' to close grouping");
  return create_grouping_expr(parser->arena, expr, line, col);
}

Expr *binary(Parser *parser, Expr *left, BindingPower bp) {
  int line = p_current(parser).line;
  int col = p_current(parser).col;

  Token current = p_current(parser);
  BinaryOp op;

  switch (current.type_) {
  case TOK_PLUS:
    op = BINOP_ADD;
    break;
  case TOK_MINUS:
    op = BINOP_SUB;
    break;
  case TOK_STAR:
    op = BINOP_MUL;
    break;
  case TOK_SLASH:
    op = BINOP_DIV;
    break;
  case TOK_EQEQ:
    op = BINOP_EQ;
    break;
  case TOK_NEQ:
    op = BINOP_NE;
    break;
  case TOK_LT:
    op = BINOP_LT;
    break;
  case TOK_LE:
    op = BINOP_LE;
    break;
  case TOK_GT:
    op = BINOP_GT;
    break;
  case TOK_GE:
    op = BINOP_GE;
    break;
  case TOK_AND:
    op = BINOP_AND;
    break;
  case TOK_OR:
    op = BINOP_OR;
    break;
  default:
    return left; // No valid binary operator found
  }

  p_advance(parser); // Consume the token
  Expr *right = parse_expr(parser, bp);

  return create_binary_expr(parser->arena, op, left, right, line, col);
}

Expr *call_expr(Parser *parser, Expr *left, BindingPower bp) { return NULL; }

Expr *assign_expr(Parser *parser, Expr *left, BindingPower bp) { return NULL; }

Expr *prefix_expr(Parser *parser, Expr *left, BindingPower bp) { return NULL; }

Expr *array_expr(Parser *parser) {
  int line = p_current(parser).line;
  int col = p_current(parser).col;

  GrowableArray elements;
  if (!growable_array_init(&elements, parser->arena, 4, sizeof(Expr *))) {
    fprintf(stderr, "Failed to initialize array elements\n");
    return NULL;
  }

  p_consume(parser, TOK_LBRACKET, "Expected '[' for array expression");
  while (p_current(parser).type_ != TOK_RBRACKET) {
    Expr *element = parse_expr(parser, BP_LOWEST);
    if (!element) {
      fprintf(stderr, "Expected expression inside array\n");
      return NULL;
    }

    Expr **slot = (Expr **)growable_array_push(&elements);
    if (!slot) {
      fprintf(stderr, "Out of memory while growing array elements\n");
      return NULL;
    }

    *slot = element;

    if (p_current(parser).type_ == TOK_COMMA) {
      p_advance(parser); // Consume the comma
    }
  }
  p_consume(parser, TOK_RBRACKET, "Expected ']' to close array expression");

  return create_array_expr(parser->arena, (Expr **)elements.data,
                           elements.count, line, col);
}
