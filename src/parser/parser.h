/**
 * @file parser.h
 * @brief Recursive descent and Pratt parser for the Zura language.
 *
 * This module implements parsing of tokens into an Abstract Syntax Tree (AST),
 * handling expressions, statements, types, and error reporting.
 * The parser uses binding power (precedence) to correctly parse expressions.
 *
 * The parser supports:
 * - Parsing expressions with Pratt parsing techniques (nud/led functions).
 * - Parsing statements including variable declarations, functions, loops, etc.
 * - Parsing types including pointers and arrays.
 * - Error reporting with detailed location info.
 *
 * @author Connor Harris
 * @date 2025
 */

#pragma once

#include <stddef.h>

#include "../ast/ast.h"
#include "../c_libs/memory/memory.h"
#include "../lexer/lexer.h"

/**
 * @def CURRENT_TOKEN_LENGTH(parser)
 * @brief Get the length of the current token's lexeme.
 */
#define CURRENT_TOKEN_LENGTH(parser) ((int)p_current(parser).length)

/**
 * @def CURRENT_TOKEN_VALUE(parser)
 * @brief Get the value string of the current token.
 */
#define CURRENT_TOKEN_VALUE(parser) (p_current(parser).value)

/**
 * @brief Maximum allowed size for statements, expressions, and types.
 */
#define MAX_STMT 1024
#define MAX_EXPR 1024
#define MAX_TYPE 1024

/**
 * @enum BindingPower
 * @brief Binding power (precedence) levels for expression parsing.
 *
 * Used to control operator precedence and associativity in Pratt parsing.
 */
typedef enum {
  BP_NONE = 0,       /**< No binding power */
  BP_LOWEST,         /**< Lowest binding power */
  BP_ASSIGN,         /**< Assignment operators (=, +=, etc.) */
  BP_TERNARY,        /**< Ternary conditional operator (? :) */
  BP_LOGICAL_OR,     /**< Logical OR operator (||) */
  BP_LOGICAL_AND,    /**< Logical AND operator (&&) */
  BP_BITWISE_OR,     /**< Bitwise OR operator (|) */
  BP_BITWISE_XOR,    /**< Bitwise XOR operator (^) */
  BP_BITWISE_AND,    /**< Bitwise AND operator (&) */
  BP_EQUALITY,       /**< Equality operators (==, !=) */
  BP_RELATIONAL,     /**< Relational operators (<, >, <=, >=) */
  BP_SHIFT,          /**< Shift operators (<<, >>) */
  BP_SUM,            /**< Addition and subtraction (+, -) */
  BP_PRODUCT,        /**< Multiplication, division, modulo (*, /, %) */
  BP_EXPONENT,       /**< Exponentiation operator (**) */
  BP_UNARY,          /**< Unary operators (!, ~, +, -, prefix ++/--) */
  BP_POSTFIX,        /**< Postfix operators (++/-- postfix) */
  BP_CALL,           /**< Function call or indexing */
  BP_PRIMARY         /**< Primary expressions (literals, variables) */
} BindingPower;

/**
 * @brief Maps token types to their corresponding literal types for primary expressions.
 */
static const LiteralType PRIMARY_LITERAL_TYPE_MAP[] = {
  [TOK_NUMBER]      = LITERAL_INT,
  [TOK_STRING]      = LITERAL_STRING,
  [TOK_CHAR_LITERAL] = LITERAL_CHAR,
  [TOK_TRUE]       = LITERAL_BOOL,
  [TOK_FALSE]      = LITERAL_BOOL,
  [TOK_IDENTIFIER] = LITERAL_IDENT,
};

/**
 * @brief Maps token types to their corresponding binary operators.
 */
static const BinaryOp TOKEN_TO_BINOP_MAP[] = {
  [TOK_PLUS]  = BINOP_ADD,
  [TOK_MINUS] = BINOP_SUB,
  [TOK_STAR]  = BINOP_MUL,
  [TOK_SLASH] = BINOP_DIV,
  [TOK_EQEQ]  = BINOP_EQ,
  [TOK_NEQ]   = BINOP_NE,
  [TOK_LT]    = BINOP_LT,
  [TOK_LE]    = BINOP_LE,
  [TOK_GT]    = BINOP_GT,
  [TOK_GE]    = BINOP_GE,
  [TOK_AND]   = BINOP_AND,
  [TOK_OR]    = BINOP_OR,
  [TOK_AMP]   = BINOP_BIT_AND,
  [TOK_PIPE]  = BINOP_BIT_OR,
  [TOK_CARET] = BINOP_BIT_XOR,
};

/**
 * @brief Maps token types to their corresponding unary operators.
 */
static const UnaryOp TOKEN_TO_UNOP_MAP[] = {
  [TOK_BANG]       = UNOP_NOT,
  [TOK_TILDE]      = UNOP_BIT_NOT,
  [TOK_PLUS]       = UNOP_POS,
  [TOK_MINUS]      = UNOP_NEG,
  [TOK_PLUSPLUS]   = UNOP_PRE_INC,
  [TOK_MINUSMINUS] = UNOP_PRE_DEC,
};

