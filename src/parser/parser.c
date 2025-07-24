#include <stdalign.h>
#include <stdio.h>

#include "../c_libs/memory/memory.h"
#include "../ast/ast_utils.h"
#include "../ast/ast.h"
#include "parser.h"

Stmt *parse(GrowableArray *tks, ArenaAllocator *arena) {
  size_t estimated_stmts = (tks->count / 4) + 10;
  
  Parser parser = {
      .arena = arena,
      .tks = (Token *)tks->data,
      // Store pointers to statements, not statement values
      .stmts = (Stmt *)arena_alloc(arena, sizeof(Stmt *) * estimated_stmts, alignof(Stmt *)),
      .tk_count = tks->count,
      .stmt_count = 0,
      .capacity = estimated_stmts,
      .pos = 0,
  };

  if (!parser.tks || !parser.stmts) {
    fprintf(stderr, "Failed to allocate parser memory\n");
    return NULL;
  }
  
  // Create an array of statement pointers
  Stmt **stmt_ptrs = (Stmt **)arena_alloc(arena, sizeof(Stmt *) * parser.capacity, alignof(Stmt *));
  if (!stmt_ptrs) {
    fprintf(stderr, "Failed to allocate statement pointers\n");
    return NULL;
  }
  
  while (p_has_tokens(&parser) && p_current(&parser).type_ != TOK_EOF) {
    Stmt *stmt = parse_stmt(&parser);
    if (stmt == NULL) {
        fprintf(stderr, "Failed to parse statement at position %zu\n", parser.pos);
        break;
    }

    // Store the pointer to the statement (not the statement itself)
    stmt_ptrs[parser.stmt_count] = stmt;
    parser.stmt_count++;
    
    if (parser.stmt_count >= parser.capacity) {
        fprintf(stderr, "Too many statements, reached capacity limit of %zu\n", parser.capacity);
        break;
    }
  }

  // Cast to AstNode** since program expects AstNode** not Stmt**
  return create_program_node(parser.arena, (AstNode **)stmt_ptrs, parser.stmt_count, 0, 0);
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
    return unary(parser);
  case TOK_LPAREN:
    return grouping(parser);
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
  case TOK_AND:
  case TOK_OR:
    return binary(parser, left, bp);
  case TOK_LPAREN:
    return call_expr(parser, left, bp);
  case TOK_LBRACKET:
    return assign_expr(parser, left, bp);
  case TOK_DOT:
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
  switch (p_current(parser).type_) {
    case TOK_VAR:
      p_advance(parser);
      return var_stmt(parser);
    case TOK_FN:
      p_advance(parser);
      return fn_stmt(parser, NULL);
    case TOK_ENUM:
      p_advance(parser);
      return enum_stmt(parser, NULL);
    case TOK_STRUCT:
      p_advance(parser);
      return struct_stmt(parser, NULL);
    case TOK_RETURN:
      p_advance(parser);
      return return_stmt(parser);
    case TOK_LBRACE:
      return block_stmt(parser); // don't advance here; block_stmt does
    case TOK_LOOP:
      p_advance(parser);
      return loop_stmt(parser);
    case TOK_IF:
      p_advance(parser);
      return if_stmt(parser);
    case TOK_PRINT:
      p_advance(parser);
      return print_stmt(parser, false);
    case TOK_PRINTLN:
      p_advance(parser);
      return print_stmt(parser, true);
    default:
      return expr_stmt(parser); // expression statements handle their own semicolon
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
    return tnud(parser);

  // Optionally: handle identifiers like 'MyStruct' or user-defined types
  case TOK_IDENTIFIER:
    return tled(parser, NULL, BP_NONE);

  default:
    fprintf(stderr, "[parse_type] Unexpected token for type: %d\n", tok);
    return NULL;
  }
}

