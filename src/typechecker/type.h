/**
 * @file type.h
 * @brief Type checking and symbol table management for AST nodes
 * 
 * This module provides comprehensive type checking functionality for an Abstract Syntax Tree (AST).
 * It includes symbol table management with scoped lookups, type comparison utilities, and 
 * specialized type checking functions for various AST node types.
 * 
 * The type system supports:
 * - Hierarchical scoped symbol tables
 * - Type compatibility checking with implicit conversions
 * - Function, struct, and enum type validation
 * - Expression type inference and validation
 * - Comprehensive error reporting
 */

#pragma once

#include "../ast/ast.h"
#include "../c_libs/memory/memory.h"

/**
 * @brief Enhanced symbol structure to work with AST types
 * 
 * Represents a symbol in the symbol table with associated type information
 * and metadata for scope management and access control.
 */
typedef struct {
    const char *name;        /**< Symbol name identifier */
    AstNode *type;          /**< Points to AST type nodes for type information */
    bool is_public;         /**< Public accessibility flag */
    bool is_mutable;        /**< Mutability flag for the symbol */
    size_t scope_depth;     /**< Track nesting level for better debugging */
} Symbol;

/**
 * @brief Enhanced scope with additional metadata
 * 
 * Represents a lexical scope in the program with hierarchical parent-child
 * relationships and associated metadata for debugging and analysis.
 */
typedef struct Scope {
    struct Scope *parent;       /**< Parent scope for hierarchical lookup */
    GrowableArray symbols;      /**< Array of Symbol structures in this scope */
    GrowableArray children;     /**< Array of child Scope* pointers */
    const char *scope_name;     /**< Name for debugging (function name, block, etc.) */
    size_t depth;              /**< Nesting depth from global scope */
    bool is_function_scope;    /**< Special handling flag for function scopes */
    AstNode *associated_node;  /**< Link back to AST node that created this scope */
} Scope;

/**
 * @brief Type comparison result enumeration
 * 
 * Represents the level of compatibility between two types during type checking.
 */
typedef enum {
    TYPE_MATCH_EXACT,       /**< Types match exactly */
    TYPE_MATCH_COMPATIBLE,  /**< Types can be implicitly converted */
    TYPE_MATCH_NONE        /**< Types are incompatible */
} TypeMatchResult;

/**
 * @brief Error reporting structure for type checking errors
 * 
 * Contains detailed information about type errors including location
 * and contextual information for meaningful error messages.
 */
typedef struct {
    const char *message;    /**< Error description message */
    size_t line;           /**< Line number where error occurred */
    size_t column;         /**< Column number where error occurred */
    const char *context;   /**< Additional context information */
} TypeError;

// ============================================================================
// Scope Management Functions
// ============================================================================

/**
 * @brief Initialize a new scope structure
 * 
 * @param scope Pointer to the scope structure to initialize
 * @param parent Parent scope for hierarchical lookup (can be NULL for global scope)
 * @param name Descriptive name for the scope (for debugging)
 * @param arena Arena allocator for memory management
 */
void init_scope(Scope *scope, Scope *parent, const char *name, ArenaAllocator *arena);

/**
 * @brief Add a symbol to the specified scope
 * 
 * @param scope Target scope to add the symbol to
 * @param name Symbol name identifier
 * @param type AST node representing the symbol's type
 * @param is_public Public accessibility flag
 * @param is_mutable Mutability flag for the symbol
 * @param arena Arena allocator for memory management
 * @return true if symbol was added successfully, false if name already exists
 */
bool scope_add_symbol(Scope *scope, const char *name, AstNode *type, 
                     bool is_public, bool is_mutable, ArenaAllocator *arena);

/**
 * @brief Look up a symbol by name with hierarchical scope traversal
 * 
 * Searches for a symbol starting from the given scope and traversing up
 * the parent chain until found or reaching the global scope.
 * 
 * @param scope Starting scope for the lookup
 * @param name Symbol name to search for
 * @return Pointer to Symbol if found, NULL otherwise
 */
Symbol *scope_lookup(Scope *scope, const char *name);

