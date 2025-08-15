/**
 * @file scope.c
 * @brief Implementation of scope and symbol table management functions
 * 
 * This file contains the implementation of the hierarchical scope system for symbol
 * table management. It provides functionality for creating, managing, and searching
 * through nested scopes with proper parent-child relationships and symbol resolution.
 * 
 * The scope system supports:
 * - Hierarchical scope creation with automatic depth tracking
 * - Symbol addition with duplicate detection
 * - Multi-level symbol lookup with parent traversal
 * - Automatic parent-child relationship management
 */

#include <stdio.h>
#include <string.h>

#include "../c_libs/memory/memory.h"
#include "type.h"

/**
 * @brief Initialize a scope structure with enhanced metadata
 * 
 * Sets up a new scope with proper initialization of all fields, including
 * parent-child relationships, depth calculation, and growable arrays for
 * symbols and children. The scope is fully prepared for symbol addition
 * and child scope creation.
 * 
 * @param scope Pointer to the scope structure to initialize (must be pre-allocated)
 * @param parent Parent scope for hierarchical lookup (NULL for global scope)
 * @param name Descriptive name for debugging and identification (NULL becomes "unnamed")
 * @param arena Arena allocator for all memory allocations within this scope
 * 
 * @details
 * Initialization includes:
 * - Parent pointer assignment and depth calculation
 * - Name duplication into arena memory for persistence
 * - Default flag initialization (not a function scope, no associated AST node)
 * - Growable array setup with reasonable initial capacities (16 symbols, 8 children)
 * 
 * @note The scope name is duplicated using arena allocation, ensuring it remains
 *       valid for the lifetime of the scope without external string management
 * 
 * @warning The scope parameter must point to valid, allocated memory. This function
 *          does not allocate the scope structure itself, only initializes it.
 */
void init_scope(Scope *scope, Scope *parent, const char *name, ArenaAllocator *arena) {
    // Set up parent-child relationship
    scope->parent = parent;
    
    // Copy name into arena memory, defaulting to "unnamed" if NULL
    scope->scope_name = name ? arena_strdup(arena, name) : "unnamed";
    
    // Calculate depth based on parent chain (root scope has depth 0)
    scope->depth = parent ? parent->depth + 1 : 0;
    
    // Initialize scope type flags to default values
    scope->is_function_scope = false;
    scope->associated_node = NULL;
    
    // Initialize growable arrays with reasonable initial capacities
    // 16 symbols should handle most local scopes without reallocation
    growable_array_init(&scope->symbols, arena, 16, sizeof(Symbol));
    
    // 8 child scopes should handle most nested scope scenarios
    growable_array_init(&scope->children, arena, 8, sizeof(Scope*));
}

/**
 * @brief Add a symbol to the specified scope with duplicate checking
 * 
 * Creates a new symbol entry in the given scope after verifying that no symbol
 * with the same name already exists in the current scope. The symbol is fully
 * initialized with the provided type information and attributes.
 * 
 * @param scope Target scope to add the symbol to
 * @param name Symbol name identifier (will be duplicated into arena memory)
 * @param type AST node representing the symbol's type
 * @param is_public Public accessibility flag for the symbol
 * @param is_mutable Mutability flag indicating if the symbol can be modified
 * @param arena Arena allocator for memory management
 * @return true if symbol was added successfully, false on error or duplicate name
 * 
 * @details
 * Error conditions that return false:
 * - Symbol name already exists in the current scope (not parent scopes)
 * - Memory allocation failure during symbol array expansion
 * 
 * Success conditions:
 * - Symbol name is unique within the current scope
 * - All memory allocations succeed
 * - Symbol is fully initialized and added to the scope's symbol array
 * 
 * @note This function only checks for duplicates within the current scope,
 *       allowing for symbol shadowing of parent scope symbols
 * 
 * @warning The symbol name is duplicated into arena memory. The original name
 *          string can be freed or modified after this call without affecting
 *          the stored symbol.
 */
bool scope_add_symbol(Scope *scope, const char *name, AstNode *type,
                     bool is_public, bool is_mutable, ArenaAllocator *arena) {
    // Check for duplicate symbols in current scope only (shadowing is allowed)
    Symbol *existing = scope_lookup_current_only(scope, name);
    if (existing) {
        fprintf(stderr, "Error: Symbol '%s' already declared in current scope\n", name);
        return false;
    }

    // Attempt to expand the symbols array and get a pointer to the new slot
    Symbol *s = (Symbol *)growable_array_push(&scope->symbols);
    if (!s) {
        fprintf(stderr, "Out of memory while adding symbol '%s'\n", name);
        return false;   
    }
    
    // Initialize the new symbol with all provided information
    s->name = arena_strdup(arena, name);  // Duplicate name into arena memory
    s->type = type;                       // Store type AST node reference
    s->is_public = is_public;             // Set accessibility flag
    s->is_mutable = is_mutable;           // Set mutability flag
    s->scope_depth = scope->depth;        // Record the scope depth for debugging
    
    return true;
}

