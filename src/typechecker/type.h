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
void init_scope(Scope *scope, Scope *parent, const char *name, ArenaAllocator *arena);
bool scope_add_symbol(Scope *scope, const char *name, AstNode *type, 
                     bool is_public, bool is_mutable, ArenaAllocator *arena);
Symbol *scope_lookup(Scope *scope, const char *name);
Symbol *scope_lookup_current_only(Scope *scope, const char *name);

// ============================================================================
// Type Operations
// ============================================================================

TypeMatchResult types_match(AstNode *type1, AstNode *type2);
bool is_numeric_type(AstNode *type);
bool is_pointer_type(AstNode *type);
bool is_array_type(AstNode *type);
AstNode *get_element_type(AstNode *array_or_pointer_type, ArenaAllocator *arena);
const char *type_to_string(AstNode *type, ArenaAllocator *arena);

// ============================================================================
// Main Typechecking Functions
// ============================================================================

bool typecheck(AstNode *node, Scope *scope, ArenaAllocator *arena);
AstNode *typecheck_expression(AstNode *expr, Scope *scope, ArenaAllocator *arena);
bool typecheck_statement(AstNode *stmt, Scope *scope, ArenaAllocator *arena);

// ============================================================================
// Specific Node Type Handlers
// ============================================================================

bool typecheck_var_decl(AstNode *node, Scope *scope, ArenaAllocator *arena);
bool typecheck_func_decl(AstNode *node, Scope *scope, ArenaAllocator *arena);
bool typecheck_struct_decl(AstNode *node, Scope *scope, ArenaAllocator *arena);
bool typecheck_enum_decl(AstNode *node, Scope *scope, ArenaAllocator *arena);
bool typecheck_return_decl(AstNode *node, Scope *scope, ArenaAllocator *arena);

AstNode *typecheck_binary_expr(AstNode *expr, Scope *scope, ArenaAllocator *arena);
AstNode *typecheck_call_expr(AstNode *expr, Scope *scope, ArenaAllocator *arena);

AstNode *get_enclosing_function_return_type(Scope *scope);

// ============================================================================
// Utility Functions
// ============================================================================

void print_type_error(TypeError *error);
Scope *create_child_scope(Scope *parent, const char *name, ArenaAllocator *arena);
void debug_print_scope(Scope *scope, int indent_level);