#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

Expr *primary(Parser *parser) {
  int line = p_current(parser).line;
  int col = p_current(parser).col;

  Token current = p_current(parser);
  LiteralType lit_type = PRIMARY_LITERAL_TYPE_MAP[current.type_];
  
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
  UnaryOp op = TOKEN_TO_UNOP_MAP[current.type_];

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
  BinaryOp op = TOKEN_TO_BINOP_MAP[current.type_];
  p_advance(parser); // Consume the token
  Expr *right = parse_expr(parser, bp);

  return create_binary_expr(parser->arena, op, left, right, line, col);
}

Expr *call_expr(Parser *parser, Expr *left, BindingPower bp) {
  (void)bp; // Unused parameter, can be removed if not needed 
  int line = p_current(parser).line;
  int col = p_current(parser).col;

  GrowableArray args;
  if (!growable_array_init(&args, parser->arena, 4, sizeof(Expr *))) {
    fprintf(stderr, "Failed to initialize call arguments\n"); 
  }

  p_consume(parser, TOK_LPAREN, "Expected '(' for function call");
  while (p_current(parser).type_ != TOK_RPAREN) {
    Expr *arg = parse_expr(parser, BP_LOWEST);
    if (!arg) {      
      fprintf(stderr, "Expected expression inside function call\n");
      return NULL;
    }
    Expr **slot = (Expr **)growable_array_push(&args);
    if (!slot) {      
      fprintf(stderr, "Out of memory while growing call arguments\n");
      return NULL;
    }
    *slot = arg;  
    if (p_current(parser).type_ == TOK_COMMA) {
      p_advance(parser); // Consume the comma
    }
  }
  p_consume(parser, TOK_RPAREN, "Expected ')' to close function call");

  return create_call_expr(parser->arena, left, (Expr **)args.data,
                           args.count, line, col);
}

Expr *assign_expr(Parser *parser, Expr *left, BindingPower bp) {
  (void)bp; // Unused parameter, can be removed if not needed
  int line = p_current(parser).line;
  int col = p_current(parser).col;

  if (p_current(parser).type_ != TOK_EQUAL) {
    fprintf(stderr, "Expected '=' for assignment\n");
    return NULL;
  }
  p_advance(parser); // Consume the '=' token

  Expr *value = parse_expr(parser, BP_ASSIGN);
  if (!value) {
    parser_error(parser, "Assignment Error", "parser.c",
                 "Failed to parse assignment value", line, col,
                 p_current(parser).length);
    return NULL;
  }

  return create_assignment_expr(parser->arena, left, value, line, col);
}

Expr *prefix_expr(Parser *parser, Expr *left, BindingPower bp) {
  (void)bp; // Unused parameter, can be removed if not needed
  int line = p_current(parser).line;
  int col = p_current(parser).col;

  Token current = p_current(parser);
  switch(current.type_) {
  case TOK_LBRACKET:
    p_advance(parser); // Consume the '[' token
    Expr *index = parse_expr(parser, BP_LOWEST);
    if (!index) {
      fprintf(stderr, "Expected expression inside index\n");
      return NULL;  
    }
    p_consume(parser, TOK_RBRACKET, "Expected ']' to close index expression");
    return create_index_expr(parser->arena, left, index, line, col);
  case TOK_DOT:
    p_advance(parser); // Consume the '.' token
    if (p_current(parser).type_ != TOK_IDENTIFIER) {
      fprintf(stderr, "Expected identifier after '.' for member access\n");
      return NULL;
    }
    char *member = get_name(parser);
    p_advance(parser); // Consume the identifier token
    return create_member_expr(parser->arena, left, member, line, col);
  case TOK_PLUSPLUS:
  case TOK_MINUSMINUS: {
    UnaryOp op = (current.type_ == TOK_PLUSPLUS) ? UNOP_POST_INC : UNOP_POST_DEC;
    p_advance(parser); // Consume the token
    return create_unary_expr(parser->arena, op, left, line, col);
  }
  default:
    fprintf(stderr, "Unexpected token for prefix expression: %s\n",
            current.value);
    return NULL; // Handle unexpected token 
  }
}

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