/**
 * @struct Parser
 * @brief Parser state holding token stream and current position.
 */
typedef struct {
  ArenaAllocator *arena; /**< Memory arena for AST node allocations */
  Token *tks;            /**< Array of tokens to parse */
  size_t tk_count;       /**< Number of tokens in the array */
  size_t capacity;       /**< Capacity for statements and expressions */
  size_t pos;            /**< Current token position */
} Parser;

/**
 * @brief Report a parser error with detailed location info.
 *
 * @param psr Pointer to the parser.
 * @param error_type Type/category of the error.
 * @param file Source file where error occurred.
 * @param msg Error message.
 * @param line Line number of error.
 * @param col Column number of error.
 * @param tk_length Length of the token causing the error.
 */
void parser_error(Parser *psr, const char *error_type, const char *file,
                  const char *msg, int line, int col, int tk_length);

/**
 * @brief Checks if the parser has more tokens to process.
 *
 * @param psr Parser pointer.
 * @return true if more tokens are available, false otherwise.
 */
bool p_has_tokens(Parser *psr);

/**
 * @brief Peeks at a token at a given offset from the current position.
 *
 * @param psr Parser pointer.
 * @param offset Offset from current token.
 * @return The token at the given offset.
 */
Token p_peek(Parser *psr, size_t offset);

/**
 * @brief Returns the current token the parser is looking at.
 *
 * @param psr Parser pointer.
 * @return The current token.
 */
Token p_current(Parser *psr);

/**
 * @brief Advances the parser to the next token.
 *
 * @param psr Parser pointer.
 * @return The token advanced past.
 */
Token p_advance(Parser *psr);

/**
 * @brief Consume a token of the specified type, or report an error.
 *
 * @param psr Parser pointer.
 * @param type Expected token type.
 * @param error_msg Error message if token is not the expected type.
 * @return The consumed token.
 */
Token p_consume(Parser *psr, TokenType type, const char *error_msg);

/**
 * @brief Retrieves the name string from the current token (for identifiers).
 *
 * @param psr Parser pointer.
 * @return Pointer to the token's string value.
 */
char *get_name(Parser *psr);

/**
 * @brief Parses a full program from tokens into an AST of statements.
 *
 * @param tks GrowableArray containing tokens.
 * @param arena Memory arena for allocations.
 * @return Pointer to the root AST statement node (program).
 */
Stmt *parse(GrowableArray *tks, ArenaAllocator *arena);

/**
 * @brief Parses an expression with a given minimum binding power.
 *
 * @param parser Parser pointer.
 * @param bp Minimum binding power (precedence).
 * @return Parsed expression node.
 */
Expr *parse_expr(Parser *parser, BindingPower bp);

/**
 * @brief Parses a single statement.
 *
 * @param parser Parser pointer.
 * @return Parsed statement node.
 */
Stmt *parse_stmt(Parser *parser);

/**
 * @brief Parses a type expression.
 *
 * @param parser Parser pointer.
 * @return Parsed type node.
 */
Type *parse_type(Parser *parser);

/**
 * @brief Pratt parser function for null denotation (prefix parsing).
 *
 * @param parser Parser pointer.
 * @return Parsed expression node.
 */
Expr *nud(Parser *parser);

/**
 * @brief Pratt parser function for left denotation (infix/postfix parsing).
 *
 * @param parser Parser pointer.
 * @param left Left expression.
 * @param bp Binding power.
 * @return Parsed expression node.
 */
Expr *led(Parser *parser, Expr *left, BindingPower bp);

/**
 * @brief Gets the binding power (precedence) of a token type.
 *
 * @param kind Token type.
 * @return Binding power enum value.
 */
BindingPower get_bp(TokenType kind);

/**
 * @brief Parses a primary expression (literal, identifier).
 *
 * @param parser Parser pointer.
 * @return Parsed expression node.
 */
Expr *primary(Parser *parser);

/**
 * @brief Parses a unary expression.
 *
 * @param parser Parser pointer.
 * @return Parsed expression node.
 */
Expr *unary(Parser *parser);

/**
 * @brief Parses a grouped expression (parentheses).
 *
 * @param parser Parser pointer.
 * @return Parsed expression node.
 */
Expr *grouping(Parser *parser);

/**
 * @brief Parses a binary expression.
 *
 * @param parser Parser pointer.
 * @param left Left expression node.
 * @param bp Binding power.
 * @return Parsed expression node.
 */
Expr *binary(Parser *parser, Expr *left, BindingPower bp);

/**
 * @brief Parses a function call expression.
 *
 * @param parser Parser pointer.
 * @param left Left expression node.
 * @param bp Binding power.
 * @return Parsed call expression node.
 */
Expr *call_expr(Parser *parser, Expr *left, BindingPower bp);

