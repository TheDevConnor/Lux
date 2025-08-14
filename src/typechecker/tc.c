#include <stdio.h>

#include "type.h"

bool typecheck(AstNode *node, Scope *scope, ArenaAllocator *arena) {
    // This function will implement the type checking logic
    // For now, we will just return true to indicate success
    // In a real implementation, this would contain the logic to check types
    // based on the AST node and the current scope.

    scope_add_symbol(scope, "x", NULL, true, true, arena);
    scope_add_symbol(scope, "y", NULL, false, true, arena);

    Symbol *s = scope_lookup(scope, "x");
    if (s) {
        printf("Found: %s of type %p, public: %d, mutable: %d\n",
               s->name, s->type, s->is_public, s->is_mutable);
    }

    s = scope_lookup(scope, "y");
    if (s) {
        printf("Found: %s of type %p, public: %d, mutable: %d\n",
               s->name, s->type, s->is_public, s->is_mutable);
    }
    
    // Example: Check if node is NULL
    if (node == NULL) {
        return false; // Error: Null node
    }

    // Placeholder for actual type checking logic
    return true; // Indicate successful type checking
}