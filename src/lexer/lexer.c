#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../c_libs/error/error.h"
#include "../c_libs/memory/memory.h"
#include "lexer.h"

#define STR_EQUALS_LEN(str, key, len)                                          \
  (strncmp(str, key, len) == 0 && key[len] == '\0')
#define MATCH_NEXT(lx, a, b) (peek(lx, 0) == (a) && peek(lx, 1) == (b))
#define MAKE_TOKEN(type, start, lx, length, whitespace_len)                    \
  make_token(type, start, lx->line, lx->col - 1, length, whitespace_len)

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
    {"priv", TOK_PRIVATE},
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

void report_lexer_error(Lexer *lx, const char *error_type, const char *file,
                        const char *msg, const char *line_text, int line,
                        int col, int tk_length) {
  ErrorInformation err = {
      .error_type = error_type,
      .file_path = file,
      .message = msg,
      .line = line,
      .col = col,
      .line_text = arena_strdup(lx->arena, line_text),
      .token_length = tk_length,
      .label = "Undefined Token",
      .note = NULL,
      .help = NULL,
  };
  error_add(err);
}

const char *get_line_text_from_source(const char *source, int target_line) {
  static char line_buffer[1024];
  const char *start = source; // Always start from beginning of source
  int current_line = 1;

  // Skip to the beginning of the target line
  while (current_line < target_line && *start != '\0') {
    if (*start == '\n') {
      current_line++;
    }
    start++;
  }

  if (current_line != target_line) {
    line_buffer[0] = '\0';
    return line_buffer;
  }

  // Extract the complete line
  int i = 0;
  while (*start != '\0' && *start != '\n' && i < (int)sizeof(line_buffer) - 1) {
    line_buffer[i++] = *start++;
  }
  line_buffer[i] = '\0';

  return line_buffer;
}

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

void init_lexer(Lexer *lexer, const char *source, ArenaAllocator *arena) {
  lexer->arena = arena;
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
                 int length, int whitespace_len) {
  return (Token){type, start, line, col, length, whitespace_len};
}

int skip_multiline_comment(Lexer *lx) {
  int count = 0;
  advance(lx); // skip '/'
  advance(lx); // skip '*'
  count += 2;
  while (!is_at_end(lx) && !MATCH_NEXT(lx, '*', '/')) {
    advance(lx);
    count++;
  }
  if (!is_at_end(lx)) {
    advance(lx); // skip '*'
    advance(lx); // skip '/'
  }
  return count + 2;
}

int skip_whitespace(Lexer *lx) {
  int count = 0;
  while (!is_at_end(lx)) {
    char c = peek(lx, 0);
    if (isspace(c)) {
      advance(lx);
      count++;
    } else if (c == ';' && peek(lx, 1) == ';') {
      // Skip single-line comment
      while (!is_at_end(lx) && peek(lx, 0) != '\n') {
        advance(lx);
        count++;
      }
    } else if (c == '/' && peek(lx, 1) == '*') {
      count = skip_multiline_comment(lx);
    } else {
      break;
    }
  }

  return count;
}

Token next_token(Lexer *lx) {
  int wh_count = skip_whitespace(lx);
  if (is_at_end(lx)) {
    return MAKE_TOKEN(TOK_EOF, lx->current, lx, 0, wh_count);
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
    return MAKE_TOKEN(type, start, lx, len, wh_count);
  }

  // Numbers
  if (isdigit(c)) {
    while (isdigit(peek(lx, 0))) {
      advance(lx);
    }
    int len = (int)(lx->current - start);
    return MAKE_TOKEN(TOK_NUMBER, start, lx, len, wh_count);
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
    return MAKE_TOKEN(TOK_STRING, start + 1, lx, len, wh_count);
  }

  // Try to match two-character symbol
  if (!is_at_end(lx)) {
    char two[3] = {start[0], peek(lx, 0), '\0'};
    TokenType ttype = lookup_symbol(two, 2);
    if (ttype != TOK_SYMBOL) {
      advance(lx);
      return MAKE_TOKEN(ttype, start, lx, 2, wh_count);
    }
  }

  // Fallback: single-character symbol
  TokenType single_type = lookup_symbol(start, 1);
  if (single_type != TOK_SYMBOL)
    return MAKE_TOKEN(single_type, start, lx, 1, wh_count);

  static char error_msg[64];
  snprintf(error_msg, sizeof(error_msg), "Token not found: '%c'", c);
  report_lexer_error(lx, "LexerError", "unknown_file", error_msg,
                     get_line_text_from_source(lx->src, lx->line), lx->line,
                     lx->col, 1);
  return MAKE_TOKEN(TOK_ERROR, start, lx, 1, wh_count);
}
