#pragma once
#include <stdint.h>

typedef enum {
  TOK_EOF,
  TOK_IDENTIFIER,
  TOK_KEYWORD,
  TOK_NUMBER,
  TOK_STRING,
  TOK_CHAR_LITERAL,

  // Types (int, float, etc.)
  TOK_INT,     // int
  TOK_DOUBLE,  // double
  TOK_UINT,    // unsigned int
  TOK_FLOAT,   // float
  TOK_BOOL,    // bool
  TOK_STRINGT, // str
  TOK_VOID,    // void
  TOK_CHAR,    // char

  // Keywords
  TOK_IF,       // if
  TOK_ELSE,     // else
  TOK_LOOP,     // loop
  TOK_RETURN,   // return
  TOK_BREAK,    // break
  TOK_CONTINUE, // continue
  TOK_STRUCT,   // struct
  TOK_ENUM,     // enum
  TOK_MOD,      // mod
  TOK_IMPORT,   // import
  TOK_TRUE,     // true
  TOK_FALSE,    // false
  TOK_PUBLIC,   // pub
  TOK_PRIVATE,  // private
  TOK_VAR,      // let
  TOK_FN,       // fn
  TOK_PRINT,    // output
  TOK_PRINTLN,  // println

  // Symbols
  TOK_SYMBOL,      // fallback
  TOK_LPAREN,      // (
  TOK_RPAREN,      // )
  TOK_LBRACE,      // {
  TOK_RBRACE,      // }
  TOK_LBRACKET,    // [
  TOK_RBRACKET,    // ]
  TOK_SEMICOLON,   // ;
  TOK_COMMA,       // ,
  TOK_DOT,         // .
  TOK_EQUAL,       // =
  TOK_PLUS,        // +
  TOK_MINUS,       // -
  TOK_STAR,        // *
  TOK_SLASH,       // /
  TOK_LT,          // <
  TOK_GT,          // >
  TOK_LE,          // <=
  TOK_GE,          // >=
  TOK_EQEQ,        // ==
  TOK_NEQ,         // !=
  TOK_AMP,         // &
  TOK_PIPE,        // |
  TOK_CARET,       // ^
  TOK_TILDE,       // ~
  TOK_AND,         // &&
  TOK_OR,          // ||
  TOK_RESOLVE,     // ::
  TOK_COLON,       // :
  TOK_BANG,        // !
  TOK_QUESTION,    // ?
  TOK_PLUSPLUS,    // ++
  TOK_MINUSMINUS,  // --
  TOK_SHIFT_LEFT,  // <<
  TOK_SHIFT_RIGHT, // >>
  TOK_WHITESPACE,  // whitespace
  TOK_COMMENT,     // comment
} TokenType;

typedef struct {
  const char *src;
  const char *current;
  int line, col;
} Lexer;

typedef struct {
  TokenType type_;
  const char *value;
  int line, col, length;
} Token;

typedef struct {
  const char *text;
  TokenType type;
} SymbolEntry;

typedef struct {
  const char *text;
  TokenType type;
} KeywordEntry;

void init_lexer(Lexer *lexer, const char *source);
Token next_token(Lexer *lexer);
