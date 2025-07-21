#include "helper/help.h"
#include "lexer/lexer.h"

int main() {
  const char *source = "int main() {\n"
                       "  int x = 42;\n"
                       "  if (x > 0) return x;\n"
                       "  // comment\n"
                       "  return 0;\n"
                       "}";

  Lexer lexer;
  init_lexer(&lexer, source);

  Token t;
  do {
    t = next_token(&lexer);
    print_token(&t);
  } while (t.type != TOK_EOF);

  return 0;
}
