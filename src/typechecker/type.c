/**
 * @file type.c
 * @brief Implementation of type checking and symbol table management functions
 * 
 * This file contains the implementation of the type checking system declared in type.h.
 * It provides concrete implementations for type comparison, type introspection utilities,
 * and debugging functions for the symbol table and scope management system.
 */

#include <string.h>
#include <stdio.h>

#include "../ast/ast_utils.h"
#include "type.h"

/**
 * @brief Compare two AST type nodes for compatibility
 * 
 * Performs comprehensive type matching including exact matches and compatible
 * conversions. Handles basic types, pointer types, and array types with
 * recursive comparison for complex types.
 * 
 * @param type1 First type node to compare
 * @param type2 Second type node to compare
 * @return TYPE_MATCH_EXACT for identical types, TYPE_MATCH_COMPATIBLE for 
 *         implicitly convertible types, TYPE_MATCH_NONE for incompatible types
 * 
 * @details
 * Comparison rules:
 * - Identical pointers return TYPE_MATCH_EXACT
 * - Basic types: exact string match returns TYPE_MATCH_EXACT
 * - Numeric compatibility: int/float conversions return TYPE_MATCH_COMPATIBLE
 * - Pointer types: recursively compare pointee types
 * - Array types: recursively compare element types
 * - All other combinations return TYPE_MATCH_NONE
 */
TypeMatchResult types_match(AstNode *type1, AstNode *type2) {
    // Null pointer safety check
    if (!type1 || !type2) return TYPE_MATCH_NONE;
    
    // Identity check - same pointer means exact match
    if (type1 == type2) return TYPE_MATCH_EXACT;
    
    // Both must be type nodes to be comparable
    if (type1->category != Node_Category_TYPE || type2->category != Node_Category_TYPE) {
        return TYPE_MATCH_NONE;
    }
    
    // Basic type matching with string comparison
    if (type1->type == AST_TYPE_BASIC && type2->type == AST_TYPE_BASIC) {
        if (strcmp(type1->type_data.basic.name, type2->type_data.basic.name) == 0) {
            return TYPE_MATCH_EXACT;
        }
        
        // Check for compatible numeric types (implicit conversions)
        const char *name1 = type1->type_data.basic.name;
        const char *name2 = type2->type_data.basic.name;
        
        if ((strcmp(name1, "int") == 0 && strcmp(name2, "float") == 0) ||
            (strcmp(name1, "float") == 0 && strcmp(name2, "int") == 0)) {
            return TYPE_MATCH_COMPATIBLE;
        }
    }
    
    // Pointer type matching - recursively check pointee types
    if (type1->type == AST_TYPE_POINTER && type2->type == AST_TYPE_POINTER) {
        return types_match(type1->type_data.pointer.pointee_type, 
                          type2->type_data.pointer.pointee_type);
    }
    
    // Array type matching - recursively check element types
    if (type1->type == AST_TYPE_ARRAY && type2->type == AST_TYPE_ARRAY) {
        return types_match(type1->type_data.array.element_type,
                          type2->type_data.array.element_type);
    }
    
    return TYPE_MATCH_NONE;
}

/**
 * @brief Check if an AST type node represents a numeric type
 * 
 * Determines whether the given type is a basic numeric type that supports
 * arithmetic operations. Currently supports int, float, double, and char types.
 * 
 * @param type AST type node to examine
 * @return true if the type is numeric (int, float, double, char), false otherwise
 * 
 * @note char is considered numeric as it can participate in arithmetic operations
 *       and has an underlying integer representation
 */
bool is_numeric_type(AstNode *type) {
    // Validate input and ensure it's a basic type node
    if (!type || type->category != Node_Category_TYPE || type->type != AST_TYPE_BASIC) {
        return false;
    }
    
    const char *name = type->type_data.basic.name;
    return strcmp(name, "int") == 0 || 
           strcmp(name, "float") == 0 || 
           strcmp(name, "double") == 0 ||
           strcmp(name, "char") == 0;
}

/**
 * @brief Check if an AST type node represents a pointer type
 * 
 * Simple type classification function that determines if the given
 * type node represents a pointer to another type.
 * 
 * @param type AST type node to examine
 * @return true if the type is a pointer type, false otherwise
 */
