#include <stdio.h>

#include "../ast/ast.h"
#include "parser.h"

Stmt *expr_stmt(Parser *parser) {
  // Capture line/col info at the beginning
  int line = p_current(parser).line;
  int col = p_current(parser).col;

  Expr *expr = parse_expr(parser, BP_LOWEST);
  p_consume(parser, TOK_SEMICOLON, "Expected semicolon after expression statement");
  return create_expr_stmt(parser->arena, expr, line, col);
}

// const 'name' = (fn, struct, or enum)
// If no type is specified, it defaults to the type of the value
// const 'name': Type = value
Stmt *const_stmt(Parser *parser, bool is_public) {
  int line = p_current(parser).line;
  int col = p_current(parser).col;

  p_consume(parser, TOK_CONST, "Expected 'const' keyword");
  const char *name = get_name(parser);
  p_advance(parser); // Advance past the identifier token

  if (p_current(parser).type_ == TOK_COLON) {
    p_consume(parser, TOK_COLON, "Expected ':' after const name");

    Type *type = parse_type(parser);
    p_advance(parser); // Advance past the type token

    p_consume(parser, TOK_EQUAL, "Expected '=' after const type");
    Expr *value = parse_expr(parser, BP_LOWEST);

    p_consume(parser, TOK_SEMICOLON, "Expected semicolon after const declaration");
    return create_var_decl_stmt(parser->arena, name, type, value, false, is_public, line, col);
  } 
  
  p_consume(parser, TOK_EQUAL, "Expected '=' after const name");

  switch (p_current(parser).type_) {
    case TOK_FN:     return fn_stmt(parser, name, is_public);
    case TOK_STRUCT: return struct_stmt(parser, name);
    case TOK_ENUM:   return enum_stmt(parser, name, is_public);
    default: {
      fprintf(stderr, "Expected function, struct, or enum after const '%s'\n", name);
      return NULL;
    }
  }
}

Stmt *fn_stmt(Parser *parser, const char *name, bool is_public) {
  int line = p_current(parser).line;
  int col = p_current(parser).col;

  GrowableArray param_names, param_types;
  if (!growable_array_init(&param_names, parser->arena, 4, sizeof(char *)) ||
      !growable_array_init(&param_types, parser->arena, 4, sizeof(Type *))) {
    fprintf(stderr, "Failed to initialize parameter arrays.\n");
    return NULL;
  }

  p_consume(parser, TOK_FN, "Expected 'fn' keyword");
  p_consume(parser, TOK_LPAREN, "Expected '(' after function name");

  while (p_has_tokens(parser) && p_current(parser).type_ != TOK_RPAREN) {
    if (p_current(parser).type_ != TOK_IDENTIFIER) {
      fprintf(stderr, "Expected identifier for function parameter\n");
      return NULL;
    }

    char *param_name = get_name(parser);
    p_advance(parser); // Advance past the identifier token

    p_consume(parser, TOK_COLON, "Expected ':' after parameter name");

    Type *param_type = parse_type(parser);
    if (!param_type) {
      fprintf(stderr, "Failed to parse type for parameter '%s'\n", param_name);
      return NULL;
    }
    p_advance(parser); // Advance past the type token

    char **name_slot = (char **)growable_array_push(&param_names);
    Type **type_slot = (Type **)growable_array_push(&param_types);
    if (!name_slot || !type_slot) {
      fprintf(stderr, "Out of memory while growing parameter arrays\n");
      return NULL;
    }

    *name_slot = param_name;
    *type_slot = param_type;

    if (p_current(parser).type_ == TOK_COMMA) {
      p_advance(parser); // Advance past the comma
    }
  }

  p_consume(parser, TOK_RPAREN, "Expected ')' after function parameters");

  Type *return_type = parse_type(parser);
  p_advance(parser); // Advance past the return type token

  Stmt *body = block_stmt(parser);
  p_consume(parser, TOK_SEMICOLON, "Expected semicolon after function declaration");

  return create_func_decl_stmt(parser->arena, name, (char **)param_names.data,
                               (AstNode **)param_types.data, param_names.count,
                               return_type, is_public, body, line, col);
}

