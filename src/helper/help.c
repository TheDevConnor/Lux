#include "help.h"
#include "../lexer/lexer.h"
#include <stdio.h>

void print_token(const Token *t) {
  printf("%.*s -> ", (int)t->length, t->start);

  switch (t->type) {
  case TOK_EOF:
    puts("EOF");
    break;
  case TOK_IDENTIFIER:
    puts("IDENTIFIER");
    break;
  case TOK_KEYWORD:
    puts("KEYWORD");
    break;
  case TOK_NUMBER:
    puts("NUMBER");
    break;
  case TOK_STRING:
    puts("STRING");
    break;

  case TOK_INT:
    puts("INT");
    break;
  case TOK_UINT:
    puts("UINT");
    break;
  case TOK_FLOAT:
    puts("FLOAT");
    break;
  case TOK_BOOL:
    puts("BOOL");
    break;
  case TOK_STRINGT:
    puts("STRINGT");
    break;
  case TOK_VOID:
    puts("VOID");
    break;
  case TOK_CHAR:
    puts("CHAR");
    break;

  case TOK_IF:
    puts("IF");
    break;
  case TOK_ELSE:
    puts("ELSE");
    break;
  case TOK_LOOP:
    puts("LOOP");
    break;
  case TOK_FN:
    puts("FN");
    break;
  case TOK_RETURN:
    puts("RETURN");
    break;
  case TOK_BREAK:
    puts("BREAK");
    break;
  case TOK_CONTINUE:
    puts("CONTINUE");
    break;
  case TOK_STRUCT:
    puts("STRUCT");
    break;
  case TOK_ENUM:
    puts("ENUM");
    break;

  case TOK_SYMBOL:
    puts("SYMBOL");
    break;
  case TOK_LPAREN:
    puts("LPAREN");
    break;
  case TOK_RPAREN:
    puts("RPAREN");
    break;
  case TOK_LBRACE:
    puts("LBRACE");
    break;
  case TOK_RBRACE:
    puts("RBRACE");
    break;
  case TOK_LBRACKET:
    puts("LBRACKET");
    break;
  case TOK_RBRACKET:
    puts("RBRACKET");
    break;
  case TOK_SEMICOLON:
    puts("SEMICOLON");
    break;
  case TOK_COMMA:
    puts("COMMA");
    break;
  case TOK_DOT:
    puts("DOT");
    break;
  case TOK_EQUAL:
    puts("EQUAL");
    break;
  case TOK_PLUS:
    puts("PLUS");
    break;
  case TOK_MINUS:
    puts("MINUS");
    break;
  case TOK_STAR:
    puts("STAR");
    break;
  case TOK_SLASH:
    puts("SLASH");
    break;
  case TOK_LT:
    puts("LT");
    break;
  case TOK_GT:
    puts("GT");
    break;
  case TOK_LE:
    puts("LE");
    break;
  case TOK_GE:
    puts("GE");
    break;
  case TOK_EQEQ:
    puts("EQEQ");
    break;
  case TOK_NEQ:
    puts("NEQ");
    break;
  case TOK_AMP:
    puts("AMP");
    break;
  case TOK_PIPE:
    puts("PIPE");
    break;
  case TOK_CARET:
    puts("CARET");
    break;
  case TOK_TILDE:
    puts("TILDE");
    break;
  case TOK_AND:
    puts("AND");
    break;
  case TOK_OR:
    puts("OR");
    break;
  case TOK_BANG:
    puts("BANG");
    break;
  case TOK_QUESTION:
    puts("QUESTION");
    break;

  default:
    puts("UNKNOWN");
    break;
  }
}
