#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file color.h
 * @brief ANSI escape codes and convenience macros for terminal text coloring and styling.
 *
 * This header provides definitions for ANSI color and style escape sequences,
 * as well as convenient macros for coloring and styling text conditionally
 * based on whether the terminal supports color output.
 *
 * The macros wrap strings with appropriate ANSI sequences and reset codes,
 * enabling colorful, bold, and underlined text output in terminal applications.
 *
 * Usage example:
 * @code
 *   printf("%s\n", RED("Error: Something went wrong"));
 *   printf("%s\n", BOLD_GREEN("Success!"));
 * @endcode
 */

/** @name ANSI Color Codes */
/**@{*/
#define COLOR_RED     "\033[31m"     /**< ANSI code for red text */
#define COLOR_GREEN   "\033[32m"     /**< ANSI code for green text */
#define COLOR_BLUE    "\033[34m"     /**< ANSI code for blue text */
#define COLOR_CYAN    "\033[36m"     /**< ANSI code for cyan text */
#define COLOR_MAGENTA "\033[35m"     /**< ANSI code for magenta text */
#define COLOR_YELLOW  "\033[33m"     /**< ANSI code for yellow text */
#define COLOR_WHITE   "\033[37m"     /**< ANSI code for white text */
#define COLOR_BLACK   "\033[30m"     /**< ANSI code for black text */
#define COLOR_GRAY    "\033[90m"     /**< ANSI code for gray (bright black) text */
/**@}*/

/** @name ANSI Style Codes */
/**@{*/
#define STYLE_RESET     "\033[0m"    /**< Reset all styles */
#define STYLE_BOLD      "\033[1m"    /**< Bold text style */
#define STYLE_UNDERLINE "\033[4m"    /**< Underline text style */
/**@}*/

/** @name Background Color Codes */
/**@{*/
#define BG_RED     "\033[41m"        /**< ANSI code for red background */
#define BG_GREEN   "\033[42m"        /**< ANSI code for green background */
#define BG_BLUE    "\033[44m"        /**< ANSI code for blue background */
#define BG_CYAN    "\033[46m"        /**< ANSI code for cyan background */
#define BG_MAGENTA "\033[45m"        /**< ANSI code for magenta background */
#define BG_YELLOW  "\033[43m"        /**< ANSI code for yellow background */
#define BG_WHITE   "\033[47m"        /**< ANSI code for white background */
#define BG_BLACK   "\033[40m"        /**< ANSI code for black background */
#define BG_GRAY    "\033[100m"       /**< ANSI code for gray background */
/**@}*/

/** @name Convenience Macros for Common Color Usage
 *
 * These macros conditionally apply color styling to text if the terminal supports color,
 * otherwise return the original text unchanged.
 */
/**@{*/
#define RED(text)     (terminal_supports_color() ? COLOR_RED text STYLE_RESET : text)
#define GREEN(text)   (terminal_supports_color() ? COLOR_GREEN text STYLE_RESET : text)
#define BLUE(text)    (terminal_supports_color() ? COLOR_BLUE text STYLE_RESET : text)
#define CYAN(text)    (terminal_supports_color() ? COLOR_CYAN text STYLE_RESET : text)
#define MAGENTA(text) (terminal_supports_color() ? COLOR_MAGENTA text STYLE_RESET : text)
#define YELLOW(text)  (terminal_supports_color() ? COLOR_YELLOW text STYLE_RESET : text)
#define WHITE(text)   (terminal_supports_color() ? COLOR_WHITE text STYLE_RESET : text)
#define BLACK(text)   (terminal_supports_color() ? COLOR_BLACK text STYLE_RESET : text)
#define GRAY(text)    (terminal_supports_color() ? COLOR_GRAY text STYLE_RESET : text)
/**@}*/

/** @name Bold Color Macros
 *
 * Macros to print bold colored text conditionally based on terminal support.
 */
