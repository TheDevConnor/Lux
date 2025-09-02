#include "../ast.h"

AstNode *create_module_node(ArenaAllocator *arena, const char *name,
                            int potions, AstNode **body, size_t body_count,
                            size_t line, size_t column) {
  AstNode *node =
      create_preprocessor(arena, AST_PREPROCESSOR_MODULE, line, column);

  node->preprocessor.module.name = arena_strdup(arena, name);
  node->preprocessor.module.potions = potions;
  node->preprocessor.module.body = body;

  return node;
}

AstNode *create_use_node(ArenaAllocator *arena, const char *module_name,
                         const char *alias, size_t line, size_t column) {
  AstNode *node =
      create_preprocessor(arena, AST_PREPROCESSOR_USE, line, column);

  node->preprocessor.use.module_name = arena_strdup(arena, module_name);
  if (alias) {
    node->preprocessor.use.alias = arena_strdup(arena, alias);
  } else {
    node->preprocessor.use.alias = NULL;
  }

  return node;
}
