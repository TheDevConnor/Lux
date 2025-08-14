#include <stdio.h>
#include <string.h>

#include "../c_libs/memory/memory.h"
#include "type.h"

void init_scope(Scope *scope, Scope *parent, const char *name, ArenaAllocator *arena) {
    scope->parent = parent;
    scope->scope_name = name ? arena_strdup(arena, name) : "unnamed";
    scope->depth = parent ? parent->depth + 1 : 0;
    scope->is_function_scope = false;
    scope->associated_node = NULL;
    growable_array_init(&scope->symbols, arena, 16, sizeof(Symbol));
}

bool scope_add_symbol(Scope *scope, const char *name, AstNode *type,
                     bool is_public, bool is_mutable, ArenaAllocator *arena) {
    // Check for duplicate symbols in current scope
    Symbol *existing = scope_lookup_current_only(scope, name);
    if (existing) {
        fprintf(stderr, "Error: Symbol '%s' already declared in current scope\n", name);
        return false;
    }

    Symbol *s = (Symbol *)growable_array_push(&scope->symbols);
    if (!s) {
        fprintf(stderr, "Out of memory while adding symbol '%s'\n", name);
        return false;   
    }
    
    s->name = arena_strdup(arena, name);
    s->type = type;
    s->is_public = is_public;
    s->is_mutable = is_mutable;
    s->scope_depth = scope->depth;
    return true;
}

Symbol *scope_lookup(Scope *scope, const char *name) {
    for (size_t i = 0; i < scope->symbols.count; ++i) {
        Symbol *s = (Symbol *)((char*)scope->symbols.data + i * sizeof(Symbol));
        if (strcmp(s->name, name) == 0) {
            return s;
        }
    }
    return scope->parent ? scope_lookup(scope->parent, name) : NULL;
}

Symbol *scope_lookup_current_only(Scope *scope, const char *name) {
    for (size_t i = 0; i < scope->symbols.count; ++i) {
        Symbol *s = (Symbol *)((char*)scope->symbols.data + i * sizeof(Symbol));
        if (strcmp(s->name, name) == 0) {
            return s;
        }
    }
    return NULL;
}

Scope *create_child_scope(Scope *parent, const char *name, ArenaAllocator *arena) {
    Scope *child = arena_alloc(arena, sizeof(Scope), alignof(Scope));
    init_scope(child, parent, name, arena);
    return child;
}