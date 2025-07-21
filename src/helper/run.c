#include "help.h"

bool run_build(BuildConfig config, ArenaAllocator *allocator) {
  const char *source = read_file(config.filepath);
  if (!source) {
    fprintf(stderr, "Failed to read source file: %s\n", config.filepath);
    return false;
  }

  Lexer lexer;
  init_lexer(&lexer, source);

  GrowableArray tokens;
  if (!growable_array_init(&tokens, allocator, MAX_TOKENS, sizeof(Token))) {
    fprintf(stderr, "Failed to initialize token array.\n");
    free((void *)source);
    return false;
  }

  Token tk;
  while ((tk = next_token(&lexer)).type_ != TOK_EOF) {
    Token *slot = (Token *)growable_array_push(&tokens);
    if (!slot) {
      fprintf(stderr, "Out of memory while growing token array\n");
      free((void *)source);
      return false;
    }
    *slot = tk;
  }

  for (size_t i = 0; i < tokens.count; i++)
    print_token(&((Token *)tokens.data)[i]);

  if (config.name)
    printf("Building target: %s\n", config.name);
  if (config.save)
    printf("Saving output LLVM file.\n");
  if (config.clean)
    printf("Cleaning build artifacts.\n");

  free((void *)source);
  return true;
}
