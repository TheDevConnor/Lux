#pragma once

#include <stddef.h>

#include "../ast/ast.h"
#include "../c_libs/memory/memory.h"
#include "../lexer/lexer.h"

typedef enum {
  BP_NONE = 0, // No binding power
  BP_LOWEST,   // Lowest binding power
  BP_ASSIGN,   // Assignment operators (=, +=, -=, *=, /=, %=, &=, |=, ^=, <<=,
               // >>=)
  BP_TERNARY,  // Ternary conditional operator (? :)
  BP_LOGICAL_OR,  // Logical OR operator (||)
  BP_LOGICAL_AND, // Logical AND operator (&&)
  BP_BITWISE_OR,  // Bitwise OR operator (|)
  BP_BITWISE_XOR, // Bitwise XOR operator (^)
  BP_BITWISE_AND, // Bitwise AND operator (&)
  BP_EQUALITY,    // Equality operators (==, !=)
  BP_RELATIONAL,  // Relational operators (<, >, <=, >=)
  BP_SHIFT,       // Shift operators (<<, >>)
  BP_SUM,         // Addition and subtraction (+, -)
  BP_PRODUCT,     // Multiplication, division, and modulo (*, /, %)
  BP_EXPONENT,    // Exponentiation operator (**)
  BP_UNARY,       // Unary operators (!, ~, +, -, ++prefix, --prefix)
  BP_POSTFIX,     // Postfix operators (++postfix, --postfix)
  BP_CALL,        // Function call or indexing
  BP_PRIMARY      // Primary expressions (literals, variables)
} BindingPower;

typedef struct {
  Token *tks;        // Pointer to the array of tokens
  Stmt *stmts;       // Pointer to the array of statements
  size_t tk_count;   // Number of tokens
  size_t stmt_count; // Number of statements
  size_t capacity;   // Capacity of the token and statement arrays
  size_t pos;
} Parser;

// Parser Utility Functions
bool p_has_tokens(Parser *psr);
Token p_peek(Parser *psr, size_t offset);
Token p_current(Parser *psr);
Token p_advance(Parser *psr);
Token p_consume(Parser *psr, TokenType type, const char *error_msg);

// Main parsing functions
Stmt *parse(GrowableArray *tks, ArenaAllocator *arena);
Expr *parse_expr(Parser *parser, BindingPower bp);
Stmt *parse_stmt(Parser *parser);
Type *parse_type(Parser *parser);

// Pratt parser functions
Expr *nud(Parser *parser);
Expr *led(Parser *parser, Expr *left, BindingPower bp);
BindingPower get_bp(TokenType kind);

// NUD (Null Denotation) functions
Expr *primary(Parser *parser);
Expr *unary(Parser *parser);
Expr *grouping(Parser *parser);

// LED (Left Denotation) functions
Expr *binary(Parser *parser, Expr *left, BindingPower bp);
Expr *call_expr(Parser *parser, Expr *left, BindingPower bp);
Expr *assign_expr(Parser *parser, Expr *left, BindingPower bp);
Expr *prefix_expr(Parser *parser, Expr *left, BindingPower bp);

// Type parsing functions
Type *tnud(Parser *parser);
Type *tled(Parser *parser, Type *left, BindingPower bp);
BindingPower tget_bp(Parser *parser, TokenType kind);

// Statement parsing functions
Stmt *expr_stmt(Parser *parser);
Stmt *var_stmt(Parser *parser);
Stmt *const_stmt(Parser *parser);
Stmt *print_stmt(Parser *parser);
Stmt *fn_stmt(Parser *parser, const char *name);
Stmt *enum_stmt(Parser *parser, const char *name);
Stmt *struct_stmt(Parser *parser, const char *name);
Stmt *block_stmt(Parser *parser);
Stmt *return_stmt(Parser *parser);
Stmt *loop_stmt(Parser *parser);
Stmt *if_stmt(Parser *parser);

// Parser creation and destruction
Parser *parser_create(ArenaAllocator *arena);
void parser_destroy(Parser *parser);
