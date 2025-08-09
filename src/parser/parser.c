#include <stdalign.h>
#include <stdio.h>

#include "../ast/ast.h"
#include "../c_libs/error/error.h"
#include "../c_libs/memory/memory.h"
#include "parser.h"

void parser_error(Parser *psr, const char *error_type, const char *file,
                  const char *msg, int line, int col, int tk_length) {
  // Use the same approach as the lexer to get the line text
  const char *line_text = get_line_text_from_source(psr->tks->value, line);

  ErrorInformation err = {
      .error_type = error_type,
      .file_path = file,
      .message = msg,
      .line = line,
      .col = col,
      .line_text = arena_strdup(psr->arena, line_text),
      .token_length = tk_length,
      .label = "Parser Error",
      .note = NULL,
      .help = NULL,
  };
  error_add(err);
}

Stmt *parse(GrowableArray *tks, ArenaAllocator *arena) {
  size_t estimated_stmts = (tks->count / 4) + 10;

  Parser parser = {
      .arena = arena,
      .tks = (Token *)tks->data,
      .tk_count = tks->count,
      .capacity = estimated_stmts,
      .pos = 0,
  };

  if (!parser.tks) {
    fprintf(stderr, "Failed to get tokens from GrowableArray\n");
    return NULL;
  }

  // Allocate array of statement pointers with proper alignment
  GrowableArray stmts;
  if (!growable_array_init(&stmts, parser.arena, 1024, sizeof(Stmt *))) {
    fprintf(stderr, "Failed to initialize the statements array.\n");
    return NULL;
  }

  while (p_has_tokens(&parser) && p_current(&parser).type_ != TOK_EOF) {
    Stmt *stmt = parse_stmt(&parser);
    if (!stmt) {
      fprintf(stderr, "parse_stmt returned NULL inside block\n");
      continue; // or return NULL to fail the entire block
    }

    Stmt **slot = (Stmt **)growable_array_push(&stmts);
    if (!slot) {
      fprintf(stderr, "Out of memory while growing block statement array\n");
      return NULL;
    }

    *slot = stmt;
  }

  // Cast to AstNode** since program expects AstNode**
  return create_program_node(parser.arena, (AstNode **)stmts.data, stmts.count,
                             0, 0);
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
  case TOK_PLUSPLUS:
  case TOK_MINUSMINUS:
    return unary(parser);
  case TOK_LPAREN:
    return grouping(parser);
  case TOK_LBRACKET:
    return array_expr(parser);
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
  case TOK_AND:     // Add logical AND
  case TOK_OR:      // Add logical OR
    return binary(parser, left, bp);
  case TOK_LPAREN:
    return call_expr(parser, left, bp);
  case TOK_LBRACKET:
    return assign_expr(parser, left, bp);
  case TOK_DOT:
  case TOK_PLUSPLUS:
  case TOK_MINUSMINUS:
    return prefix_expr(parser, left, bp);
  default:
    p_advance(parser);
    return left; // No valid LED found, return left expression
  }
}

Expr *parse_expr(Parser *parser, BindingPower bp) {
  Expr *left = nud(parser);

  while (p_has_tokens(parser) && get_bp(p_current(parser).type_) > bp) {
    left = led(parser, left, get_bp(p_current(parser).type_));
  }

  return left;
}

Stmt *parse_stmt(Parser *parser) {
  bool is_public = false;

  if (p_current(parser).type_ == TOK_PUBLIC) {
    is_public = true;
    p_advance(parser);
  } else if (p_current(parser).type_ == TOK_PRIVATE) {
    is_public = false;
    p_advance(parser);
  }

  switch (p_current(parser).type_) {
  case TOK_CONST:
    return const_stmt(parser, is_public);
  case TOK_VAR:
    return var_stmt(parser, is_public);
  case TOK_RETURN:
    return return_stmt(parser);
  case TOK_LBRACE:
    return block_stmt(parser);
  case TOK_IF:
    return if_stmt(parser);
  case TOK_LOOP:
    return loop_stmt(parser);
  case TOK_PRINT:
    return print_stmt(parser, false);
  case TOK_PRINTLN:
    return print_stmt(parser, true);
  default:
    return expr_stmt(
        parser); // expression statements handle their own semicolon
  }
}

Type *parse_type(Parser *parser) {
  TokenType tok = p_current(parser).type_;

  switch (tok) {
  case TOK_INT:
  case TOK_UINT:
  case TOK_FLOAT:
  case TOK_BOOL:
  case TOK_STRINGT:
  case TOK_VOID:
  case TOK_CHAR:
  case TOK_STAR: // Pointer type
  case TOK_LBRACKET: // Array type
    return tnud(parser);

  // Optionally: handle identifiers like 'MyStruct' or user-defined types
  case TOK_IDENTIFIER:
    return tled(parser, NULL, BP_NONE);

  default:
    fprintf(stderr, "[parse_type] Unexpected token for type: %d\n", tok);
    return NULL;
  }
}
