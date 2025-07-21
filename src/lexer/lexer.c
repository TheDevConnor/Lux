#include "lexer.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static const KeywordEntry keywords[] = {
    {"int", TOK_INT},
    {"uint", TOK_UINT},
    {"float", TOK_FLOAT},
    {"bool", TOK_BOOL},
    {"str", TOK_STRINGT},
    {"char", TOK_CHAR},
    {"void", TOK_VOID},

    {"if", TOK_IF},
    {"else", TOK_ELSE},
    {"loop", TOK_LOOP},
    {"fn", TOK_FN},
    {"return", TOK_RETURN},
    {"break", TOK_BREAK},
    {"continue", TOK_CONTINUE},
    {"struct", TOK_STRUCT},
    {"enum", TOK_ENUM},
};

static const SymbolEntry symbols[] = {
    {"(", TOK_LPAREN},    {")", TOK_RPAREN},   {"{", TOK_LBRACE},
    {"}", TOK_RBRACE},    {"[", TOK_LBRACKET}, {"]", TOK_RBRACKET},
    {";", TOK_SEMICOLON}, {",", TOK_COMMA},    {".", TOK_DOT},
    {"==", TOK_EQEQ},     {"!=", TOK_NEQ},     {"<=", TOK_LE},
    {">=", TOK_GE},       {"&&", TOK_AND},     {"||", TOK_OR},
    {"=", TOK_EQUAL},     {"+", TOK_PLUS},     {"-", TOK_MINUS},
    {"*", TOK_STAR},      {"/", TOK_SLASH},    {"<", TOK_LT},
    {">", TOK_GT},        {"&", TOK_AMP},      {"|", TOK_PIPE},
    {"^", TOK_CARET},     {"~", TOK_TILDE},    {"!", TOK_BANG},
    {"?", TOK_QUESTION},
};

static TokenType lookup_keyword(const char *str, int length) {
  for (int i = 0; i < (int)(sizeof(keywords) / sizeof(*keywords)); ++i) {
    if (strncmp(str, keywords[i].text, length) == 0 &&
        keywords[i].text[length] == '\0') {
      return keywords[i].type;
    }
  }
  return TOK_IDENTIFIER;
}

static TokenType lookup_symbol(const char *str, int length) {
  for (int i = 0; i < (int)(sizeof(symbols) / sizeof(*symbols)); ++i) {
    if (strncmp(str, symbols[i].text, length) == 0 &&
        symbols[i].text[length] == '\0') {
      return symbols[i].type;
    }
  }
  return TOK_SYMBOL;
}

void init_lexer(Lexer *lexer, const char *source) {
  lexer->src = source;
  lexer->current = source;
}

char peek(Lexer *lx, int offset) { return lx->current[offset]; }
char advance(Lexer *lx) { return *lx->current++; }
bool is_at_end(Lexer *lx) { return *lx->current == '\0'; }

Token make_token(TokenType type, const char *start, int length) {
  return (Token){type, start, length};
}

void skip_whitespace(Lexer *lx) {
  while (!is_at_end(lx)) {
    char c = peek(lx, 0);
    if (isspace(c)) {
      advance(lx);
    } else if (c == '/' && peek(lx, 1) == '/') {
      // Skip single-line comment
      while (!is_at_end(lx) && peek(lx, 0) != '\n') {
        advance(lx);
      }
    } else if (c == '/' && peek(lx, 1) == '*') {
      // Skip multi-line comment
      advance(lx); // Skip '/'
      advance(lx); // Skip '*'
      while (!is_at_end(lx) && !(peek(lx, 0) == '*' && peek(lx, 1) == '/')) {
        advance(lx);
      }
      if (!is_at_end(lx)) {
        advance(lx); // Skip '*'
        advance(lx); // Skip '/'
      }
    } else {
      break;
    }
  }
}

Token next_token(Lexer *lx) {
  skip_whitespace(lx);
  if (is_at_end(lx)) {
    return make_token(TOK_EOF, lx->current, 0);
  }

  const char *start = lx->current;
  char c = advance(lx);

  // Identifiers and keywords
  if (isalpha(c) || c == '_') {
    while (isalnum(peek(lx, 0)) || peek(lx, 0) == '_')
      advance(lx);
    int len = (int)(lx->current - start);
    TokenType type = lookup_keyword(start, len);
    return make_token(type, start, len);
  }

  // Numbers
  if (isdigit(c)) {
    while (isdigit(peek(lx, 0)))
      advance(lx);
    return make_token(TOK_NUMBER, start, (int)(lx->current - start));
  }

  // Strings
  if (c == '"') {
    while (!is_at_end(lx) && peek(lx, 0) != '"')
      advance(lx);
    if (!is_at_end(lx))
      advance(lx); // Skip closing quote
    return make_token(TOK_STRING, start + 1, (int)(lx->current - start - 2));
  }

  // Try to match two-character symbol
  if (!is_at_end(lx)) {
    char two[3] = {start[0], peek(lx, 0), '\0'};
    TokenType ttype = lookup_symbol(two, 2);
    if (ttype != TOK_SYMBOL) {
      advance(lx);
      return make_token(ttype, start, 2);
    }
  }

  // Fallback: single-character symbol
  TokenType single_type = lookup_symbol(start, 1);
  return make_token(single_type, start, 1);
}
