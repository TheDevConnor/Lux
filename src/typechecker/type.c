#include <string.h>
#include <stdio.h>

#include "type.h"

TypeMatchResult types_match(AstNode *type1, AstNode *type2) {
    if (!type1 || !type2) return TYPE_MATCH_NONE;
    if (type1 == type2) return TYPE_MATCH_EXACT;
    
    // Both must be type nodes
    if (type1->category != Node_Category_TYPE || type2->category != Node_Category_TYPE) {
        return TYPE_MATCH_NONE;
    }
    
    // Basic type matching
    if (type1->type == AST_TYPE_BASIC && type2->type == AST_TYPE_BASIC) {
        if (strcmp(type1->type_data.basic.name, type2->type_data.basic.name) == 0) {
            return TYPE_MATCH_EXACT;
        }
        
        // Check for compatible numeric types
        const char *name1 = type1->type_data.basic.name;
        const char *name2 = type2->type_data.basic.name;
        
        if ((strcmp(name1, "int") == 0 && strcmp(name2, "float") == 0) ||
            (strcmp(name1, "float") == 0 && strcmp(name2, "int") == 0)) {
            return TYPE_MATCH_COMPATIBLE;
        }
    }
    
    // Pointer type matching
    if (type1->type == AST_TYPE_POINTER && type2->type == AST_TYPE_POINTER) {
        return types_match(type1->type_data.pointer.pointee_type, 
                          type2->type_data.pointer.pointee_type);
    }
    
    // Array type matching
    if (type1->type == AST_TYPE_ARRAY && type2->type == AST_TYPE_ARRAY) {
        return types_match(type1->type_data.array.element_type,
                          type2->type_data.array.element_type);
    }
    
    return TYPE_MATCH_NONE;
}

bool is_numeric_type(AstNode *type) {
    if (!type || type->category != Node_Category_TYPE || type->type != AST_TYPE_BASIC) {
        return false;
    }
    
    const char *name = type->type_data.basic.name;
    return strcmp(name, "int") == 0 || 
           strcmp(name, "float") == 0 || 
           strcmp(name, "double") == 0 ||
           strcmp(name, "char") == 0;
}

bool is_pointer_type(AstNode *type) {
    return type && type->category == Node_Category_TYPE && type->type == AST_TYPE_POINTER;
}

bool is_array_type(AstNode *type) {
    return type && type->category == Node_Category_TYPE && type->type == AST_TYPE_ARRAY;
}

const char *type_to_string(AstNode *type, ArenaAllocator *arena) {
    if (!type || type->category != Node_Category_TYPE) {
        return arena_strdup(arena, "unknown");
    }
    
    switch (type->type) {
        case AST_TYPE_BASIC:
            return arena_strdup(arena, type->type_data.basic.name);
        
        case AST_TYPE_POINTER: {
            const char *pointee = type_to_string(type->type_data.pointer.pointee_type, arena);
            size_t len = strlen(pointee) + 2;  // +1 for '*', +1 for null terminator
            char *result = arena_alloc(arena, len, len);
            snprintf(result, len, "%s*", pointee);
            return result;
        }
        
        case AST_TYPE_ARRAY: {
            const char *element = type_to_string(type->type_data.array.element_type, arena);
            size_t len = strlen(element) + 3; // +2 for "[]", +1 for null terminator
            char *result = arena_alloc(arena, len, len);
            snprintf(result, len, "%s[]", element);
            return result;
        }
        
        default:
            return arena_strdup(arena, "unknown");
    }
}

void debug_print_scope(Scope *scope, int indent_level) {
    for (int i = 0; i < indent_level; i++) printf("  ");
    printf("Scope '%s' (depth %zu, %zu symbols):\n", 
           scope->scope_name, scope->depth, scope->symbols.count);
    
    for (size_t i = 0; i < scope->symbols.count; i++) {
        Symbol *s = (Symbol *)((char*)scope->symbols.data + i * sizeof(Symbol));
        for (int j = 0; j < indent_level + 1; j++) printf("  ");
        printf("- %s: %s (public: %d, mutable: %d)\n", 
               s->name, 
               s->type && s->type->category == Node_Category_TYPE && s->type->type == AST_TYPE_BASIC 
                   ? s->type->type_data.basic.name : "complex_type",
               s->is_public, s->is_mutable);
    }
}