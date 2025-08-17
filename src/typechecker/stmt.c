#include <stdio.h>
#include <string.h>

#include "type.h"

bool typecheck_var_decl(AstNode *node, Scope *scope, ArenaAllocator *arena) {
  const char *name = node->stmt.var_decl.name;
  AstNode *declared_type = node->stmt.var_decl.var_type;
  AstNode *initializer = node->stmt.var_decl.initializer;

  // If there's an initializer, check its type matches the declared type
  if (initializer) {
    AstNode *init_type = typecheck_expression(initializer, scope, arena);
    if (!init_type)
      return false;

    if (declared_type) {
      TypeMatchResult match = types_match(declared_type, init_type);
      if (match == TYPE_MATCH_NONE) {
        fprintf(
            stderr,
            "Error: Type mismatch in variable declaration '%s' at line %zu\n",
            name, node->line);
        return false;
      }
    } else {
      // Type inference - use the initializer's type
      declared_type = init_type;
    }
  }

  if (!declared_type) {
    fprintf(stderr,
            "Error: Variable '%s' has no type information at line %zu\n", name,
            node->line);
    return false;
  }

  return scope_add_symbol(scope, name, declared_type,
                          node->stmt.var_decl.is_public,
                          node->stmt.var_decl.is_mutable, arena);
}

// Stub implementations for remaining functions
bool typecheck_func_decl(AstNode *node, Scope *scope, ArenaAllocator *arena) {
  const char *name = node->stmt.func_decl.name;
  AstNode *return_type = node->stmt.func_decl.return_type;
  AstNode **param_types = node->stmt.func_decl.param_types;
  char **param_names = node->stmt.func_decl.param_names;
  size_t param_count = node->stmt.func_decl.param_count;
  AstNode *body = node->stmt.func_decl.body;

  // 1. Validate return type exists and is valid
  if (!return_type || return_type->category != Node_Category_TYPE) {
    fprintf(stderr,
            "Error: Function '%s' has invalid return type at line %zu\n", name,
            node->line);
    return false;
  }

  // 2. Validate all parameter types
  for (size_t i = 0; i < param_count; i++) {
    const char *p_name = param_names[i];
    AstNode *p_type = param_types[i];

    if (!p_name || !p_type) {
      fprintf(stderr,
              "Error: Function '%s' has invalid parameter %zu at line %zu\n",
              name, i, node->line);
      return false;
    }

    if (p_type->category != Node_Category_TYPE) {
      fprintf(stderr,
              "Error: Parameter '%s' in function '%s' has invalid type at line "
              "%zu\n",
              p_name, name, node->line);
      return false;
    }
  }

  // 3. Create function type for the symbol table
  AstNode *func_type = create_function_type(
      arena, param_types, param_count, return_type, node->line, node->column);

  // 4. Add function to current scope BEFORE checking body (for recursion)
  if (!scope_add_symbol(scope, name, func_type, node->stmt.func_decl.is_public,
                        false, arena)) {
    return false;
  }

  // 5. Create function scope for parameters and body
  Scope *func_scope = create_child_scope(scope, name, arena);
  func_scope->is_function_scope = true;
  func_scope->associated_node = node;

  // 6. Add parameters to function scope
  for (size_t i = 0; i < param_count; i++) {
    const char *p_name = param_names[i];
    AstNode *p_type = param_types[i];

    if (!scope_add_symbol(func_scope, p_name, p_type, false, true, arena)) {
      fprintf(stderr,
              "Error: Could not add parameter '%s' to function '%s' scope\n",
              p_name, name);
      return false;
    }
  }

  // 7. Typecheck function body in the function scope
  if (body) {
    if (!typecheck_statement(body, func_scope, arena)) {
      fprintf(stderr, "Error: Function '%s' body failed typechecking\n", name);
      return false;
    }
  }
  return true;
}

