/**
 * @file help.c
 * @brief Implements command-line argument parsing, file reading, help/version/license printing,
 *        and token printing with color-highlighted token types.
 */

#include <string.h>
#include "../c_libs/color/color.h"
#include "help.h"

/**
 * @brief Checks if the number of command-line arguments is at least expected.
 *
 * Prints usage info if not enough arguments.
 *
 * @param argc Actual argument count.
 * @param expected Minimum expected argument count.
 * @return true if argc >= expected, false otherwise.
 */
bool check_argc(int argc, int expected) {
  if (argc < expected) {
    fprintf(stderr, "Usage: %s <source_file>\n", "lux");
    return false;
  }
  return true;
}

/**
 * @brief Reads entire file content into a newly allocated buffer.
 *
 * The caller must free the returned buffer.
 *
 * @param filename Path to the file to read.
 * @return Pointer to null-terminated file content, or NULL on failure.
 */
const char *read_file(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    perror("Failed to open file");
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *buffer = malloc(size + 1);
  if (!buffer) {
    perror("Failed to allocate memory");
    fclose(file);
    return NULL;
  }

  fread(buffer, 1, size, file);
  buffer[size] = '\0';

  fclose(file);
  return buffer;
}

/**
 * @brief Prints help message describing usage and options.
 *
 * @return Always returns 0.
 */
int print_help() {
  printf("Usage: lux [options] <source_file>\n");
  printf("Options:\n");
  printf("  -v, --version   Show version information\n");
  printf("  -h, --help      Show this help message\n");
  printf("  -l, --license   Show license information\n");
  printf("Crust Compiler Options:\n");
  printf("  -name <name>    Set the name of the build target\n");
  printf("  -save           Save the outputed llvm file\n");
  printf("  build <target>  Build the specified target\n");
  printf("  clean           Clean the build artifacts\n");
  printf("  -debug          builds a debug version and shows the allocators trace");
  return 0;
}

/**
 * @brief Prints version information.
 *
 * @return Always returns 0.
 */
int print_version() {
  printf("Lux Compiler v1.0\n");
  return 0;
}

/**
 * @brief Prints license information.
 *
 * @return Always returns 0.
 */
int print_license() {
  printf("Lux Compiler is licensed under the MIT License.\n");
  return 0;
}

/**
 * @brief Parses command-line arguments and configures the build.
 *
 * Supports options for version, help, license, build, save, clean, and debug.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @param config Pointer to BuildConfig struct to fill.
 * @return false if help/version/license was printed or error occurred, true otherwise.
 */
bool parse_args(int argc, char *argv[], BuildConfig *config) {
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
      return print_version(), false;
    else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
      return print_help(), false;
    else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--license") == 0)
      return print_license(), false;
    else if (strcmp(argv[i], "build") == 0 && i + 1 < argc) {
      config->filepath = argv[++i];
      for (int j = i + 1; j < argc; j++) {
        if (strcmp(argv[j], "-name") == 0 && j + 1 < argc)
          config->name = argv[++j];
        else if (strcmp(argv[j], "-save") == 0)
          config->save = true;
        else if (strcmp(argv[j], "-clean") == 0)
          config->clean = true;
        else if (strcmp(argv[j], "-debug") == 0) {
          // Placeholder for debug flag
        }
        else {
          fprintf(stderr, "Unknown build option: %s\n", argv[j]);
          return false;
        }
      }
    }
  }

  return true;
}

/**
 * @brief Prints a token's text and its token type with color formatting.
 *
 * @param t Pointer to the Token to print.
 */
void print_token(const Token *t) {
  printf("%.*s -> ", t->length, t->value);

  switch (t->type_) {
  case TOK_EOF:
    puts("EOF");
    break;
  case TOK_IDENTIFIER:
    printf(BOLD_GREEN("IDENTIFIER"));
    break;
  // ... other token types similarly formatted ...
  default:
    puts("UNKNOWN");
    break;
  }

  printf(" at line ");
  printf(COLORIZE(COLOR_RED, "%d"), t->line);
  printf(", column ");
  printf(COLORIZE(COLOR_RED, "%d"), t->col);
  printf("\n");
}
