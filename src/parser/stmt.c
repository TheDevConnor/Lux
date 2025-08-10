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
    case TOK_STRUCT: return struct_stmt(parser, name, is_public);
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

/*
  const Person = struct {
  pub: 
    name: str,
    age: int,
    email: str,

    init = fn (self: Person) void {
      // Constructor logic here
    }
  priv: 
    ssn: str,

    // Constructor logic here
  };
*/

Stmt *struct_stmt(Parser *parser, const char *name, bool is_public) {
  int line = p_current(parser).line;
  int col = p_current(parser).col;

  p_consume(parser, TOK_STRUCT, "Expected 'struct' keyword");
  p_consume(parser, TOK_LBRACE, "Expected '{' after struct name");

  GrowableArray public_fields;
  GrowableArray private_fields;
  if (!growable_array_init(&public_fields, parser->arena, 4, sizeof(Stmt *)) || 
      !growable_array_init(&private_fields, parser->arena, 4, sizeof(Stmt *))) {
    fprintf(stderr, "Failed to initialize field arrays.\n");
    return NULL;
  }

  bool public_member = false;

  while (p_has_tokens(parser) && p_current(parser).type_ != TOK_RBRACE) {
    switch (p_current(parser).type_) {
      case TOK_PUBLIC: 
        public_member = true;
        p_advance(parser); // Advance past the 'public' keyword
        break;
      case TOK_PRIVATE:
        public_member = false;
        p_advance(parser); // Advance past the 'private' keyword
        break;
      default:
        fprintf(stderr, "Unexpected token in struct declaration\n");
        return NULL;
    }
    p_consume(parser, TOK_COLON, "Expected ':' after visibility keyword");

   while (p_has_tokens(parser) &&
           p_current(parser).type_ != TOK_PUBLIC  &&
           p_current(parser).type_ != TOK_PRIVATE &&
           p_current(parser).type_ != TOK_RBRACE) {
        int field_line = p_current(parser).line;
        int field_col = p_current(parser).col;

        char *field_name = get_name(parser);
        p_advance(parser);
        p_consume(parser, TOK_COLON, "Expected ':' after field name");
        Type *field_type = parse_type(parser);
        p_advance(parser);

        if (p_current(parser).type_ == TOK_COMMA)
            p_advance(parser);
        else if (p_current(parser).type_ != TOK_RBRACE) { // We expected a ','
          parser_error(parser, "Unexpected token", __FILE__, "Expected ',' to separate struct fields", field_line, field_col, 1);
          return NULL;
        }

        Stmt *field_decl = create_field_decl_stmt(parser->arena, field_name, field_type, public_member, field_line, field_col);
        Stmt **slot = public_member ?
            (Stmt **)growable_array_push(&public_fields) :
            (Stmt **)growable_array_push(&private_fields);

        *slot = field_decl;
    }
  }

  p_consume(parser, TOK_RBRACE, "Expected '}' to end struct declaration");
  p_consume(parser, TOK_SEMICOLON, "Expected semicolon after struct declaration");

  return create_struct_decl_stmt(parser->arena, name,
                                 (Stmt **)public_fields.data, public_fields.count, 
                                 (Stmt **)private_fields.data, private_fields.count,
                                 is_public, line, col);
}

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

// loop { ... }
Stmt *infinite_loop_stmt(Parser *parser, int line, int col) {
  Stmt *body = block_stmt(parser);
  if (!body) {
    parser_error(parser, "Syntax Error", __FILE__, "Expected block statement", line, col, 1);
    return NULL;
  }
  return create_infinite_loop_stmt(parser->arena, body, line, col);
}

//       init    cond     o_cond   body
// loop [ ... ]( ... ) : ( ... ) { ... }
// loop [ ... ]( ... ) { ... }
Stmt *loop_init(Parser *parser, int line, int col) {
  const char *name = get_name(parser);
  p_advance(parser); // Advance past the identifier token

  p_consume(parser, TOK_COLON, "Expected ':' after loop initializer");
  Type *type = parse_type(parser);
  p_advance(parser); // Advance past the type token

  p_consume(parser, TOK_EQUAL, "Expected '=' after loop initializer");
  Expr *initializer = parse_expr(parser, BP_LOWEST);
  return create_var_decl_stmt(parser->arena, name, type, initializer, true, false, line, col);
}

