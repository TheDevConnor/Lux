#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#include "lexer.h"

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

static const KeywordEntry keywords[] = {
    {"if", TOK_IF},         {"else", TOK_ELSE},
    {"loop", TOK_LOOP},     {"return", TOK_RETURN},
    {"break", TOK_BREAK},   {"continue", TOK_CONTINUE},
    {"struct", TOK_STRUCT}, {"enum", TOK_ENUM},
    {"mod", TOK_MOD},       {"import", TOK_IMPORT},
    {"true", TOK_TRUE},     {"false", TOK_FALSE},
    {"pub", TOK_PUBLIC},    {"private", TOK_PRIVATE},
    {"void", TOK_VOID},     {"char", TOK_CHAR},
    {"str", TOK_STRINGT},   {"int", TOK_INT},
    {"float", TOK_FLOAT},   {"double", TOK_DOUBLE},
    {"bool", TOK_BOOL},
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

Token make_token(TokenType type, const char *start, int line, int col, int length) {
  return (Token){type, start, line, col, length};
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
    return make_token(TOK_EOF, lx->current, 0, 0, 0);
  }

  const char *start = lx->current;
  char c = advance(lx);

  // Identifiers and keywords
  if (isalpha(c) || c == '_') {
    while (isalnum(peek(lx, 0)) || peek(lx, 0) == '_')
      advance(lx);
    int len = (int)(lx->current - start);
    TokenType type = lookup_keyword(start, len);
    return make_token(type, start, lx->line, lx->col - 1, len);
  }

  // Numbers
  if (isdigit(c)) {
    while (isdigit(peek(lx, 0)))
      advance(lx);
    return make_token(TOK_NUMBER, start, lx->line, lx->col - 1, (int)(lx->current - start));
  }

  // Strings
  if (c == '"') {
    while (!is_at_end(lx) && peek(lx, 0) != '"')
      advance(lx);
    if (!is_at_end(lx))
      advance(lx); // Skip closing quote
    return make_token(TOK_STRING, start + 1, lx->line, lx->col - 1, (int)(lx->current - start - 2));
  }

  // Try to match two-character symbol
  if (!is_at_end(lx)) {
    char two[3] = {start[0], peek(lx, 0), '\0'};
    TokenType ttype = lookup_symbol(two, 2);
    if (ttype != TOK_SYMBOL) {
      advance(lx);
      return make_token(ttype, start, lx->line, lx->col - 1, 2);
    }
  }

  // Fallback: single-character symbol
  TokenType single_type = lookup_symbol(start, 1);
  return make_token(single_type, start, lx->line, lx->col - 1, 1);
}