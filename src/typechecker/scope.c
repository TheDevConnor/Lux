#include <string.h>
#include <stdio.h>

#include "../c_libs/memory/memory.h"
#include "type.h"

void init_scope(Scope *scope, Scope *parent, ArenaAllocator *arena) {
    scope->parent = parent;
    growable_array_init(&scope->symbols, arena, 16, sizeof(Symbol));
}

bool scope_add_symbol(Scope *scope, const char *name, Type *type,
                      bool is_public, bool is_mutable, ArenaAllocator *arena) {
    Symbol *s = (Symbol *)growable_array_push(&scope->symbols);
    if (!s) {
        fprintf(stderr, "Out of memory while adding symbol '%s'\n", name);
        return false;   
    }
    s->name = arena_strdup(arena, name);
    s->type = type;
    s->is_public = is_public;
    s->is_mutable = is_mutable;
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