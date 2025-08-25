/**
 * @file expr.c
 * @brief Expression parsing implementation for the programming language
 * compiler
 *
 * This file contains implementations for parsing all types of expressions in
 * the programming language. It works in conjunction with the Pratt parser
 * implementation in parser.c to handle operator precedence and associativity
 * correctly.
 *
 * The expression parser handles:
 * - Primary expressions: literals, identifiers
 * - Unary expressions: prefix and postfix operators
 * - Binary expressions: arithmetic, logical, comparison operators
 * - Function call expressions with argument lists
 * - Assignment expressions
 * - Member access and array indexing expressions
 * - Grouping expressions with parentheses
 * - Array literal expressions
 *
 * All parsing functions follow the Pratt parser pattern, where expressions are
 * built recursively based on operator precedence (binding power). The functions
 * correspond to null denotation (nud) and left denotation (led) operations.
 *
 * @author Connor Harris
 * @date 2025
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

/**
 * @brief Parses primary expressions (literals and identifiers)
 *
 * This function handles the most basic expressions that don't have any
 * operators: integer literals, floating-point literals, string literals,
 * character literals, boolean literals, and identifiers. It uses a lookup table
 * to map token types to literal types and handles the appropriate conversion
 * and memory allocation for each type.
 *
 * @param parser Pointer to the parser instance
 *
 * @return Pointer to the created literal or identifier expression AST node,
 *         or NULL if the current token is not a valid primary expression
 *
 * @note Primary expressions are the "atoms" of the expression language
 * @note Memory for literal values is allocated using the arena allocator
 * @note String literals are copied to ensure they remain valid after parsing
 * @note Identifiers are handled separately and create identifier expression
 * nodes
 * @note Numeric conversions use standard library functions (strtoll, strtod)
 * @note Boolean literals are converted from "true"/"false" string comparisons
 *
 * @see create_literal_expr(), create_identifier_expr(), get_name()
 */
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

/**
 * @brief Parses unary expressions (prefix operators)
 *
 * This function handles prefix unary operators such as negation (-), logical
 * NOT (!), unary plus (+), and prefix increment/decrement (++, --). It uses a
 * lookup table to map token types to unary operator types and recursively
 * parses the operand with appropriate precedence.
 *
 * @param parser Pointer to the parser instance
 *
 * @return Pointer to the created unary expression AST node, or NULL if the
 *         current token is not a valid unary operator
 *
 * @note Unary expressions have higher precedence than most binary operators
 * @note The operand is parsed with BP_UNARY precedence to handle operator
 * nesting
 * @note Supports both arithmetic unary operators (-, +) and logical operators
 * (!)
 * @note Prefix increment/decrement operators are handled here (postfix in
 * prefix_expr)
 *
 * @see create_unary_expr(), parse_expr(), UnaryOp
 */
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

/**
 * @brief Parses grouped expressions (parenthesized expressions)
 *
 * This function handles expressions enclosed in parentheses, which are used
 * to override the default operator precedence. The parentheses themselves
 * don't create a separate AST node; they just influence parsing order.
 *
 * @param parser Pointer to the parser instance
 *
 * @return Pointer to the expression AST node inside the parentheses,
 *         or NULL if parsing fails
 *
 * @note Parentheses don't create additional AST nodes - they only affect
 * parsing order
 * @note The inner expression is parsed with BP_LOWEST to allow any expression
 * inside
 * @note Both opening and closing parentheses are consumed and required
 * @note Nested parentheses are handled naturally through recursive expression
 * parsing
 *
 * @see create_grouping_expr(), parse_expr(), p_consume()
 */
Expr *grouping(Parser *parser) {
  int line = p_current(parser).line;
  int col = p_current(parser).col;

  p_consume(parser, TOK_LPAREN, "Expected '(' for grouping");
  Expr *expr = parse_expr(parser, BP_LOWEST);
  p_consume(parser, TOK_RPAREN, "Expected ')' to close grouping");
  return create_grouping_expr(parser->arena, expr, line, col);
}

