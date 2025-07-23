#include "ast_utils.h"
#include "ast.h"
#include <string.h>

static void indent(int level) {
  for (int i = 0; i < level; ++i) printf("  ");
}

const char *node_type_to_string(NodeType type) {
  switch (type) {
    case AST_EXPR_LITERAL: return "Literal";
    case AST_EXPR_IDENTIFIER: return "Identifier";
    case AST_EXPR_BINARY: return "Binary";
    case AST_EXPR_UNARY: return "Unary";
    case AST_EXPR_CALL: return "Call";
    case AST_EXPR_ASSIGNMENT: return "Assignment";
    case AST_EXPR_TERNARY: return "Ternary";
    case AST_EXPR_MEMBER: return "Member";
    case AST_EXPR_INDEX: return "Index";
    case AST_EXPR_GROUPING: return "Grouping";
    case AST_STMT_EXPRESSION: return "ExprStmt";
    case AST_STMT_VAR_DECL: return "VarDecl";
    case AST_STMT_CONST_DECL: return "ConstDecl";
    case AST_STMT_FUNCTION: return "Function";
    case AST_STMT_IF: return "If";
    case AST_STMT_LOOP: return "Loop";
    case AST_STMT_RETURN: return "Return";
    case AST_STMT_BREAK: return "Break";
    case AST_STMT_CONTINUE: return "Continue";
    case AST_STMT_BLOCK: return "Block";
    case AST_STMT_PRINT: return "Print";
    case AST_STMT_MODULE: return "Module";
    case AST_STMT_ENUM: return "Enum";
    case AST_STMT_STRUCT: return "Struct";
    case AST_TYPE_BASIC: return "TypeBasic";
    case AST_TYPE_POINTER: return "TypePointer";
    case AST_TYPE_ARRAY: return "TypeArray";
    case AST_TYPE_FUNCTION: return "TypeFunction";
    case AST_TYPE_STRUCT: return "TypeStruct";
    case AST_TYPE_ENUM: return "TypeEnum";
    default: return "Unknown";
  }
}

const char *binop_to_string(BinaryOp op) {
  switch (op) {
    case BINOP_ADD: return "+";
    case BINOP_SUB: return "-";
    case BINOP_MUL: return "*";
    case BINOP_DIV: return "/";
    case BINOP_MOD: return "%";
    case BINOP_POW: return "**";
    case BINOP_EQ: return "==";
    case BINOP_NE: return "!=";
    case BINOP_LT: return "<";
    case BINOP_LE: return "<=";
    case BINOP_GT: return ">";
    case BINOP_GE: return ">=";
    case BINOP_AND: return "&&";
    case BINOP_OR: return "||";
    case BINOP_BIT_AND: return "&";
    case BINOP_BIT_OR: return "|";
    case BINOP_BIT_XOR: return "^";
    case BINOP_SHL: return "<<";
    case BINOP_SHR: return ">>";
    default: return "??";
  }
}

const char *unop_to_string(UnaryOp op) {
  switch (op) {
    case UNOP_NOT: return "!";
    case UNOP_NEG: return "-";
    case UNOP_POS: return "+";
    case UNOP_BIT_NOT: return "~";
    case UNOP_PRE_INC: return "++";
    case UNOP_PRE_DEC: return "--";
    case UNOP_POST_INC: return "x++";
    case UNOP_POST_DEC: return "x--";
    case UNOP_DEREF: return "*";
    case UNOP_ADDR: return "&";
    default: return "??";
  }
}

const char *literal_type_to_string(LiteralType type) {
  switch (type) {
    case LITERAL_INT: return "int";
    case LITERAL_FLOAT: return "float";
    case LITERAL_STRING: return "string";
    case LITERAL_CHAR: return "char";
    case LITERAL_BOOL: return "bool";
    case LITERAL_NULL: return "null";
    default: return "unknown";
  }
}

void print_ast(const AstNode *node, int indent_level) {
  if (!node) {
    indent(indent_level);
    printf("<null>\n");
    return;
  }

  indent(indent_level);
  printf("[%s] @ line %zu\n", node_type_to_string(node->type), node->line);

  if (IS_LITERAL(node)) {
    indent(indent_level + 1);
    printf("Literal (%s): ", literal_type_to_string(node->expr.literal.lit_type));
    switch (node->expr.literal.lit_type) {
      case LITERAL_INT: printf("%lld\n", node->expr.literal.value.int_val); break;
      case LITERAL_FLOAT: printf("%f\n", node->expr.literal.value.float_val); break;
      case LITERAL_STRING: printf("\"%s\"\n", node->expr.literal.value.string_val); break;
      case LITERAL_CHAR: printf("'%c'\n", node->expr.literal.value.char_val); break;
      case LITERAL_BOOL: printf(node->expr.literal.value.bool_val ? "true\n" : "false\n"); break;
      case LITERAL_NULL: printf("null\n"); break;
    }
  } else if (IS_BINARY(node)) {
    indent(indent_level + 1);
    printf("Operator: %s\n", binop_to_string(node->expr.binary.op));
    print_ast(node->expr.binary.left, indent_level + 2);
    print_ast(node->expr.binary.right, indent_level + 2);
  } else if (IS_UNARY(node)) {
    indent(indent_level + 1);
    printf("Operator: %s\n", unop_to_string(node->expr.unary.op));
    print_ast(node->expr.unary.operand, indent_level + 2);
  } else if (IS_CALL(node)) {
    indent(indent_level + 1);
    printf("Function Call:\n");
    print_ast(node->expr.call.callee, indent_level + 2);
    for (size_t i = 0; i < node->expr.call.arg_count; ++i) {
      print_ast(node->expr.call.args[i], indent_level + 2);
    }
  } else if (IS_VAR_DECL(node)) {
    indent(indent_level + 1);
    printf("VarDecl: %s %s\n", node->stmt.var_decl.is_mutable ? "mut" : "const", node->stmt.var_decl.name);
    if (node->stmt.var_decl.var_type) {
      indent(indent_level + 2);
      printf("Type:\n");
      print_ast(node->stmt.var_decl.var_type, indent_level + 3);
    }
    if (node->stmt.var_decl.initializer) {
      indent(indent_level + 2);
      printf("Initializer:\n");
      print_ast(node->stmt.var_decl.initializer, indent_level + 3);
    }
  } else if (node->type == AST_STMT_BLOCK) {
    indent(indent_level);
    printf("Block:\n");
    for (size_t i = 0; i < node->stmt.block.stmt_count; ++i) {
      print_ast(node->stmt.block.statements[i], indent_level + 1);
    }
  }
  // More node types can be added similarly as needed.
}