bool typecheck_struct_decl(AstNode *node, Scope *scope, ArenaAllocator *arena) {
  // TODO: Implement struct declaration typechecking
  const char *name = node->stmt.struct_decl.name;
  return scope_add_symbol(scope, name, create_basic_type(arena, "struct", 0, 0),
                          node->stmt.struct_decl.is_public, false, arena);
}

bool typecheck_enum_decl(AstNode *node, Scope *scope, ArenaAllocator *arena) {
    const char *enum_name = node->stmt.enum_decl.name;
    char **member_names = node->stmt.enum_decl.members;
    size_t member_count = node->stmt.enum_decl.member_count;
    
    // 1. Add enum name as an int type (for variable declarations like "Color c;")
    AstNode *int_type = create_basic_type(arena, "int", node->line, node->column);
    if (!scope_add_symbol(scope, enum_name, int_type, 
                         node->stmt.enum_decl.is_public, false, arena)) {
        return false;
    }
    
    // 2. Add each member as an int constant with qualified names
    for (size_t i = 0; i < member_count; i++) {
        size_t qualified_len = strlen(enum_name) + strlen(member_names[i]) + 2;
        char *qualified_name = arena_alloc(arena, qualified_len, 1);
        snprintf(qualified_name, qualified_len, "%s.%s", enum_name, member_names[i]);
        
        // Each enum member is just an int constant
        if (!scope_add_symbol(scope, qualified_name, int_type, true, false, arena)) {
            fprintf(stderr, "Error: Could not add enum member '%s'\n", qualified_name);
            return false;
        }
    }
    
    return true;
}
bool typecheck_return_decl(AstNode *node, Scope *scope, ArenaAllocator *arena) {
  // Find the enclosing function's return type
  AstNode *expected_return_type = get_enclosing_function_return_type(scope);
  if (!expected_return_type) {
    fprintf(stderr, "Error: Return statement outside of function at line %zu\n",
            node->line);
    return false;
  }

  AstNode *return_value = node->stmt.return_stmt.value;

  // Check if function expects void
  bool expects_void =
      (expected_return_type->type == AST_TYPE_BASIC &&
       strcmp(expected_return_type->type_data.basic.name, "void") == 0);

  if (expects_void && return_value != NULL) {
    fprintf(stderr, "Error: Void function cannot return a value at line %zu\n",
            node->line);
    return false;
  }

  if (!expects_void) {
    if (!return_value) {
      fprintf(stderr,
              "Error: Non-void function must return a value at line %zu\n",
              node->line);
      return false;
    }

    // Typecheck with current scope where x is visible
    AstNode *actual_return_type =
        typecheck_expression(return_value, scope, arena);
    if (!actual_return_type)
      return false;

    TypeMatchResult match =
        types_match(expected_return_type, actual_return_type);
    if (match == TYPE_MATCH_NONE) {
      fprintf(stderr, "Error: Return type mismatch at line %zu\n", node->line);
      return false;
    }
  }

  return true;
}

bool typecheck_if_decl(AstNode *node, Scope *scope, ArenaAllocator *arena) {
  Scope *then_branch = create_child_scope(scope, "then_branch", arena);
  Scope *else_branch = create_child_scope(scope, "else_branch", arena);

  Type *expected =
      create_basic_type(arena, "bool", node->stmt.if_stmt.condition->line,
                        node->stmt.if_stmt.condition->column);
  Type *user = typecheck_expression(node->stmt.if_stmt.condition, scope, arena);
  TypeMatchResult condition = types_match(expected, user);
  if (condition == TYPE_MATCH_NONE) {
    fprintf(stderr,
            "Error: If condition expected to be of type 'bool', but got '%s' "
            "instead\n",
            type_to_string(user, arena));
    return false;
  }

  typecheck_statement(node->stmt.if_stmt.then_stmt, then_branch, arena);

  // TODO: Handle elif cases
  // for (int i = 0; i < node->stmt.if_stmt.elif_count; i++) {
  //   typecheck_statement(node->stmt.if_stmt.elif_stmts[i], then_branch,
  //   arena);
  // }

  typecheck_statement(node->stmt.if_stmt.else_stmt, else_branch, arena);

  return true;
}