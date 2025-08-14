#include <stdio.h>

#include "type.h"

Type_TC *lookup_builtin_type(const char *name, ArenaAllocator *arena) {
    static BuiltinTypes *builtin_types = NULL;
    
    if (!builtin_types) {
        builtin_types = (BuiltinTypes *)arena_alloc(arena, sizeof(BuiltinTypes), sizeof(BuiltinTypes));
        if (!builtin_types) {
        fprintf(stderr, "Out of memory while initializing builtin types\n");
        return NULL;
        }
        init_builtin_types(builtin_types);
    }
    
    if (strcmp(name, "int") == 0) return builtin_types->t_int;
    if (strcmp(name, "float") == 0) return builtin_types->t_float;
    if (strcmp(name, "bool") == 0) return builtin_types->t_bool;
    if (strcmp(name, "string") == 0) return builtin_types->t_string;
    if (strcmp(name, "void") == 0) return builtin_types->t_void;
    
    return NULL; // Not a builtin type
}


Type_TC *make_primitive(ArenaAllocator *arena, TypeKind kind, const char *name) {
  Type_TC *type = (Type_TC *)arena_alloc(arena, sizeof(Type_TC), sizeof(Type_TC));
  if (!type) {
    fprintf(stderr, "Out of memory while creating primitive type '%s'\n", name);
    return NULL;
  }
  type->kind = kind;
  type->name = arena_strdup(arena, name);
  return type;
}

void init_builtin_types(ArenaAllocator *arena) {
  BuiltinTypes *bt = (BuiltinTypes *)arena_alloc(arena, sizeof(BuiltinTypes));
  if (!bt) {
    fprintf(stderr, "Out of memory while initializing builtin types\n");
    return;
  }

  bt->t_int    = make_primitive(arena, TYPE_INT, "int");
  bt->t_float  = make_primitive(arena, TYPE_FLOAT, "float");
  bt->t_bool   = make_primitive(arena, TYPE_BOOL, "bool");
  bt->t_string = make_primitive(arena, TYPE_STRING, "string");
  bt->t_void   = make_primitive(arena, TYPE_VOID, "void");
}