bool is_pointer_type(AstNode *type) {
    return type && type->category == Node_Category_TYPE && type->type == AST_TYPE_POINTER;
}

/**
 * @brief Check if an AST type node represents an array type
 * 
 * Simple type classification function that determines if the given
 * type node represents an array of elements.
 * 
 * @param type AST type node to examine
 * @return true if the type is an array type, false otherwise
 */
bool is_array_type(AstNode *type) {
    return type && type->category == Node_Category_TYPE && type->type == AST_TYPE_ARRAY;
}

/**
 * @brief Convert an AST type node to a human-readable string representation
 * 
 * Generates a string representation of the type suitable for error messages,
 * debugging output, and user-facing diagnostics. Handles basic types, pointers,
 * and arrays with appropriate syntax.
 * 
 * @param type AST type node to convert
 * @param arena Arena allocator for string allocation
 * @return Allocated string representation of the type
 * 
 * @details
 * String format examples:
 * - Basic types: "int", "float", "char"
 * - Pointer types: "int*", "float*"
 * - Array types: "int[]", "float[]"
 * - Nested types: "int*[]" (array of int pointers)
 * - Invalid/unknown types: "unknown"
 * 
 * @note The returned string is allocated using the provided arena allocator
 *       and will be automatically freed when the arena is destroyed
 */
const char *type_to_string(AstNode *type, ArenaAllocator *arena) {
    // Handle null or invalid type nodes
    if (!type || type->category != Node_Category_TYPE) {
        return arena_strdup(arena, "unknown");
    }
    
    switch (type->type) {
        case AST_TYPE_BASIC:
            // Simple case - just return the basic type name
            return arena_strdup(arena, type->type_data.basic.name);
        
        case AST_TYPE_POINTER: {
            // Recursively get pointee type and append '*'
            const char *pointee = type_to_string(type->type_data.pointer.pointee_type, arena);
            size_t len = strlen(pointee) + 2;  // +1 for '*', +1 for null terminator
            char *result = arena_alloc(arena, len, len);
            snprintf(result, len, "%s*", pointee);
            return result;
        }
        
        case AST_TYPE_ARRAY: {
            // Recursively get element type and append '[]'
            const char *element = type_to_string(type->type_data.array.element_type, arena);
            size_t len = strlen(element) + 3; // +2 for "[]", +1 for null terminator
            char *result = arena_alloc(arena, len, len);
            snprintf(result, len, "%s[]", element);
            return result;
        }
        
        default:
            // Fallback for unknown or unhandled type categories
            return arena_strdup(arena, "unknown");
    }
}

void collect_return_statements(AstNode *node, AstNode ***returns, size_t *return_count, ArenaAllocator *arena) {
    if (!node) return;
    
    // If this is a return statement, add it to our collection
    if (node->category == Node_Category_STMT && node->type == AST_STMT_RETURN) {
        // Grow the array if needed
        *returns = arena_alloc(arena, (*return_count + 1) * sizeof(AstNode*), alignof(AstNode));
        (*returns)[*return_count] = node;
        (*return_count)++;
        return; // Don't recurse into return statements
    }
    
    // Recursively search through different node types
    switch (node->category) {
        case Node_Category_STMT:
            switch (node->type) {
                case AST_STMT_BLOCK:
                    for (size_t i = 0; i < node->stmt.block.stmt_count; i++) {
                        collect_return_statements(node->stmt.block.statements[i], returns, return_count, arena);
                    }
                    break;
                    
                case AST_STMT_IF:
                    collect_return_statements(node->stmt.if_stmt.then_stmt, returns, return_count, arena);
                    for (int i = 0; i < node->stmt.if_stmt.elif_count; i++) {
                        collect_return_statements(node->stmt.if_stmt.elif_stmts[i], returns, return_count, arena);
                    }
                    if (node->stmt.if_stmt.else_stmt) {
                        collect_return_statements(node->stmt.if_stmt.else_stmt, returns, return_count, arena);
                    }
                    break;
                    
                case AST_STMT_LOOP:
                    collect_return_statements(node->stmt.loop_stmt.body, returns, return_count, arena);
                    break;
                    
                case AST_STMT_FUNCTION:
                    // Don't recurse into nested functions
                    break;
                    
                default:
                    // For other statement types, we don't expect returns
                    break;
            }
            break;
            
        default:
            // Expressions and types don't contain return statements
            break;
    }
}

