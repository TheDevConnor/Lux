#include <string.h>

#include "../c_libs/color/color.h"
#include "help.h"

bool check_argc(int argc, int expected) {
  if (argc < expected) {
    fprintf(stderr, "Usage: %s <source_file>\n", "lux");
    return false;
  }
  return true;
}

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
  return 0;
}

int print_version() {
  printf("Lux Compiler v1.0\n");
  return 0;
}

int print_license() {
  printf("Lux Compiler is licensed under the MIT License.\n");
  return 0;
}

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
        else {
          fprintf(stderr, "Unknown build option: %s\n", argv[j]);
          return false;
        }
      }
    }
  }

  return true;
}

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
  case TOK_RESOLVE:
    printf(BOLD_GREEN("RESOLVE"));
    break;
  case TOK_COLON:
    printf(BOLD_GREEN("COLON"));
    break;

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
