#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ANSI Color Codes */
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_WHITE   "\033[37m"
#define COLOR_BLACK   "\033[30m"
#define COLOR_GRAY    "\033[90m"

/* ANSI Style Codes */
#define STYLE_RESET     "\033[0m"
#define STYLE_BOLD      "\033[1m"
#define STYLE_UNDERLINE "\033[4m"

/* Background Colors */
#define BG_RED     "\033[41m"
#define BG_GREEN   "\033[42m"
#define BG_BLUE    "\033[44m"
#define BG_CYAN    "\033[46m"
#define BG_MAGENTA "\033[45m"
#define BG_YELLOW  "\033[43m"
#define BG_WHITE   "\033[47m"
#define BG_BLACK   "\033[40m"
#define BG_GRAY    "\033[100m"

/* Convenience Macros for Common Usage */
#define RED(text)     (terminal_supports_color() ? COLOR_RED text STYLE_RESET : text)
#define GREEN(text)   (terminal_supports_color() ? COLOR_GREEN text STYLE_RESET : text)
#define BLUE(text)    (terminal_supports_color() ? COLOR_BLUE text STYLE_RESET : text)
#define CYAN(text)    (terminal_supports_color() ? COLOR_CYAN text STYLE_RESET : text)
#define MAGENTA(text) (terminal_supports_color() ? COLOR_MAGENTA text STYLE_RESET : text)
#define YELLOW(text)  (terminal_supports_color() ? COLOR_YELLOW text STYLE_RESET : text)
#define WHITE(text)   (terminal_supports_color() ? COLOR_WHITE text STYLE_RESET : text)
#define BLACK(text)   (terminal_supports_color() ? COLOR_BLACK text STYLE_RESET : text)
#define GRAY(text)    (terminal_supports_color() ? COLOR_GRAY text STYLE_RESET : text)

/* Bold Color Macros */
#define BOLD_RED(text)     (terminal_supports_color() ? STYLE_BOLD COLOR_RED text STYLE_RESET : text)
#define BOLD_GREEN(text)   (terminal_supports_color() ? STYLE_BOLD COLOR_GREEN text STYLE_RESET : text)
#define BOLD_BLUE(text)    (terminal_supports_color() ? STYLE_BOLD COLOR_BLUE text STYLE_RESET : text)
#define BOLD_CYAN(text)    (terminal_supports_color() ? STYLE_BOLD COLOR_CYAN text STYLE_RESET : text)
#define BOLD_MAGENTA(text) (terminal_supports_color() ? STYLE_BOLD COLOR_MAGENTA text STYLE_RESET : text)
#define BOLD_YELLOW(text)  (terminal_supports_color() ? STYLE_BOLD COLOR_YELLOW text STYLE_RESET : text)
#define BOLD_WHITE(text)   (terminal_supports_color() ? STYLE_BOLD COLOR_WHITE text STYLE_RESET : text)
#define BOLD_BLACK(text)   (terminal_supports_color() ? STYLE_BOLD COLOR_BLACK text STYLE_RESET : text)
#define BOLD_GRAY(text)    (terminal_supports_color() ? STYLE_BOLD COLOR_GRAY text STYLE_RESET : text)

/* Underlined Color Macros */
#define UNDERLINE_RED(text)     (terminal_supports_color() ? STYLE_UNDERLINE COLOR_RED text STYLE_RESET : text)
#define UNDERLINE_GREEN(text)   (terminal_supports_color() ? STYLE_UNDERLINE COLOR_GREEN text STYLE_RESET : text)
#define UNDERLINE_BLUE(text)    (terminal_supports_color() ? STYLE_UNDERLINE COLOR_BLUE text STYLE_RESET : text)
#define UNDERLINE_CYAN(text)    (terminal_supports_color() ? STYLE_UNDERLINE COLOR_CYAN text STYLE_RESET : text)
#define UNDERLINE_MAGENTA(text) (terminal_supports_color() ? STYLE_UNDERLINE COLOR_MAGENTA text STYLE_RESET : text)
#define UNDERLINE_YELLOW(text)  (terminal_supports_color() ? STYLE_UNDERLINE COLOR_YELLOW text STYLE_RESET : text)
#define UNDERLINE_WHITE(text)   (terminal_supports_color() ? STYLE_UNDERLINE COLOR_WHITE text STYLE_RESET : text)
#define UNDERLINE_BLACK(text)   (terminal_supports_color() ? STYLE_UNDERLINE COLOR_BLACK text STYLE_RESET : text)
#define UNDERLINE_GRAY(text)    (terminal_supports_color() ? STYLE_UNDERLINE COLOR_GRAY text STYLE_RESET : text)

/* Custom formatting macros */
#define COLORIZE(color, text)           (terminal_supports_color() ? color text STYLE_RESET : text)
#define BOLD_COLORIZE(color, text)      (terminal_supports_color() ? STYLE_BOLD color text STYLE_RESET : text)
#define UNDERLINE_COLORIZE(color, text) (terminal_supports_color() ? STYLE_UNDERLINE color text STYLE_RESET : text)
#define STYLE_COLORIZE(style, color, text) (terminal_supports_color() ? style color text STYLE_RESET : text)

/* Check if terminal supports color */
bool terminal_supports_color(void);

#ifdef __cplusplus
}
#endif