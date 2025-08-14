#pragma once

#include "../ast/ast.h"
#include "../c_libs/memory/memory.h"

// Enhanced symbol structure to work with AST types
typedef struct {
    const char *name;
    AstNode *type;        // Now points to AST type nodes
    bool is_public;
    bool is_mutable;
    size_t scope_depth;   // Track nesting level for better debugging
} Symbol;

// Enhanced scope with additional metadata
typedef struct Scope {
    struct Scope *parent;
    GrowableArray symbols;
    const char *scope_name;     // For debugging (function name, block, etc.)
    size_t depth;               // Nesting depth
    bool is_function_scope;     // Special handling for function scopes
    AstNode *associated_node;   // Link back to AST node that created this scope
} Scope;

// Type comparison result
typedef enum {
    TYPE_MATCH_EXACT,
    TYPE_MATCH_COMPATIBLE,  // Can be implicitly converted
    TYPE_MATCH_NONE
} TypeMatchResult;

// Error reporting structure
typedef struct {
    const char *message;
    size_t line;
    size_t column;
    const char *context;
} TypeError;

// Function declarations
void init_scope(Scope *scope, Scope *parent, const char *name, ArenaAllocator *arena);
bool scope_add_symbol(Scope *scope, const char *name, AstNode *type, 
                     bool is_public, bool is_mutable, ArenaAllocator *arena);
Symbol *scope_lookup(Scope *scope, const char *name);
Symbol *scope_lookup_current_only(Scope *scope, const char *name);

// Type operations
TypeMatchResult types_match(AstNode *type1, AstNode *type2);
bool is_numeric_type(AstNode *type);
bool is_pointer_type(AstNode *type);
bool is_array_type(AstNode *type);
AstNode *get_element_type(AstNode *array_or_pointer_type, ArenaAllocator *arena);
const char *type_to_string(AstNode *type, ArenaAllocator *arena);

// Main typechecking functions
bool typecheck(AstNode *node, Scope *scope, ArenaAllocator *arena);
AstNode *typecheck_expression(AstNode *expr, Scope *scope, ArenaAllocator *arena);
bool typecheck_statement(AstNode *stmt, Scope *scope, ArenaAllocator *arena);

// Specific node type handlers
bool typecheck_var_decl(AstNode *node, Scope *scope, ArenaAllocator *arena);
bool typecheck_func_decl(AstNode *node, Scope *scope, ArenaAllocator *arena);
bool typecheck_struct_decl(AstNode *node, Scope *scope, ArenaAllocator *arena);
bool typecheck_enum_decl(AstNode *node, Scope *scope, ArenaAllocator *arena);
AstNode *typecheck_binary_expr(AstNode *expr, Scope *scope, ArenaAllocator *arena);
AstNode *typecheck_call_expr(AstNode *expr, Scope *scope, ArenaAllocator *arena);

// Utility functions
void print_type_error(TypeError *error);
Scope *create_child_scope(Scope *parent, const char *name, ArenaAllocator *arena);
void debug_print_scope(Scope *scope, int indent_level);