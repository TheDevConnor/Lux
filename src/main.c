#include "c_libs/memory/memory.h"
#include "helper/help.h"

int main(int argc, char *argv[]) {
  if (!check_argc(argc, 1))
    return ARGC_ERROR;

  BuildConfig config = {0};

  if (!parse_args(argc, argv, &config)) {
    // Commands like --help or --version already handled inside parse_args
    return ARGC_ERROR;
  }

  if (!config.filepath) {
    fprintf(stderr, "No source file provided.\n");
    return ARGC_ERROR;
  }

  ArenaAllocator allocator;
  arena_allocator_init(&allocator, 1024 * 1024);

  bool success = run_build(config, &allocator);

  arena_destroy(&allocator);
  return success ? 0 : 1;
}