/**
 * @brief Parses binary expressions (infix operators)
 *
 * This function handles all binary infix operators including arithmetic (+, -,
 * *, /), comparison (==, !=, <, >, <=, >=), logical (&&, ||), and bitwise (&,
 * |, ^) operators. It's called as part of the left denotation (led) process in
 * the Pratt parser.
 *
 * @param parser Pointer to the parser instance
 * @param left The left operand expression (already parsed)
 * @param bp The current binding power context for precedence handling
 *
 * @return Pointer to the created binary expression AST node
 *
 * @note This function is part of the Pratt parser's led (left denotation)
 * mechanism
 * @note The left operand is already parsed by the time this function is called
 * @note The right operand is parsed with the current binding power to handle
 * precedence
 * @note Operator associativity is handled by the binding power passed to
 * parse_expr
 * @note Uses a lookup table to map token types to binary operator types
 *
 * @see create_binary_expr(), parse_expr(), BinaryOp, BindingPower
 */
Expr *binary(Parser *parser, Expr *left, BindingPower bp) {
  int line = p_current(parser).line;
  int col = p_current(parser).col;

  Token current = p_current(parser);
  BinaryOp op = TOKEN_TO_BINOP_MAP[current.type_];
  p_advance(parser); // Consume the token
  Expr *right = parse_expr(parser, bp);

  return create_binary_expr(parser->arena, op, left, right, line, col);
}

/**
 * @brief Parses function call expressions
 *
 * This function handles function call syntax where the left operand is the
 * function to call and the arguments are provided in parentheses. It supports
 * zero or more arguments separated by commas.
 *
 * @param parser Pointer to the parser instance
 * @param left The function expression to call (already parsed)
 * @param bp The current binding power context (unused in this function)
 *
 * @return Pointer to the created function call expression AST node,
 *         or NULL if parsing fails
 *
 * @note Function calls have very high precedence (BP_CALL)
 * @note Arguments are stored in a growable array and can be of any expression
 * type
 * @note Empty argument lists are allowed: `function()`
 * @note Arguments are separated by commas with optional trailing comma handling
 * @note The function being called (left operand) can be any expression
 * (identifier, member access, etc.)
 *
 * @see create_call_expr(), parse_expr(), GrowableArray
 */
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

  return create_call_expr(parser->arena, left, (Expr **)args.data, args.count,
                          line, col);
}

/**
 * @brief Parses assignment expressions
 *
 * This function handles assignment operations where a value is assigned to
 * a variable or other assignable expression (lvalue). The left operand must
 * be a valid assignment target.
 *
 * @param parser Pointer to the parser instance
 * @param left The left-hand side expression being assigned to (already parsed)
 * @param bp The current binding power context (unused in this function)
 *
 * @return Pointer to the created assignment expression AST node,
 *         or NULL if parsing fails
 *
 * @note Assignment is right-associative with low precedence
 * @note The left operand should be an lvalue (identifier, member access, index)
 * @note The right operand is parsed with BP_ASSIGN precedence for
 * right-associativity
 * @note Validation of assignment targets is typically done during semantic
 * analysis
 *
 * @see create_assignment_expr(), parse_expr(), parser_error()
 */
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

/**
 * @brief Parses prefix expressions (member access, indexing, postfix operators)
 *
 * This function handles expressions that operate on a left operand:
 * - Member access: `object.member`
 * - Array indexing: `array[index]`
 * - Postfix increment/decrement: `variable++`, `variable--`
 *
 * Despite the name "prefix", this function handles postfix and infix operations
 * that take a left operand. It's part of the led (left denotation) mechanism.
 *
 * @param parser Pointer to the parser instance
 * @param left The left operand expression (object, array, or variable)
 * @param bp The current binding power context (unused in this function)
 *
 * @return Pointer to the appropriate expression AST node (member, index, or
 * unary), or NULL if parsing fails
 *
 * @note Despite the name, this handles postfix and member access operations
 * @note Member access requires an identifier after the dot
 * @note Array indexing can use any expression as the index
 * @note Postfix increment/decrement create unary expressions with special
 * operators
 * @note All operations have high precedence (BP_CALL level)
 *
 * @see create_index_expr(), create_member_expr(), create_unary_expr()
 */
Expr *prefix_expr(Parser *parser, Expr *left, BindingPower bp) {
  (void)bp; // Unused parameter, can be removed if not needed
  int line = p_current(parser).line;
  int col = p_current(parser).col;

  Token current = p_current(parser);
  switch (current.type_) {
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
    UnaryOp op =
        (current.type_ == TOK_PLUSPLUS) ? UNOP_POST_INC : UNOP_POST_DEC;
    p_advance(parser); // Consume the token
    return create_unary_expr(parser->arena, op, left, line, col);
  }
  default:
    fprintf(stderr, "Unexpected token for prefix expression: %s\n",
            current.value);
    return NULL; // Handle unexpected token
  }
}

