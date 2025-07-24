#include <stdio.h>

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
  /*
    * fn main () int {
    *   return 0;
    * }
  */

  name = CURRENT_TOKEN_VALUE(parser) - CURRENT_TOKEN_LENGTH(parser);
  p_consume(parser, TOK_IDENTIFIER, "Expected function name after 'fn' keyword");

  p_consume(parser, TOK_LPAREN, "Expected '(' after function name");
  p_consume(parser, TOK_RPAREN, "Expected ')' after function parameters");

  Type *return_type = parse_type(parser);
  p_advance(parser);

  Stmt *body = block_stmt(parser);

  return create_func_decl_stmt(parser->arena, name, NULL, NULL,
                               0, return_type, body, p_current(parser).line, p_current(parser).col);
}

Stmt *enum_stmt(Parser *parser, const char *name) { return NULL; }

Stmt *struct_stmt(Parser *parser, const char *name) { return NULL; }

Stmt *print_stmt(Parser *parser, bool ln) { return NULL; }

Stmt *return_stmt(Parser *parser) { return NULL; }

Stmt *block_stmt(Parser *parser) { 
  p_consume(parser, TOK_LBRACE, "Expected '{' to start block statement");
  Stmt *block = (Stmt *)arena_alloc(parser->arena, sizeof(Stmt), alignof(Stmt));

  while (p_has_tokens(parser) && p_current(parser).type_ != TOK_RBRACE) {
    Stmt *stmt = parse_stmt(parser);
    block->stmt.block.statements[block->stmt.block.stmt_count++] = stmt;
  }

  p_consume(parser, TOK_RBRACE, "Expected '}' to end block statement");
  return create_block_stmt(parser->arena, &block, block->stmt.block.stmt_count, p_current(parser).line, p_current(parser).col);
}

Stmt *loop_stmt(Parser *parser) { return NULL; }

Stmt *if_stmt(Parser *parser) { return NULL; }