Stmt *for_loop_stmt(Parser *parser, int line, int col) { 
  GrowableArray intializers;
  if (!growable_array_init(&intializers, parser->arena, 4, sizeof(Expr *))) {
    fprintf(stderr, "Failed to initialize loop initializers array.\n");
    return NULL;
  }

  p_consume(parser, TOK_LBRACKET, "Expected '[' after 'loop' keyword");
  // i: int = 0, j: int = 1
  while (p_has_tokens(parser) && p_current(parser).type_ != TOK_RBRACKET) {
    Expr *init = loop_init(parser, line, col);
    if (!init) {
      fprintf(stderr, "Failed to parse loop initializer\n");
      return NULL;
    }

    Expr **slot = (Expr **)growable_array_push(&intializers);
    if (!slot) {
      fprintf(stderr, "Out of memory while growing loop initializers array\n");
      return NULL;
    }

    *slot = init;

    if (p_current(parser).type_ == TOK_COMMA) {
      p_advance(parser); // Advance past the comma
    }
  }
  p_consume(parser, TOK_RBRACKET, "Expected ']' after loop initializer");

  p_consume(parser, TOK_LPAREN, "Expected '(' after loop initializer");
  Expr *condition = parse_expr(parser, BP_LOWEST);
  p_consume(parser, TOK_RPAREN, "Expected ')' after loop initializer");

  // check for the optional condition
  Expr *optional_condition = NULL;
  if (p_current(parser).type_ == TOK_COLON) {
    p_consume(parser, TOK_COLON, "Expected ':' after loop condition");
    p_consume(parser, TOK_LPAREN, "Expected '(' after ':' in loop statement");
    optional_condition = parse_expr(parser, BP_LOWEST);
    p_consume(parser, TOK_RPAREN, "Expected ')' after optional condition in loop statement");
  }

  Stmt *body = block_stmt(parser);
  return create_for_loop_stmt(parser->arena, (AstNode **)intializers.data, intializers.count, condition, optional_condition, body, line, col);
}

// loop (condition) { ... }
// loop (condition) : (optional_condition) { ... }
Stmt *loop_stmt(Parser *parser) { 
  int line = p_current(parser).line;
  int col = p_current(parser).col;

  p_consume(parser, TOK_LOOP, "Expected 'loop' keyword");

  if (p_current(parser).type_ == TOK_LBRACE) { // Aka we have a infinite loop 'loop { ... }'
    return infinite_loop_stmt(parser, line, col);
  }

  if (p_current(parser).type_ == TOK_LBRACKET) { // Aka we have a for loop 'loop [ ... ] { ... }'
    return for_loop_stmt(parser, line, col);
  }

  // else we have a standard while loop 'loop (condition) { ... }'
  p_consume(parser, TOK_LPAREN, "Expected '(' after 'loop' keyword");
  Expr *condition = parse_expr(parser, BP_LOWEST);
  p_consume(parser, TOK_RPAREN, "Expected ')' after loop condition");

  // check if there is an optional condition 'loop (condition) : (optional_condition) { ... }'
  Expr *optional_condition = NULL;
  if (p_current(parser).type_ == TOK_COLON) {
    p_advance(parser); // Advance past the colon
    p_consume(parser, TOK_LPAREN, "Expected '(' after ':' in loop statement");
    optional_condition = parse_expr(parser, BP_LOWEST);
    p_consume(parser, TOK_RPAREN, "Expected ')' after optional condition in loop statement");
  }

  Stmt *body = block_stmt(parser);
  return create_loop_stmt(parser->arena, condition, optional_condition, body, line, col);
}

Stmt *print_stmt(Parser *parser, bool ln) { 
  int line = p_current(parser).line;
  int col = p_current(parser).col;

  p_consume(parser, ln ? TOK_PRINTLN : TOK_PRINT, "Expected 'output' or 'outputln' keyword");
  p_consume(parser, TOK_LPAREN, "Expected '(' after print statement");

  GrowableArray expressions;
  if (!growable_array_init(&expressions, parser->arena, 4, sizeof(Expr *))) {
    fprintf(stderr, "Failed to initialize print expressions array.\n");
    return NULL;
  }

  while (p_has_tokens(parser) && p_current(parser).type_ != TOK_RPAREN) {
    Expr *expr = parse_expr(parser, BP_LOWEST);
    if (!expr) {
      fprintf(stderr, "Failed to parse expression in print statement\n");
      return NULL;
    }

    Expr **slot = (Expr **)growable_array_push(&expressions);
    if (!slot) {
      fprintf(stderr, "Out of memory while growing print expressions array\n");
      return NULL;
    }
    
    *slot = expr;

    if (p_current(parser).type_ == TOK_COMMA) {
      p_advance(parser); // Advance past the comma
    }
  }
  p_consume(parser, TOK_RPAREN, "Expected ')' to end print statement");
  p_consume(parser, TOK_SEMICOLON, "Expected semicolon after print statement");

  return create_print_stmt(parser->arena, (Expr **)expressions.data,
                           expressions.count, ln, line, col);
}