/**@{*/
#define BOLD_RED(text)     (terminal_supports_color() ? STYLE_BOLD COLOR_RED text STYLE_RESET : text)
#define BOLD_GREEN(text)   (terminal_supports_color() ? STYLE_BOLD COLOR_GREEN text STYLE_RESET : text)
#define BOLD_BLUE(text)    (terminal_supports_color() ? STYLE_BOLD COLOR_BLUE text STYLE_RESET : text)
#define BOLD_CYAN(text)    (terminal_supports_color() ? STYLE_BOLD COLOR_CYAN text STYLE_RESET : text)
#define BOLD_MAGENTA(text) (terminal_supports_color() ? STYLE_BOLD COLOR_MAGENTA text STYLE_RESET : text)
#define BOLD_YELLOW(text)  (terminal_supports_color() ? STYLE_BOLD COLOR_YELLOW text STYLE_RESET : text)
#define BOLD_WHITE(text)   (terminal_supports_color() ? STYLE_BOLD COLOR_WHITE text STYLE_RESET : text)
#define BOLD_BLACK(text)   (terminal_supports_color() ? STYLE_BOLD COLOR_BLACK text STYLE_RESET : text)
#define BOLD_GRAY(text)    (terminal_supports_color() ? STYLE_BOLD COLOR_GRAY text STYLE_RESET : text)
/**@}*/

/** @name Underlined Color Macros
 *
 * Macros to print underlined colored text conditionally based on terminal support.
 */
/**@{*/
#define UNDERLINE_RED(text)     (terminal_supports_color() ? STYLE_UNDERLINE COLOR_RED text STYLE_RESET : text)
#define UNDERLINE_GREEN(text)   (terminal_supports_color() ? STYLE_UNDERLINE COLOR_GREEN text STYLE_RESET : text)
#define UNDERLINE_BLUE(text)    (terminal_supports_color() ? STYLE_UNDERLINE COLOR_BLUE text STYLE_RESET : text)
#define UNDERLINE_CYAN(text)    (terminal_supports_color() ? STYLE_UNDERLINE COLOR_CYAN text STYLE_RESET : text)
#define UNDERLINE_MAGENTA(text) (terminal_supports_color() ? STYLE_UNDERLINE COLOR_MAGENTA text STYLE_RESET : text)
#define UNDERLINE_YELLOW(text)  (terminal_supports_color() ? STYLE_UNDERLINE COLOR_YELLOW text STYLE_RESET : text)
#define UNDERLINE_WHITE(text)   (terminal_supports_color() ? STYLE_UNDERLINE COLOR_WHITE text STYLE_RESET : text)
#define UNDERLINE_BLACK(text)   (terminal_supports_color() ? STYLE_UNDERLINE COLOR_BLACK text STYLE_RESET : text)
#define UNDERLINE_GRAY(text)    (terminal_supports_color() ? STYLE_UNDERLINE COLOR_GRAY text STYLE_RESET : text)
/**@}*/

/** @name Custom Formatting Macros
 *
 * Macros to apply arbitrary color and style sequences conditionally.
 */
/**@{*/
#define COLORIZE(color, text)           (terminal_supports_color() ? color text STYLE_RESET : text)
#define BOLD_COLORIZE(color, text)      (terminal_supports_color() ? STYLE_BOLD color text STYLE_RESET : text)
#define UNDERLINE_COLORIZE(color, text) (terminal_supports_color() ? STYLE_UNDERLINE color text STYLE_RESET : text)
#define STYLE_COLORIZE(style, color, text) (terminal_supports_color() ? style color text STYLE_RESET : text)
/**@}*/

/**
 * @brief Checks if the terminal supports ANSI color escape sequences.
 *
 * This function should detect whether the current terminal session supports
 * colored output. Common implementations check environment variables, platform,
 * or terminal capabilities.
 *
 * @return true if color output is supported, false otherwise.
 */
bool terminal_supports_color(void);

#ifdef __cplusplus
}
#endif
