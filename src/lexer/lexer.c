#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#include "lexer.h"

#define STR_EQUALS_LEN(str, key, len)                                          \
  (strncmp(str, key, len) == 0 && key[len] == '\0')
#define MATCH_NEXT(lx, a, b) (peek(lx, 0) == (a) && peek(lx, 1) == (b))
#define MAKE_TOKEN(type, start, lx, length)                                    \
  make_token(type, start, lx->line, lx->col - 1, length)

static const SymbolEntry symbols[] = {
    {"(", TOK_LPAREN},      {")", TOK_RPAREN},       {"{", TOK_LBRACE},
    {"}", TOK_RBRACE},      {"[", TOK_LBRACKET},     {"]", TOK_RBRACKET},
    {";", TOK_SEMICOLON},   {",", TOK_COMMA},        {".", TOK_DOT},
    {"==", TOK_EQEQ},       {"!=", TOK_NEQ},         {"<=", TOK_LE},
    {">=", TOK_GE},         {"&&", TOK_AND},         {"||", TOK_OR},
    {"=", TOK_EQUAL},       {"+", TOK_PLUS},         {"-", TOK_MINUS},
    {"*", TOK_STAR},        {"/", TOK_SLASH},        {"<", TOK_LT},
    {">", TOK_GT},          {"&", TOK_AMP},          {"|", TOK_PIPE},
    {"^", TOK_CARET},       {"~", TOK_TILDE},        {"!", TOK_BANG},
    {"?", TOK_QUESTION},    {"::", TOK_RESOLVE},     {":", TOK_COLON},
    {"_", TOK_SYMBOL},      {"++", TOK_PLUSPLUS},    {"--", TOK_MINUSMINUS},
    {"<<", TOK_SHIFT_LEFT}, {">>", TOK_SHIFT_RIGHT},
};

static const KeywordEntry keywords[] = {
    {"if", TOK_IF},
    {"else", TOK_ELSE},
    {"elif", TOK_ELIF},
    {"loop", TOK_LOOP},
    {"return", TOK_RETURN},
    {"break", TOK_BREAK},
    {"continue", TOK_CONTINUE},
    {"struct", TOK_STRUCT},
    {"enum", TOK_ENUM},
    {"mod", TOK_MOD},
    {"import", TOK_IMPORT},
    {"true", TOK_TRUE},
    {"false", TOK_FALSE},
    {"pub", TOK_PUBLIC},
    {"private", TOK_PRIVATE},
    {"void", TOK_VOID},
    {"char", TOK_CHAR},
    {"str", TOK_STRINGT},
    {"int", TOK_INT},
    {"float", TOK_FLOAT},
    {"double", TOK_DOUBLE},
    {"bool", TOK_BOOL},
    {"let", TOK_VAR},
    {"fn", TOK_FN},
    {"output", TOK_PRINT},
    {"outputln", TOK_PRINTLN},
    {"const", TOK_CONST},
};

static TokenType lookup_keyword(const char *str, int length) {
  for (int i = 0; i < (int)(sizeof(keywords) / sizeof(*keywords)); ++i) {
    if (STR_EQUALS_LEN(str, keywords[i].text, length)) {
      return keywords[i].type;
    }
  }
  return TOK_IDENTIFIER;
}

static TokenType lookup_symbol(const char *str, int length) {
  for (int i = 0; i < (int)(sizeof(symbols) / sizeof(*symbols)); ++i) {
    if (STR_EQUALS_LEN(str, symbols[i].text, length)) {
      return symbols[i].type;
    }
  }
  return TOK_SYMBOL;
}

void init_lexer(Lexer *lexer, const char *source) {
  lexer->src = source;
  lexer->current = source;
  lexer->line = 1;
  lexer->col = 0;
}

char peek(Lexer *lx, int offset) { return lx->current[offset]; }
bool is_at_end(Lexer *lx) { return *lx->current == '\0'; }
char advance(Lexer *lx) {
  char c = *lx->current++;
  if (c == '\n') {
    lx->line++;
    lx->col = 0;
  } else if (c != '\0') {
    lx->col++;
  }
  return c;
}

Token make_token(TokenType type, const char *start, int line, int col,
                 int length) {
  return (Token){type, start, line, col, length};
}

void skip_multiline_comment(Lexer *lx) {
  advance(lx); // skip '/'
  advance(lx); // skip '*'
  while (!is_at_end(lx) && !MATCH_NEXT(lx, '*', '/')) {
    advance(lx);
  }
  if (!is_at_end(lx)) {
    advance(lx); // skip '*'
    advance(lx); // skip '/'
  }
}

void skip_whitespace(Lexer *lx) {
  while (!is_at_end(lx)) {
    char c = peek(lx, 0);
    if (isspace(c)) {
      advance(lx);
    } else if (c == ';' && peek(lx, 1) == ';') {
      // Skip single-line comment
      while (!is_at_end(lx) && peek(lx, 0) != '\n') {
        advance(lx);
      }
    } else if (c == '/' && peek(lx, 1) == '*') {
      skip_multiline_comment(lx);
    } else {
      break;
    }
  }
}

Token next_token(Lexer *lx) {
  skip_whitespace(lx);
  if (is_at_end(lx)) {
    return MAKE_TOKEN(TOK_EOF, lx->current, lx, 0);
  }

  const char *start = lx->current;
  char c = advance(lx);

  // Identifiers and keywords
  if (isalpha(c) || c == '_') {
    while (isalnum(peek(lx, 0)) || peek(lx, 0) == '_') {
      advance(lx);
    }
    int len = (int)(lx->current - start);
    TokenType type = lookup_keyword(start, len);
    return MAKE_TOKEN(type, start, lx, len);
  }

  // Numbers
  if (isdigit(c)) {
    while (isdigit(peek(lx, 0))) {
      advance(lx);
    }
    int len = (int)(lx->current - start);
    return MAKE_TOKEN(TOK_NUMBER, start, lx, len);
  }

  // Strings
  if (c == '"') {
    while (!is_at_end(lx) && peek(lx, 0) != '"') {
      advance(lx);
    }
    if (!is_at_end(lx)) {
      advance(lx); // Skip closing quote
    }
    int len = (int)(lx->current - start - 2);
    return MAKE_TOKEN(TOK_STRING, start + 1, lx, len);
  }

  // Try to match two-character symbol
  if (!is_at_end(lx)) {
    char two[3] = {start[0], peek(lx, 0), '\0'};
    TokenType ttype = lookup_symbol(two, 2);
    if (ttype != TOK_SYMBOL) {
      advance(lx);
      return MAKE_TOKEN(ttype, start, lx, 2);
    }
  }

  // Fallback: single-character symbol
  TokenType single_type = lookup_symbol(start, 1);
  return MAKE_TOKEN(single_type, start, lx, 1);
}