/**
 * @brief Look up a symbol by name with hierarchical scope traversal
 * 
 * Searches for a symbol starting from the given scope and traversing up the
 * parent chain until the symbol is found or the global scope is reached.
 * This implements the standard lexical scoping rules where inner scopes
 * can access symbols from outer scopes.
 * 
 * @param scope Starting scope for the lookup
 * @param name Symbol name to search for
 * @return Pointer to Symbol if found anywhere in the scope chain, NULL otherwise
 * 
 * @details
 * Search algorithm:
 * 1. Linear search through current scope's symbols
 * 2. If not found and parent exists, recursively search parent
 * 3. Continue until symbol found or root scope reached
 * 4. Return NULL if symbol not found in any scope
 * 
 * @note This function implements symbol shadowing - if a symbol exists in
 *       multiple scopes in the chain, the one in the innermost (current) scope
 *       is returned first
 * 
 * @complexity O(n*d) where n is average symbols per scope and d is scope depth
 * 
 * @warning This function uses pointer arithmetic on GrowableArray data.
 *          The symbols array must be properly initialized and contain Symbol elements.
 */
Symbol *scope_lookup(Scope *scope, const char *name) {
    // Linear search through current scope's symbols
    for (size_t i = 0; i < scope->symbols.count; ++i) {
        // Calculate symbol pointer using base address + offset
        Symbol *s = (Symbol *)((char*)scope->symbols.data + i * sizeof(Symbol));
        if (strcmp(s->name, name) == 0) {
            return s;  // Found in current scope
        }
    }
    
    // If not found in current scope and parent exists, search recursively
    return scope->parent ? scope_lookup(scope->parent, name) : NULL;
}

/**
 * @brief Look up a symbol only in the current scope (no parent traversal)
 * 
 * Searches for a symbol exclusively within the specified scope without
 * traversing the parent chain. Useful for checking duplicate declarations
 * and implementing scope-specific symbol resolution.
 * 
 * @param scope Scope to search in (parent scopes are ignored)
 * @param name Symbol name to search for
 * @return Pointer to Symbol if found in current scope, NULL otherwise
 * 
 * @details
 * This function is primarily used for:
 * - Duplicate symbol detection during declaration
 * - Scope-specific symbol queries
 * - Implementation of shadowing checks
 * 
 * @note Unlike scope_lookup(), this function never searches parent scopes,
 *       making it suitable for verifying symbol uniqueness within a single scope
 * 
 * @complexity O(n) where n is the number of symbols in the current scope
 * 
 * @warning This function uses pointer arithmetic on GrowableArray data.
 *          The symbols array must be properly initialized and contain Symbol elements.
 */
Symbol *scope_lookup_current_only(Scope *scope, const char *name) {
    // Linear search through current scope's symbols only
    for (size_t i = 0; i < scope->symbols.count; ++i) {
        // Calculate symbol pointer using base address + offset
        Symbol *s = (Symbol *)((char*)scope->symbols.data + i * sizeof(Symbol));
        if (strcmp(s->name, name) == 0) {
            return s;  // Found in current scope
        }
    }
    
    // Not found in current scope - do not search parents
    return NULL;
}

/**
 * @brief Create a new child scope under the specified parent
 * 
 * Allocates and initializes a new scope as a child of the given parent,
 * automatically establishing the parent-child relationship and adding
 * the new scope to the parent's children list.
 * 
 * @param parent Parent scope for the new child (NULL for root scope)
 * @param name Descriptive name for the new scope
 * @param arena Arena allocator for memory management
 * @return Pointer to the newly created and initialized child scope
 * 
 * @details
 * Creation process:
 * 1. Allocate memory for new Scope structure from arena
 * 2. Initialize the scope with proper parent linkage
 * 3. Add the new scope to parent's children array (if parent exists)
 * 4. Return pointer to fully initialized child scope
 * 
 * @note The child scope is automatically added to the parent's children list,
 *       maintaining the bidirectional parent-child relationship for debugging
 *       and traversal purposes
 * 
 * @warning If memory allocation fails, this function may return NULL or
 *          an incompletely initialized scope. The caller should verify
 *          successful creation before using the returned scope.
 * 
 * @warning If adding the child to the parent's children array fails due to
 *          memory constraints, the child scope will still be created and
 *          functional, but won't appear in the parent's children list.
 */
Scope *create_child_scope(Scope *parent, const char *name, ArenaAllocator *arena) {
    // Allocate memory for new scope structure from arena
    Scope *child = arena_alloc(arena, sizeof(Scope), alignof(Scope));
    
    // Initialize the child scope with proper parent linkage
    init_scope(child, parent, name, arena);
    
    // Add child to parent's children list for bidirectional relationship
    if (parent) {
        // Get a slot in the parent's children array
        Scope **child_ptr = (Scope **)growable_array_push(&parent->children);
        if (child_ptr) {
            *child_ptr = child;  // Store pointer to child scope
        }
        // Note: If growable_array_push fails, the child is still valid
        // but won't appear in the parent's children list
    }
    
    return child;
}