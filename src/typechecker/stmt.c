#include <stdio.h>

#include "type.h"

bool typecheck_statement(AstNode *stmt, Scope *scope, ArenaAllocator *arena) {
    switch (stmt->type) {
        case AST_PROGRAM:
            for (size_t i = 0; i < stmt->stmt.program.stmt_count; i++) {
                if (!typecheck(stmt->stmt.program.statements[i], scope, arena)) {
                    return false;
                }
            }
            return true;
        
        case AST_STMT_VAR_DECL:
            return typecheck_var_decl(stmt, scope, arena);
        
        case AST_STMT_FUNCTION:
            return typecheck_func_decl(stmt, scope, arena);
        
        case AST_STMT_STRUCT:
            return typecheck_struct_decl(stmt, scope, arena);
        
        case AST_STMT_ENUM:
            return typecheck_enum_decl(stmt, scope, arena);
        
        case AST_STMT_EXPRESSION: {
            AstNode *expr_type = typecheck_expression(stmt->stmt.expr_stmt.expression, scope, arena);
            return expr_type != NULL;
        }
        
        case AST_STMT_RETURN: {
            if (stmt->stmt.return_stmt.value) {
                AstNode *return_type = typecheck_expression(stmt->stmt.return_stmt.value, scope, arena);
                return return_type != NULL;
                // TODO: Check against function's declared return type
            }
            return true;
        }
        
        case AST_STMT_BLOCK: {
            // Create new scope for block
            Scope *block_scope = create_child_scope(scope, "block", arena);
            for (size_t i = 0; i < stmt->stmt.block.stmt_count; i++) {
                if (!typecheck(stmt->stmt.block.statements[i], block_scope, arena)) {
                    return false;
                }
            }
            return true;
        }
        
        default:
            printf("Warning: Unhandled statement type %d\n", stmt->type);
            return true; // Don't fail on unimplemented statements yet
    }
}

bool typecheck_var_decl(AstNode *node, Scope *scope, ArenaAllocator *arena) {
    const char *name = node->stmt.var_decl.name;
    AstNode *declared_type = node->stmt.var_decl.var_type;
    AstNode *initializer = node->stmt.var_decl.initializer;
    
    // If there's an initializer, check its type matches the declared type
    if (initializer) {
        AstNode *init_type = typecheck_expression(initializer, scope, arena);
        if (!init_type) return false;
        
        if (declared_type) {
            TypeMatchResult match = types_match(declared_type, init_type);
            if (match == TYPE_MATCH_NONE) {
                fprintf(stderr, "Error: Type mismatch in variable declaration '%s' at line %zu\n",
                       name, node->line);
                return false;
            }
        } else {
            // Type inference - use the initializer's type
            declared_type = init_type;
        }
    }
    
    if (!declared_type) {
        fprintf(stderr, "Error: Variable '%s' has no type information at line %zu\n",
               name, node->line);
        return false;
    }
    
    return scope_add_symbol(scope, name, declared_type,
                           node->stmt.var_decl.is_public,
                           node->stmt.var_decl.is_mutable, arena);
}

// Utility functions

// Stub implementations for remaining functions
bool typecheck_func_decl(AstNode *node, Scope *scope, ArenaAllocator *arena) {
    // TODO: Implement function declaration typechecking
    const char *name = node->stmt.func_decl.name;
    return scope_add_symbol(scope, name, create_basic_type(arena, "function", 0, 0),
                           node->stmt.func_decl.is_public, false, arena);
}

bool typecheck_struct_decl(AstNode *node, Scope *scope, ArenaAllocator *arena) {
    // TODO: Implement struct declaration typechecking
    const char *name = node->stmt.struct_decl.name;
    return scope_add_symbol(scope, name, create_basic_type(arena, "struct", 0, 0),
                           node->stmt.struct_decl.is_public, false, arena);
}

bool typecheck_enum_decl(AstNode *node, Scope *scope, ArenaAllocator *arena) {
    // TODO: Implement enum declaration typechecking
    const char *name = node->stmt.enum_decl.name;
    return scope_add_symbol(scope, name, create_basic_type(arena, "enum", 0, 0),
                           node->stmt.enum_decl.is_public, false, arena);
}