/**
 * @brief Parses an assignment expression.
 *
 * @param parser Parser pointer.
 * @param left Left expression node.
 * @param bp Binding power.
 * @return Parsed assignment expression node.
 */
Expr *assign_expr(Parser *parser, Expr *left, BindingPower bp);

/**
 * @brief Parses a prefix expression (member access, postfix operators, indexing).
 *
 * @param parser Parser pointer.
 * @param left Left expression node.
 * @param bp Binding power.
 * @return Parsed prefix expression node.
 */
Expr *prefix_expr(Parser *parser, Expr *left, BindingPower bp);

/**
 * @brief Parses an array literal expression.
 *
 * @param parser Parser pointer.
 * @return Parsed array expression node.
 */
Expr *array_expr(Parser *parser);

/**
 * @brief Type parsing null denotation function.
 *
 * @param parser Parser pointer.
 * @return Parsed type node.
 */
Type *tnud(Parser *parser);

/**
 * @brief Type parsing left denotation function.
 *
 * @param parser Parser pointer.
 * @param left Left type node.
 * @param bp Binding power.
 * @return Parsed type node.
 */
Type *tled(Parser *parser, Type *left, BindingPower bp);

/**
 * @brief Gets the binding power for a type token.
 *
 * @param parser Parser pointer.
 * @param kind Token type.
 * @return Binding power.
 */
BindingPower tget_bp(Parser *parser, TokenType kind);

/**
 * @brief Parses a pointer type.
 *
 * @param parser Parser pointer.
 * @return Parsed pointer type node.
 */
Type *pointer(Parser *parser);

/**
 * @brief Parses an array type.
 *
 * @param parser Parser pointer.
 * @return Parsed array type node.
 */
Type *array_type(Parser *parser);

/**
 * @brief Parses an expression statement.
 *
 * @param parser Parser pointer.
 * @return Parsed statement node.
 */
Stmt *expr_stmt(Parser *parser);

/**
 * @brief Parses a variable declaration statement.
 *
 * @param parser Parser pointer.
 * @param is_public Whether the variable is public.
 * @return Parsed statement node.
 */
Stmt *var_stmt(Parser *parser, bool is_public);

/**
 * @brief Parses a constant declaration statement.
 *
 * @param parser Parser pointer.
 * @param is_public Whether the constant is public.
 * @return Parsed statement node.
 */
Stmt *const_stmt(Parser *parser, bool is_public);

/**
 * @brief Parses a function declaration statement.
 *
 * @param parser Parser pointer.
 * @param name Function name.
 * @param is_public Whether the function is public.
 * @return Parsed statement node.
 */
Stmt *fn_stmt(Parser *parser, const char *name, bool is_public);

/**
 * @brief Parses an enum declaration statement.
 *
 * @param parser Parser pointer.
 * @param name Enum name.
 * @param is_public Whether the enum is public.
 * @return Parsed statement node.
 */
Stmt *enum_stmt(Parser *parser, const char *name, bool is_public);

/**
 * @brief Parses a struct declaration statement.
 *
 * @param parser Parser pointer.
 * @param name Struct name.
 * @param is_public Whether the struct is public.
 * @return Parsed statement node.
 */
Stmt *struct_stmt(Parser *parser, const char *name, bool is_public);

/**
 * @brief Parses a print statement.
 *
 * @param parser Parser pointer.
 * @param ln Whether to append a newline.
 * @return Parsed statement node.
 */
Stmt *print_stmt(Parser *parser, bool ln);

/**
 * @brief Parses a return statement.
 *
 * @param parser Parser pointer.
 * @return Parsed statement node.
 */
Stmt *return_stmt(Parser *parser);

/**
 * @brief Parses a block statement.
 *
 * @param parser Parser pointer.
 * @return Parsed statement node.
 */
Stmt *block_stmt(Parser *parser);

/**
 * @brief Parses an infinite loop statement.
 *
 * @param parser Parser pointer.
 * @param line Line number for error reporting.
 * @param col Column number for error reporting.
 * @return Parsed statement node.
 */
Stmt *infinite_loop_stmt(Parser *parser, int line, int col);

/**
 * @brief Parses a for loop statement.
 *
 * @param parser Parser pointer.
 * @param line Line number for error reporting.
 * @param col Column number for error reporting.
 * @return Parsed statement node.
 */
Stmt *for_loop_stmt(Parser *parser, int line, int col);

/**
 * @brief Parses a loop statement.
 *
 * @param parser Parser pointer.
 * @return Parsed statement node.
 */
Stmt *loop_stmt(Parser *parser);

/**
 * @brief Parses an if statement.
 *
 * @param parser Parser pointer.
 * @return Parsed statement node.
 */
Stmt *if_stmt(Parser *parser);

/**
 * @brief Parses a break or continue statement.
 *
 * @param parser Parser pointer.
 * @param is_continue true for continue, false for break.
 * @return Parsed statement node.
 */
Stmt *break_continue_stmt(Parser *parser, bool is_continue);
