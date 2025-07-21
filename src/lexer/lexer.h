#pragma once

typedef enum {
  TOK_EOF,
  TOK_IDENTIFIER,
  TOK_KEYWORD,
  TOK_NUMBER,
  TOK_STRING,

  // Types (int, float, etc.)
  TOK_INT,     // int
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
  TOK_FN,       // fn
  TOK_RETURN,   // return
  TOK_BREAK,    // break
  TOK_CONTINUE, // continue
  TOK_STRUCT,   // struct
  TOK_ENUM,     // enum

  // Symbols
  TOK_SYMBOL,    // fallback
  TOK_LPAREN,    // (
  TOK_RPAREN,    // )
  TOK_LBRACE,    // {
  TOK_RBRACE,    // }
  TOK_LBRACKET,  // [
  TOK_RBRACKET,  // ]
  TOK_SEMICOLON, // ;
  TOK_COMMA,     // ,
  TOK_DOT,       // .
  TOK_EQUAL,     // =
  TOK_PLUS,      // +
  TOK_MINUS,     // -
  TOK_STAR,      // *
  TOK_SLASH,     // /
  TOK_LT,        // <
  TOK_GT,        // >
  TOK_LE,        // <=
  TOK_GE,        // >=
  TOK_EQEQ,      // ==
  TOK_NEQ,       // !=
  TOK_AMP,       // &
  TOK_PIPE,      // |
  TOK_CARET,     // ^
  TOK_TILDE,     // ~
  TOK_AND,       // &&
  TOK_OR,        // ||
  TOK_BANG,      // !
  TOK_QUESTION,  // ?
} TokenType;

typedef struct {
  TokenType type;
  const char *start; // Points into source buffer
  int length;        // Length of token
} Token;

typedef struct {
  const char *text;
  TokenType type;
} SymbolEntry;

typedef struct {
  const char *text;
  TokenType type;
} KeywordEntry;

/// Lexer state (input source + current position).
typedef struct {
  const char *src;     // Source buffer
  const char *current; // Current scan position
} Lexer;

void init_lexer(Lexer *lexer, const char *source);
Token next_token(Lexer *lexer);
