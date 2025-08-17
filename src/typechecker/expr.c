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
    const char *func_name = expr->expr.call.callee->expr.identifier.name;
    AstNode **arguments = expr->expr.call.args;
    size_t arg_count = expr->expr.call.arg_count;
    
    // 1. Look up the function symbol
    Symbol *func_symbol = scope_lookup(scope, func_name);
    if (!func_symbol) {
        fprintf(stderr, "Error: Undefined function '%s' at line %zu\n", 
               func_name, expr->line);
        return NULL;
    }
    
    // 2. Verify it's a function type
    if (func_symbol->type->type != AST_TYPE_FUNCTION) {
        fprintf(stderr, "Error: '%s' is not a function at line %zu\n", 
               func_name, expr->line);
        return NULL;
    }
    
    AstNode *func_type = func_symbol->type;
    AstNode **param_types = func_type->type_data.function.param_types;
    size_t param_count = func_type->type_data.function.param_count;
    AstNode *return_type = func_type->type_data.function.return_type;
    
    // 3. Check argument count
    if (arg_count != param_count) {
        fprintf(stderr, "Error: Function '%s' expects %zu arguments, got %zu at line %zu\n",
               func_name, param_count, arg_count, expr->line);
        return NULL;
    }
    
    // 4. Type-check each argument
    for (size_t i = 0; i < arg_count; i++) {
        AstNode *arg_type = typecheck_expression(arguments[i], scope, arena);
        if (!arg_type) {
            fprintf(stderr, "Error: Failed to type-check argument %zu in call to '%s'\n",
                   i + 1, func_name);
            return NULL;
        }
        
        TypeMatchResult match = types_match(param_types[i], arg_type);
        if (match == TYPE_MATCH_NONE) {
            fprintf(stderr, "Error: Argument %zu to function '%s' has wrong type. "
                           "Expected '%s', got '%s' at line %zu\n",
                   i + 1, func_name, 
                   type_to_string(param_types[i], arena),
                   type_to_string(arg_type, arena),
                   expr->line);
            return NULL;
        }
    }
    
    // 5. Return the function's return type
    return return_type;
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