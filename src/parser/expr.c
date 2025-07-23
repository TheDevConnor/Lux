#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

Expr *primary(Parser *parser) {
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
    p_advance(parser); // Consume the token
    void *value = NULL;
    switch (lit_type) {
    case LITERAL_INT:
      value = malloc(sizeof(long long));
      *(long long *)value = strtoll(current.value, NULL, 10); // base 10
      break;
    case LITERAL_FLOAT:
      value = malloc(sizeof(double));
      *(double *)value = strtod(current.value, NULL);
      break;
    case LITERAL_STRING:
      value = malloc(strlen(current.value) + 1);
      strcpy((char *)value, current.value); // Duplicate the string
      break;
    case LITERAL_CHAR:
      value = malloc(sizeof(char));
      *(char *)value = current.value[0]; // Assume single character
      break;
    case LITERAL_BOOL:
      value = malloc(sizeof(bool));
      *(bool *)value = (strcmp(current.value, "true") == 0);
      break;
    case LITERAL_IDENT:
      value = malloc(strlen(current.value) + 1);
      strcpy((char *)value, current.value); // Duplicate the identifier
      break;
    default:
      value = NULL; // Handle null or unsupported literal types
      break;
    }

    return create_literal_expr(parser->arena, lit_type, value, current.line,
                               current.col);
  }

  return NULL; // Handle error or unsupported literal type
}

Expr *unary(Parser *parser) {
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
  default:
    op = 0;
    break; // or handle error
  }

  if (op) {
    p_advance(parser); // Consume the token
    Expr *operand = parse_expr(parser, BP_UNARY);
    return create_unary_expr(parser->arena, op, operand, current.line,
                             current.col);
  }

  return NULL;
}

Expr *grouping(Parser *parser) {
  p_consume(parser, TOK_LPAREN, "Expected '(' for grouping");
  Expr *expr = parse_expr(parser, BP_LOWEST);
  p_consume(parser, TOK_RPAREN, "Expected ')' to close grouping");
  return create_grouping_expr(parser->arena, expr, p_current(parser).line,
                              p_current(parser).col);
}

Expr *binary(Parser *parser, Expr *left, BindingPower bp) {
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
  default:
    return left; // No valid binary operator found
  }

  p_advance(parser); // Consume the token
  Expr *right = parse_expr(parser, get_bp(current.type_));

  return create_binary_expr(parser->arena, op, left, right, current.line,
                            current.col);
}

Expr *call_expr(Parser *parser, Expr *left, BindingPower bp) { return NULL; }

Expr *assign_expr(Parser *parser, Expr *left, BindingPower bp) { return NULL; }

Expr *prefix_expr(Parser *parser, Expr *left, BindingPower bp) { return NULL; }