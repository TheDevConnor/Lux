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

Stmt *fn_stmt(Parser *parser, const char *name) { return NULL; }

Stmt *enum_stmt(Parser *parser, const char *name) { return NULL; }

Stmt *struct_stmt(Parser *parser, const char *name) { return NULL; }

Stmt *print_stmt(Parser *parser, bool ln) { return NULL; }

Stmt *return_stmt(Parser *parser) { return NULL; }

Stmt *block_stmt(Parser *parser) { return NULL; }

Stmt *loop_stmt(Parser *parser) { return NULL; }

Stmt *if_stmt(Parser *parser) { return NULL; }