bool validate_function_returns(AstNode *body, AstNode *expected_return_type, ArenaAllocator *arena, Scope *scope) {
    if (!body || !expected_return_type) return false;
    
    // Collect all return statements in the function body
    AstNode **return_stmts = NULL;
    size_t return_count = 0;
    
    collect_return_statements(body, &return_stmts, &return_count, arena);
    
    // Check if function should return void
    bool expects_void = (expected_return_type->type == AST_TYPE_BASIC && 
                        strcmp(expected_return_type->type_data.basic.name, "void") == 0);
    
    // If expecting void, all returns should have no value
    if (expects_void) {
        for (size_t i = 0; i < return_count; i++) {
            if (return_stmts[i]->stmt.return_stmt.value != NULL) {
                fprintf(stderr, "    Error: Void function cannot return a value\n");
                return false;
            }
        }
        return true;
    }
    
    // For non-void functions, check all return values match expected type
    for (size_t i = 0; i < return_count; i++) {
        AstNode *return_value = typecheck_expression(return_stmts[i]->stmt.return_stmt.value, scope, arena);
        TypeMatchResult result = types_match(expected_return_type, return_value);
        
        if (result == TYPE_MATCH_NONE) return false;
        continue;
    }
    
    return true;
}

/**
 * @brief Print detailed debug information for a scope and its hierarchy
 * 
 * Recursively prints the complete scope tree structure including symbols,
 * child scopes, and metadata. Useful for debugging scope resolution issues
 * and understanding the program's symbol table structure.
 * 
 * @param scope Root scope to start printing from
 * @param indent_level Current indentation level for nested display
 * 
 * @details
 * Output format includes:
 * - Scope name, depth, symbol count, and child count
 * - All symbols with their types and attributes (public, mutable)
 * - Recursive display of all child scopes with increased indentation
 * 
 * @note This function uses printf for output and is intended for debugging
 *       purposes only. In a production system, consider using a logging
 *       framework or redirecting output to a debug stream.
 * 
 * @warning This function performs pointer arithmetic on GrowableArray data
 *          and assumes the arrays contain Symbol and Scope* elements respectively.
 *          Ensure the arrays are properly initialized before calling this function.
 */
void debug_print_scope(Scope *scope, int indent_level) {
    // Print current scope header with metadata
    for (int i = 0; i < indent_level; i++) printf("  ");
    printf("Scope '%s' (depth %zu, %zu symbols, %zu children):\n", 
           scope->scope_name, scope->depth, scope->symbols.count, scope->children.count);
    
    // Print all symbols in the current scope
    for (size_t i = 0; i < scope->symbols.count; i++) {
        // Calculate symbol pointer using array base + offset
        Symbol *s = (Symbol *)((char*)scope->symbols.data + i * sizeof(Symbol));
        
        // Indent for symbol display
        for (int j = 0; j < indent_level + 1; j++) printf("  ");
        
        // Print symbol information with type name (simplified for basic types)
        printf("- %s: %s (public: %d, mutable: %d)\n", 
               s->name, 
               s->type && s->type->category == Node_Category_TYPE && s->type->type == AST_TYPE_BASIC 
                   ? s->type->type_data.basic.name : "complex_type",
               s->is_public, s->is_mutable);
    }
    
    // Recursively print all child scopes if any exist
    if (scope->children.count > 0) {
        for (int i = 0; i < indent_level + 1; i++) printf("  ");
        printf("Child scopes:\n");
        
        for (size_t i = 0; i < scope->children.count; i++) {
            // Calculate child scope pointer using array base + offset
            Scope **child_ptr = (Scope **)((char*)scope->children.data + i * sizeof(Scope*));
            Scope *child = *child_ptr;
            
            // Recursive call with increased indentation
            debug_print_scope(child, indent_level + 2);
        }
    }
}