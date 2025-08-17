#include <stdio.h>
#include <string.h>

#include "type.h"

AstNode *typecheck_binary_expr(AstNode *expr, Scope *scope, ArenaAllocator *arena) {
    AstNode *left_type = typecheck_expression(expr->expr.binary.left, scope, arena);
    AstNode *right_type = typecheck_expression(expr->expr.binary.right, scope, arena);
    
    if (!left_type || !right_type) return NULL;
    
    BinaryOp op = expr->expr.binary.op;
    
    // Arithmetic operators
    if (op >= BINOP_ADD && op <= BINOP_POW) {
        if (!is_numeric_type(left_type) || !is_numeric_type(right_type)) {
            fprintf(stderr, "Error: Arithmetic operation on non-numeric types at line %zu\n", 
                   expr->line);
            return NULL;
        }
        
        // Return the "wider" type (float > int)
        if (types_match(left_type, create_basic_type(arena, "float", 0, 0)) == TYPE_MATCH_EXACT ||
            types_match(right_type, create_basic_type(arena, "float", 0, 0)) == TYPE_MATCH_EXACT) {
            return create_basic_type(arena, "float", expr->line, expr->column);
        }
        return create_basic_type(arena, "int", expr->line, expr->column);
    }
    
    // Comparison operators
    if (op >= BINOP_EQ && op <= BINOP_GE) {
        TypeMatchResult match = types_match(left_type, right_type);
        if (match == TYPE_MATCH_NONE) {
            fprintf(stderr, "Error: Cannot compare incompatible types at line %zu\n", 
                   expr->line);
            return NULL;
        }
        return create_basic_type(arena, "bool", expr->line, expr->column);
    }
    
    // Logical operators
    if (op == BINOP_AND || op == BINOP_OR) {
        // In many languages, these work with any type (truthy/falsy)
        return create_basic_type(arena, "bool", expr->line, expr->column);
    }
    
    return NULL;
}

AstNode *typecheck_call_expr(AstNode *expr, Scope *scope, ArenaAllocator *arena) {
    // TODO: Implement function call typechecking
    return create_basic_type(arena, "unknown", expr->line, expr->column);
}

AstNode *typecheck_member_expr(AstNode *expr, Scope *scope, ArenaAllocator *arena) {
    // Handle Color.RED syntax
    const char *base_name = expr->expr.member.object->expr.identifier.name;
    const char *member_name = expr->expr.member.member;
    
    // Build qualified name and look it up directly
    size_t qualified_len = strlen(base_name) + strlen(member_name) + 2;
    char *qualified_name = arena_alloc(arena, qualified_len, 1);
    snprintf(qualified_name, qualified_len, "%s.%s", base_name, member_name);
    
    Symbol *member_symbol = scope_lookup(scope, qualified_name);
    if (!member_symbol) {
        // Check if the base name exists to give a better error message
        Symbol *base_symbol = scope_lookup(scope, base_name);
        if (!base_symbol) {
            fprintf(stderr, "Error: Undefined identifier '%s' at line %zu\n", 
                    base_name, expr->line);
        } else {
            fprintf(stderr, "Error: '%s' has no member '%s' at line %zu\n", 
                    base_name, member_name, expr->line);
        }
        return NULL;
    }
    
    return member_symbol->type;
}