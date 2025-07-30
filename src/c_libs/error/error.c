#include <stdio.h>
#include <stdarg.h>

#include "../color/color.h"
#include "error.h"

#define MAX_ERRORS 256
static ErrorInformation error_list[MAX_ERRORS];
static int error_count = 0;

void error_add(ErrorInformation err) {
    if (error_count < MAX_ERRORS) {
        error_list[error_count++] = err;
    }
}

void error_clear(void) {
    error_count = 0;
}

const char *convert_line_to_string(int line) {
    static char buffer[16];
    
    // First get the width by formatting the actual number
    char temp[16];
    int width = snprintf(temp, sizeof(temp), "%d", line);
    
    // Then create a string of zeros with that width
    for (int i = 0; i < width; i++) {
        buffer[i] = '0';
    }
    buffer[width] = '\0';
    
    return buffer;
}

static void print_indicator(int col, int len, int line) {
    printf(GRAY(" %s | "), convert_line_to_string(line));
    for (int i = 1; i < col; i++) printf(" ");
    for (int i = 0; i < len; i++) printf(RED("^"));
    printf(STYLE_RESET "\n");
}

static void print_source_line(int line, const char *text) {
    if (text) {
        // printf("%s%2d |%s ", GRAY(""), line, STYLE_RESET);
        printf(GRAY(" %d | "), line);
        printf(BOLD_WHITE("%s\n"), text);
    } else {
        printf(GRAY(" %d |\n"), line);
    }
}

bool error_report(void) {
    if (error_count == 0) return false;

    printf("%s: %d\n", BOLD_WHITE("Total errors"), error_count);

    for (int i = 0; i < error_count; i++) {
        ErrorInformation *e = &error_list[i];
        printf(RED("%s: "), e->error_type);
        printf(BOLD_WHITE("%s\n"), e->message);
        printf("  -->");
        printf(BOLD_YELLOW("%s"), e->file_path);
        printf(":%d::%d\n", e->line, e->col);

        printf(GRAY(" %s |\n"), convert_line_to_string(e->line));
        print_source_line(e->line, e->line_text);
        print_indicator(e->col, e->token_length > 0 ? e->token_length : 1, e->line);

        if (e->label) {
            printf("  %s: %s\n", CYAN("label"), e->label);
        }
        if (e->note) {
            printf("  %s: %s\n", CYAN("note"), e->note);
        }
        if (e->help) {
            printf("  %s: %s\n", CYAN("help"), e->help);
        }
        printf("\n");
    }
    return true;
}