/**
 * @brief Look up a symbol only in the current scope (no parent traversal)
 * 
 * @param scope Scope to search in
 * @param name Symbol name to search for
 * @return Pointer to Symbol if found in current scope, NULL otherwise
 */
Symbol *scope_lookup_current_only(Scope *scope, const char *name);

// ============================================================================
// Type Operations
// ============================================================================

/**
 * @brief Compare two types for compatibility
 * 
 * Determines if two AST type nodes are compatible, including exact matches
 * and implicit conversion possibilities.
 * 
 * @param type1 First type to compare
 * @param type2 Second type to compare
 * @return TypeMatchResult indicating level of compatibility
 */
TypeMatchResult types_match(AstNode *type1, AstNode *type2);

/**
 * @brief Check if a type is numeric (int, float, etc.)
 * 
 * @param type AST node representing the type to check
 * @return true if type is numeric, false otherwise
 */
bool is_numeric_type(AstNode *type);

/**
 * @brief Check if a type is a pointer type
 * 
 * @param type AST node representing the type to check
 * @return true if type is a pointer, false otherwise
 */
bool is_pointer_type(AstNode *type);

/**
 * @brief Check if a type is an array type
 * 
 * @param type AST node representing the type to check
 * @return true if type is an array, false otherwise
 */
bool is_array_type(AstNode *type);

/**
 * @brief Get the element type from an array or pointer type
 * 
 * @param array_or_pointer_type AST node representing array or pointer type
 * @param arena Arena allocator for memory management
 * @return AST node representing the element type, NULL if not applicable
 */
AstNode *get_element_type(AstNode *array_or_pointer_type, ArenaAllocator *arena);

/**
 * @brief Convert a type AST node to a human-readable string representation
 * 
 * @param type AST node representing the type
 * @param arena Arena allocator for string allocation
 * @return String representation of the type
 */
const char *type_to_string(AstNode *type, ArenaAllocator *arena);

// ============================================================================
// Main Typechecking Functions
// ============================================================================

/**
 * @brief Perform comprehensive type checking on an AST node
 * 
 * Main entry point for type checking that handles all node types
 * and performs appropriate validation based on the node type.
 * 
 * @param node AST node to type check
 * @param scope Current scope for symbol resolution
 * @param arena Arena allocator for memory management
 * @return true if type checking passed, false on type errors
 */
bool typecheck(AstNode *node, Scope *scope, ArenaAllocator *arena);

/**
 * @brief Type check an expression and return its inferred type
 * 
 * @param expr Expression AST node to type check
 * @param scope Current scope for symbol resolution
 * @param arena Arena allocator for memory management
 * @return AST node representing the expression's type, NULL on error
 */
AstNode *typecheck_expression(AstNode *expr, Scope *scope, ArenaAllocator *arena);

/**
 * @brief Type check a statement for correctness
 * 
 * @param stmt Statement AST node to type check
 * @param scope Current scope for symbol resolution
 * @param arena Arena allocator for memory management
 * @return true if statement is type-correct, false on errors
 */
bool typecheck_statement(AstNode *stmt, Scope *scope, ArenaAllocator *arena);

// ============================================================================
// Specific Node Type Handlers
// ============================================================================

/**
 * @brief Type check a variable declaration node
 * 
 * Validates variable type, initializer compatibility, and adds the
 * variable to the current scope.
 * 
 * @param node Variable declaration AST node
 * @param scope Current scope for symbol addition
 * @param arena Arena allocator for memory management
 * @return true if declaration is valid, false on errors
 */
bool typecheck_var_decl(AstNode *node, Scope *scope, ArenaAllocator *arena);

/**
 * @brief Type check a function declaration node
 * 
 * Validates function signature, parameter types, return type, and
 * creates a new function scope for body checking.
 * 
 * @param node Function declaration AST node
 * @param scope Current scope for function registration
 * @param arena Arena allocator for memory management
 * @return true if function declaration is valid, false on errors
 */
bool typecheck_func_decl(AstNode *node, Scope *scope, ArenaAllocator *arena);

/**
 * @brief Type check a struct declaration node
 * 
 * Validates struct field types, checks for circular dependencies,
 * and registers the struct type in the current scope.
 * 
 * @param node Struct declaration AST node
 * @param scope Current scope for struct registration
 * @param arena Arena allocator for memory management
 * @return true if struct declaration is valid, false on errors
 */
