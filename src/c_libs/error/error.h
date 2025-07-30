#pragma once

#include <stdbool.h>

typedef struct {
  const char *error_type;
  const char *file_path;
  const char *message;

  int line;
  int col;

  const char *line_text; // Optional: source line text
  int token_length;

  const char *label; // optional
  const char *note;  // optional
  const char *help;  // optional
} ErrorInformation;

void error_add(ErrorInformation err);
bool error_report(void);
void error_clear(void);