#include <stdio.h>

#include "parser.h"

bool p_has_tokens(Parser *psr) {
  return psr->pos < psr->tk_count;
}

Token p_peek(Parser *psr, size_t offset) {
  if (psr->pos + offset < psr->tk_count) {
    return psr->tks[psr->pos + offset];
  }
  return (Token){.type_ = TOK_EOF}; // Return EOF token if out of bounds
}

Token p_current(Parser *psr) {
  if (p_has_tokens(psr)) {
    return psr->tks[psr->pos];
  }
  return (Token){.type_ = TOK_EOF}; // Return EOF token if no tokens left
}

Token p_advance(Parser *psr) {
  if (p_has_tokens(psr)) {
    return psr->tks[psr->pos++];
  }
  return (Token){.type_ = TOK_EOF}; // Return EOF token if no tokens left
}

Token p_consume(Parser *psr, TokenType type, const char *error_msg) {
  if (p_has_tokens(psr) && p_current(psr).type_ == type) {
    return p_advance(psr);
  }
  fprintf(stderr, "Error: %s at line %zu, column %zu\n", error_msg, (size_t)p_current(psr).line, (size_t)p_current(psr).col);
  return (Token){.type_ = TOK_EOF}; // Return EOF token on error
}
