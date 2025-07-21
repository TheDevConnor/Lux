#include <stdio.h>

#include "../c_libs/color/color.h"
#include "../lexer/lexer.h"
#include "help.h"

void print_token(const Token *t) {
  printf("%.*s -> ", t->length, t->value);

  // Highlight token type to be BOLD_GREEN
  switch (t->type_) {
  case TOK_EOF:
    puts("EOF");
    break;
  case TOK_IDENTIFIER:
    printf(BOLD_GREEN("IDENTIFIER"));
    break;
  case TOK_KEYWORD:
    printf(BOLD_GREEN("KEYWORD"));
    break;
  case TOK_NUMBER:
    printf(BOLD_GREEN("NUMBER"));
    break;
  case TOK_STRING:
    printf(BOLD_GREEN("STRING")); 
    break;
  case TOK_CHAR_LITERAL:
    printf(BOLD_GREEN("CHAR_LITERAL"));
    break;
  case TOK_INT:
    printf(BOLD_GREEN("INT"));  
    break;
  case TOK_DOUBLE:
    printf(BOLD_GREEN("DOUBLE"));
    break;  
  case TOK_UINT:
    printf(BOLD_GREEN("UINT"));
    break;
  case TOK_FLOAT:
    printf(BOLD_GREEN("FLOAT"));
    break;
  case TOK_BOOL:
    printf(BOLD_GREEN("BOOL"));
    break;
  case TOK_STRINGT:
    printf(BOLD_GREEN("STRINGT"));
    break;
  case TOK_VOID:
    printf(BOLD_GREEN("VOID"));
    break;  
  case TOK_CHAR:
    printf(BOLD_GREEN("CHAR"));
    break;
  case TOK_IF:
    printf(BOLD_GREEN("IF"));
    break;  
  case TOK_ELSE:
    printf(BOLD_GREEN("ELSE"));
    break;
  case TOK_LOOP:  
    printf(BOLD_GREEN("LOOP"));
    break;
  case TOK_RETURN:  
    printf(BOLD_GREEN("RETURN"));
    break;
  case TOK_BREAK:  
    printf(BOLD_GREEN("BREAK"));
    break;  
  case TOK_CONTINUE:
    printf(BOLD_GREEN("CONTINUE"));
    break;
  case TOK_STRUCT:
    printf(BOLD_GREEN("STRUCT"));
    break;
  case TOK_ENUM:
    printf(BOLD_GREEN("ENUM"));
    break;
  case TOK_MOD:
    printf(BOLD_GREEN("MOD"));
    break;
  case TOK_IMPORT:
    printf(BOLD_GREEN("IMPORT"));
    break;
  case TOK_TRUE:
    printf(BOLD_GREEN("TRUE"));
    break;
  case TOK_FALSE:
    printf(BOLD_GREEN("FALSE"));
    break;
  case TOK_PUBLIC:
    printf(BOLD_GREEN("PUBLIC"));
    break;
  case TOK_PRIVATE:
    printf(BOLD_GREEN("PRIVATE"));
    break;

  case TOK_SYMBOL:
    printf(BOLD_GREEN("SYMBOL"));
    break;
  case TOK_LPAREN:
    printf(BOLD_GREEN("LPAREN"));
    break;
  case TOK_RPAREN:
    printf(BOLD_GREEN("RPAREN"));
    break;
  case TOK_LBRACE:
    printf(BOLD_GREEN("LBRACE"));
    break; 
  case TOK_RBRACE:
    printf(BOLD_GREEN("RBRACE"));
    break;
  case TOK_LBRACKET:
    printf(BOLD_GREEN("LBRACKET"));
    break;
  case TOK_RBRACKET:
    printf(BOLD_GREEN("RBRACKET"));
    break;  
  case TOK_SEMICOLON:
    printf(BOLD_GREEN("SEMICOLON"));
    break;
  case TOK_COMMA:
    printf(BOLD_GREEN("COMMA"));
    break;  
  case TOK_DOT:
    printf(BOLD_GREEN("DOT"));
    break;
  case TOK_EQUAL:
    printf(BOLD_GREEN("EQUAL"));
    break;
  case TOK_PLUS:
    printf(BOLD_GREEN("PLUS"));
    break;
  case TOK_MINUS:
    printf(BOLD_GREEN("MINUS"));
    break;
  case TOK_STAR:
    printf(BOLD_GREEN("STAR"));
    break;
  case TOK_SLASH:
    printf(BOLD_GREEN("SLASH"));
    break;
  case TOK_LT:
    printf(BOLD_GREEN("LT"));
    break;
  case TOK_GT:  
    printf(BOLD_GREEN("GT"));
    break;
  case TOK_LE:
    printf(BOLD_GREEN("LE"));
    break;
  case TOK_GE:  
    printf(BOLD_GREEN("GE"));
    break;
  case TOK_EQEQ:
    printf(BOLD_GREEN("EQEQ"));
    break;
  case TOK_NEQ:
    printf(BOLD_GREEN("NEQ"));
    break;
  case TOK_AMP:
    printf(BOLD_GREEN("AMP"));
    break;
  case TOK_PIPE:
    printf(BOLD_GREEN("PIPE"));
    break;
  case TOK_CARET:
    printf(BOLD_GREEN("CARET"));
    break;
  case TOK_TILDE:
    printf(BOLD_GREEN("TILDE"));
    break;
  case TOK_AND:
    printf(BOLD_GREEN("AND"));
    break;
  case TOK_OR:
    printf(BOLD_GREEN("OR"));
    break;  
  case TOK_BANG:
    printf(BOLD_GREEN("BANG"));
    break;
  case TOK_QUESTION:
    printf(BOLD_GREEN("QUESTION"));
    break;

  default:
    puts("UNKNOWN");
    break;
  }

  printf(" at line ");
  printf(UNDERLINE_COLORIZE(COLOR_RED, "%d"), t->line);
  printf(", column ");
  printf(UNDERLINE_COLORIZE(COLOR_RED, "%d"), t->col);
  printf("\n");
}