Stmt *enum_stmt(Parser *parser, const char *name, bool is_public) { 
  int line = p_current(parser).line;
  int col = p_current(parser).col;

  GrowableArray members;
  if (!growable_array_init(&members, parser->arena, 4, sizeof(char *))) {
    fprintf(stderr, "Failed to initialize enum members array.\n");
    return NULL;
  }

  p_consume(parser, TOK_ENUM, "Expected 'enum' keyword");
  p_consume(parser, TOK_LBRACE, "Expected '{' after enum name");

  while (p_has_tokens(parser) && p_current(parser).type_ != TOK_RBRACE) {
    if (p_current(parser).type_ != TOK_IDENTIFIER) {
      fprintf(stderr, "Expected identifier for enum member\n");
      return NULL;
    }

    char *member_name = get_name(parser);
    p_advance(parser); // Advance past the identifier token

    char **slot = (char **)growable_array_push(&members);
    if (!slot) {
      fprintf(stderr, "Out of memory while growing enum members array\n");
      return NULL;
    }
    *slot = member_name;

    if (p_current(parser).type_ == TOK_COMMA) {
      p_advance(parser); // Advance past the comma
    }
  }

  p_consume(parser, TOK_RBRACE, "Expected '}' to end enum declaration");
  p_consume(parser, TOK_SEMICOLON, "Expected semicolon after enum declaration");

  return create_enum_decl_stmt(parser->arena, name,
                               (char **)members.data, members.count, is_public,
                               line, col);
}

Stmt *struct_stmt(Parser *parser, const char *name) { return NULL; }

// let 'name': Type = value
Stmt *var_stmt(Parser *parser, bool is_public) { 
  int line = p_current(parser).line;
  int col = p_current(parser).col;

  p_consume(parser, TOK_VAR, "Expected 'let' keyword");
  const char *name = get_name(parser);
  p_advance(parser); // Advance past the identifier token

  p_consume(parser, TOK_COLON, "Expected ':' after variable name");
  Type *type = parse_type(parser);
  p_advance(parser); // Advance past the type token

  p_consume(parser, TOK_EQUAL, "Expected '=' after variable declaration");

  Expr *value = parse_expr(parser, BP_LOWEST);
  p_consume(parser, TOK_SEMICOLON, "Expected semicolon after variable declaration");

  // const are not changable aka immutable and vars are mutable so we set is_mutable to true
  return create_var_decl_stmt(parser->arena, name, type, value, true, is_public,
                              line, col);
}

// return Expr;
Stmt *return_stmt(Parser *parser) { 
  int line = p_current(parser).line;
  int col = p_current(parser).col;
    

  p_consume(parser, TOK_RETURN, "Expected 'return' keyword");
  Expr *value = NULL;
  if (p_current(parser).type_ != TOK_SEMICOLON) {
    value = parse_expr(parser, BP_LOWEST);
  }
  p_consume(parser, TOK_SEMICOLON, "Expected semicolon after return statement");
  
  return create_return_stmt(parser->arena, value, line, col);
}

// { Stmt* }
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

Stmt *if_stmt(Parser *parser) { 
    int line = p_current(parser).line;
    int col = p_current(parser).col;
    
    if (p_current(parser).type_ != TOK_IF && p_current(parser).type_ != TOK_ELIF) {
        fprintf(stderr, "Expected 'if' or 'elif' keyword\n");
        return NULL;
    }
    p_consume(parser, p_current(parser).type_, "Expected 'if' or 'elif' keyword");
    
    p_consume(parser, TOK_LPAREN, "Expected '(' after 'if' keyword");
    Expr *condition = parse_expr(parser, BP_LOWEST);
    p_consume(parser, TOK_RPAREN, "Expected ')' after if condition");

    Stmt *then_stmt = block_stmt(parser);
    
    // Collect all elif statements in a list/array instead of recursing
    Stmt **elif_stmts = (Stmt **)arena_alloc(parser->arena, sizeof(Stmt *) * 4, alignof(Stmt *));
    int elif_count = 0;

    while (p_has_tokens(parser) && p_current(parser).type_ == TOK_ELIF) {
        int elif_line = p_current(parser).line;
        int elif_col = p_current(parser).col;

        p_consume(parser, TOK_ELIF, "Expected 'elif' keyword");
        p_consume(parser, TOK_LPAREN, "Expected '(' after 'elif' keyword");
        
        Expr *elif_condition = parse_expr(parser, BP_LOWEST);
        p_consume(parser, TOK_RPAREN, "Expected ')' after elif condition");

        Stmt *elif_stmt = block_stmt(parser);

        // Store the elif condition and statement
        elif_stmts[elif_count] = create_if_stmt(parser->arena, elif_condition, elif_stmt, NULL, elif_count, NULL, elif_line, elif_col);
        elif_count++;
    }

    Stmt *else_stmt = NULL;
    if (p_current(parser).type_ == TOK_ELSE) {
        p_consume(parser, TOK_ELSE, "Expected 'else' keyword");
        else_stmt = block_stmt(parser);
    }

    return create_if_stmt(parser->arena, condition, then_stmt, elif_stmts, elif_count, else_stmt, line, col);
}

Stmt *loop_stmt(Parser *parser) { return NULL; }

Stmt *print_stmt(Parser *parser, bool ln) { return NULL; }