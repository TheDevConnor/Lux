#include "c_libs/memory/memory.h"
#include "helper/help.h"
#include "lexer/lexer.h"

int main() {
  const char *source = "int main() {\n"
                       "  int x = 42;\n"
                       "  if (x > 0) return x;\n"
                       "  // comment\n"
                       "  return 0;\n"
                       "}";
                       
  ArenaAllocator allocator;
  arena_allocator_init(&allocator, 1024);

  Lexer lexer;
  init_lexer(&lexer, source);

  // Define a array of tokens to be stored in the arena
  Token *tokens = arena_alloc(&allocator, sizeof(Token) * 100, alignof(Token));
  int token_count = 0;
  Token tk;

  while ((tk = next_token(&lexer)).type_ != TOK_EOF) {
    tokens[token_count++] = tk;
  }

  // Print the tokens
  for (int i = 0; i < token_count; i++) {
    print_token(&tokens[i]);
  }

  // Clean up
  arena_reset(&allocator);
  // ?Note: No need to free tokens, they are managed by the arena allocator
  // Destroy the arena allocator. This will free all buffers and reset the allocator
  arena_destroy(&allocator);
  return 0;
}