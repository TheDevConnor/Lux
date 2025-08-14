#include <stdio.h>

#include "type.h"

bool typecheck(AstNode *node, Scope *scope, ArenaAllocator *arena) {
    if (!node) {
        fprintf(stderr, "Error: Null AST node\n");
        return false;
    }
    
    switch (node->category) {
        case Node_Category_STMT:
            return typecheck_statement(node, scope, arena);
        
        case Node_Category_EXPR: {
            AstNode *result_type = typecheck_expression(node, scope, arena);
            return result_type != NULL;
        }
        
        case Node_Category_TYPE:
            // Types themselves don't need typechecking, they're valid by construction
            return true;
        
        default:
            fprintf(stderr, "Error: Unknown node category\n");
            return false;
    }
}