bool typecheck_struct_decl(AstNode *node, Scope *scope, ArenaAllocator *arena);

/**
 * @brief Type check an enum declaration node
 * 
 * Validates enum value types and consistency, registers the enum
 * type and its values in the current scope.
 * 
 * @param node Enum declaration AST node
 * @param scope Current scope for enum registration
 * @param arena Arena allocator for memory management
 * @return true if enum declaration is valid, false on errors
 */
bool typecheck_enum_decl(AstNode *node, Scope *scope, ArenaAllocator *arena);

/**
 * @brief Type check a binary expression and infer its result type
 * 
 * Validates operand types, checks operator compatibility, and
 * determines the resulting type of the binary operation.
 * 
 * @param expr Binary expression AST node
 * @param scope Current scope for symbol resolution
 * @param arena Arena allocator for memory management
 * @return AST node representing the result type, NULL on error
 */
AstNode *typecheck_binary_expr(AstNode *expr, Scope *scope, ArenaAllocator *arena);

/**
 * @brief Type check a function call expression
 * 
 * Validates function existence, argument count and types,
 * and returns the function's return type.
 * 
 * @param expr Function call expression AST node
 * @param scope Current scope for function resolution
 * @param arena Arena allocator for memory management
 * @return AST node representing the return type, NULL on error
 */
AstNode *typecheck_call_expr(AstNode *expr, Scope *scope, ArenaAllocator *arena);

/**
 * @brief Validates that all return statements in a function body match the expected return type
 * 
 * This function traverses the function body AST and validates that every return statement
 * is compatible with the function's declared return type. It performs type checking to
 * ensure type safety and correctness of the function implementation.
 * 
 * @param body Pointer to the AST node representing the function body to validate
 * @param expected_return_type Pointer to the AST node representing the expected return type
 * @param arena Pointer to the arena allocator used for temporary memory allocations during validation
 * 
 * @return true if all return statements are valid and match the expected type, false otherwise
 * 
 * @note The function handles void return types and ensures proper type compatibility
 * @note Uses the provided arena allocator for any temporary allocations needed during validation
 * 
 * @see collect_return_statements
 */
bool validate_function_returns(AstNode *body, AstNode *expected_return_type, ArenaAllocator *arena);

/**
 * @brief Collects all return statements from an AST subtree
 * 
 * This function recursively traverses the given AST node and its children to find and collect
 * all return statements. The collected return statements are stored in a dynamically resized
 * array, with the count tracked separately.
 * 
 * @param node Pointer to the root AST node to search for return statements
 * @param returns Pointer to a pointer array that will be populated with found return statements.
 *                The array is dynamically allocated/reallocated using the arena allocator.
 * @param return_count Pointer to a size_t variable that will store the number of return statements found
 * @param arena Pointer to the arena allocator used for memory management of the returns array
 * 
 * @note The function modifies the contents of *returns and *return_count
 * @note Memory for the returns array is managed through the provided arena allocator
 * @note The function performs a deep traversal of the AST to find all nested return statements
 * 
 * @warning The caller should ensure that returns and return_count are properly initialized
 * 
 * @see validate_function_returns
 */
void collect_return_statements(AstNode *node, AstNode ***returns, size_t *return_count, ArenaAllocator *arena);

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * @brief Print a formatted type error message
 * 
 * @param error TypeError structure containing error details
 */
void print_type_error(TypeError *error);

/**
 * @brief Create a new child scope under the given parent
 * 
 * @param parent Parent scope for the new child scope
 * @param name Descriptive name for the new scope
 * @param arena Arena allocator for memory management
 * @return Pointer to the newly created child scope
 */
Scope *create_child_scope(Scope *parent, const char *name, ArenaAllocator *arena);

/**
 * @brief Print debug information for a scope and its hierarchy
 * 
 * Recursively prints scope structure with symbols and children
 * for debugging and analysis purposes.
 * 
 * @param scope Scope to print debug information for
 * @param indent_level Current indentation level for nested scopes
 */
void debug_print_scope(Scope *scope, int indent_level);