/**
 * @brief Parses array literal expressions
 *
 * This function handles array literal syntax with square brackets containing
 * a comma-separated list of expressions: `[expr1, expr2, expr3, ...]`.
 * Empty arrays are allowed.
 *
 * @param parser Pointer to the parser instance
 *
 * @return Pointer to the created array expression AST node,
 *         or NULL if parsing fails
 *
 * @note Array literals can contain any number of expressions (including zero)
 * @note Elements are separated by commas with optional trailing comma support
 * @note Each element can be any valid expression (literals, identifiers,
 * function calls, etc.)
 * @note Memory for the element array is managed using growable arrays
 * @note Empty arrays are valid: `[]`
 *
 * @see create_array_expr(), parse_expr(), GrowableArray
 */
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

Expr *deref_expr(Parser *parser) {
  p_advance(parser); // Advance past the *
  int line = p_current(parser).line;
  int col = p_current(parser).col;

  Expr *object = parse_expr(parser, BP_NONE);

  return create_deref_expr(parser->arena, object, line, col);
}

Expr *addr_expr(Parser *parser) {
  p_advance(parser); // Advance past the &
  int line = p_current(parser).line;
  int col = p_current(parser).col;

  Expr *object = parse_expr(parser, BP_NONE);

  return create_addr_expr(parser->arena, object, line, col);
}

// void *alloc(size_t size);
Expr *alloc_expr(Parser *parser) {
  p_advance(parser); // Advance past the alloc
  int line = p_current(parser).line;
  int col = p_current(parser).col;

  p_consume(parser, TOK_LPAREN,
            "Expected an '(' before you define your size for alloc.");
  Expr *size = parse_expr(parser, BP_NONE);
  p_consume(parser, TOK_RPAREN,
            "Expected an ')' after you define your size for alloc.");

  return create_alloc_expr(parser->arena, size, line, col);
}

// void *memcpy(void *to, void *from, size_t size);
Expr *memcpy_expr(Parser *parser) {
  p_advance(parser); // Advance past the memcpy
  int line = p_current(parser).line;
  int col = p_current(parser).col;

  p_consume(parser, TOK_LPAREN,
            "Expected an '(' before you define your params for memcpy.");
  Expr *to = parse_expr(parser, BP_NONE);
  p_consume(parser, TOK_COMMA,
            "Expected an ',' after you define your 'to' param for memcpy.");
  Expr *from = parse_expr(parser, BP_NONE);
  p_consume(parser, TOK_COMMA,
            "Expected an ',' after you define your 'from' param for memcpy.");
  Expr *size = parse_expr(parser, BP_NONE);
  p_consume(parser, TOK_RPAREN,
            "Expected an ')' after you define your params for memcpy.");

  return create_memcpy_expr(parser->arena, to, from, size, line, col);
}

// void free(void *ptr)
Expr *free_expr(Parser *parser) {
  p_advance(parser); // Advance past the memcpy
  int line = p_current(parser).line;
  int col = p_current(parser).col;

  p_consume(parser, TOK_LPAREN,
            "Expected an '(' before you pass your variable to free.");
  Expr *ptr = parse_expr(parser, BP_NONE);
  p_consume(parser, TOK_RPAREN,
            "Expected an ')' after you pass your variable to free.");

  return create_free_expr(parser->arena, ptr, line, col);
}

// cast<TYPE>(value);
Expr *cast_expr(Parser *parser) {
  p_advance(parser); // Advance past the memcpy
  int line = p_current(parser).line;
  int col = p_current(parser).col;
  return NULL;
}

// size_t sizeof(TYPE);
// sizeof<int>         Compile-time constant
// sizeof<[10]int>     Compile-time constant
// sizeof<[n]int>      Runtime when n is variable
// sizeof<MyStruct>    Compile-time constant
Expr *sizeof_expr(Parser *parser) {
  p_advance(parser); // Advance past the memcpy
  int line = p_current(parser).line;
  int col = p_current(parser).col;
  return NULL;
}
