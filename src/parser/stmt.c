#include <stdio.h>
#include <string.h>

#include "../ast/ast_utils.h"
#include "../ast/ast.h"
#include "parser.h"

Stmt *expr_stmt(Parser *parser) {
  Expr *expr = parse_expr(parser, BP_LOWEST);
  p_consume(parser, TOK_SEMICOLON, "Expected semicolon after expression statement");
  return create_expr_stmt(parser->arena, expr, p_current(parser).line,
                          p_current(parser).col);
}

Stmt *var_stmt(Parser *parser) { return NULL; }

Stmt *fn_stmt(Parser *parser, const char *name) { 
  // First consume the identifier token and get its name
  if (p_current(parser).type_ != TOK_IDENTIFIER) {
    fprintf(stderr, "Expected function name after 'fn' keyword\n");
    return NULL;
  }
  
  name = get_name(parser);
  p_advance(parser); // Now advance past the identifier

  p_consume(parser, TOK_LPAREN, "Expected '(' after function name");
  p_consume(parser, TOK_RPAREN, "Expected ')' after function parameters");

  Type *return_type = parse_type(parser);
  p_advance(parser); // Advance past the return type token

  Stmt *body = block_stmt(parser);

  return create_func_decl_stmt(parser->arena, name, NULL, NULL,
                               0, return_type, body, 
                               p_current(parser).line, p_current(parser).col);
}

Stmt *enum_stmt(Parser *parser, const char *name) { return NULL; }

Stmt *struct_stmt(Parser *parser, const char *name) { return NULL; }

Stmt *print_stmt(Parser *parser, bool ln) { return NULL; }

Stmt *return_stmt(Parser *parser) { 
  Expr *value = NULL;
  if (p_current(parser).type_ != TOK_SEMICOLON) {
    value = parse_expr(parser, BP_LOWEST);
  }
  p_consume(parser, TOK_SEMICOLON, "Expected semicolon after return statement");
  
  return create_return_stmt(parser->arena, value, p_current(parser).line, p_current(parser).col);
}

Stmt *block_stmt(Parser *parser) {
  p_consume(parser, TOK_LBRACE, "Expected '{' to start block statement");

  GrowableArray block;
  if (!growable_array_init(&block, parser->arena, 4, sizeof(Stmt *))) {
    fprintf(stderr, "Failed to initialize block statement array.\n");
    return NULL;
  }

  while (p_has_tokens(parser) && p_current(parser).type_ != TOK_RBRACE) {
    Stmt *stmt = parse_stmt(parser);
    if (!stmt) {
      fprintf(stderr, "parse_stmt returned NULL inside block\n");
      continue;  // or return NULL to fail the entire block
    }

    Stmt **slot = (Stmt **)growable_array_push(&block);
    if (!slot) {
      fprintf(stderr, "Out of memory while growing block statement array\n");
      return NULL;
    }

    *slot = stmt;
  }

  p_consume(parser, TOK_RBRACE, "Expected '}' to end block statement");

  Stmt **stmts = (Stmt **)block.data;
  if (block.count == 0) {
    return create_block_stmt(parser->arena, NULL, 0, p_current(parser).line, p_current(parser).col);
  }

  return create_block_stmt(parser->arena, stmts, block.count, p_current(parser).line, p_current(parser).col);
}

Stmt *loop_stmt(Parser *parser) { return NULL; }

Stmt *if_stmt(Parser *parser) { return NULL; }
