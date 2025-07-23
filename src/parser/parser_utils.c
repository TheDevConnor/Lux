#include <stdio.h>

#include "parser.h"

bool p_has_tokens(Parser *psr) {
  return (psr->pos < psr->tk_count && psr->tks[psr->pos].type_ != TOK_EOF);
}

Token p_peek(Parser *psr, size_t offset) {
  return (psr->pos + offset < psr->tk_count) ? psr->tks[psr->pos + offset] 
                                             : (Token){.type_ = TOK_EOF}; // Return EOF token if out of bounds
}

Token p_current(Parser *psr) {
  return psr->pos < psr->tk_count ? psr->tks[psr->pos] 
                                  : (Token){.type_ = TOK_EOF}; // Return EOF token if no tokens left
}

Token p_advance(Parser *psr) {
  if (psr->pos < psr->tk_count) {
    return psr->tks[psr->pos++];
  } else {
    return (Token){.type_ = TOK_EOF}; // Return EOF token if no tokens left
  }
}

Token p_consume(Parser *psr, TokenType type, const char *error_msg) {
  if (p_current(psr).type_ == type) {
    return p_advance(psr);
  } else {
    fprintf(stderr, "Error at line %d, column %d: %s\n", 
            p_current(psr).line, p_current(psr).col, error_msg);
    return (Token){.type_ = TOK_EOF}; // Return an error token
  }
}
