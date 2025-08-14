#pragma once

#include "../ast/ast.h"
#include "../c_libs/memory/memory.h"

typedef struct {
  const char *name;
  Type *type;
  bool is_public;  // Indicates if the symbol is public
  bool is_mutable; // Indicates if the symbol is mutable
} Symbol;

typedef struct Scope {
  struct Scope *parent;  // For nested lookups
  GrowableArray symbols; // Array of symbols in this scope
} Scope;

typedef enum {
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_STRING,
  TYPE_BOOL,
  TYPE_VOID,
  TYPE_STRUCT,
  TYPE_ARRAY,
  TYPE_FUNCTION
} TypeKind;

typedef struct {
  Type *t_int;
  Type *t_float;
  Type *t_bool;
  Type *t_string;
  Type *t_void;
} BuiltinTypes;

typedef struct {
    TypeKind kind;
    const char *name;
    // You can extend this for pointers, arrays, structs, generics...
} Type_TC;

Type_TC *make_primitive(ArenaAllocator *arena, TypeKind kind, const char *name);
Type_TC *lookup_builtin_type(const char *name, ArenaAllocator *arena);
void init_builtin_types(ArenaAllocator *arena);

void init_scope(Scope *scope, Scope *parent, ArenaAllocator *arena);
bool scope_add_symbol(Scope *scope, const char *name, Type *type,
                      bool is_public, bool is_mutable, ArenaAllocator *arena);
Symbol *scope_lookup(Scope *scope, const char *name);

bool typecheck(AstNode *node, Scope *scope, ArenaAllocator *arena);