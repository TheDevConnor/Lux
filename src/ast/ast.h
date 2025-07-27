#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "../c_libs/memory/memory.h"

// Forward declaration
typedef struct AstNode AstNode;

// Node type enumeration
typedef enum {
  // Expression nodes
  AST_EXPR_LITERAL,    // Literal values (numbers, strings, booleans)
  AST_EXPR_IDENTIFIER, // Variable/function names
  AST_EXPR_BINARY,     // Binary operations (+, -, *, /, etc.)
  AST_EXPR_UNARY,      // Unary operations (!, -, ++, --)
  AST_EXPR_CALL,       // Function calls
  AST_EXPR_ASSIGNMENT, // Assignment expressions
  AST_EXPR_TERNARY,    // Conditional expressions (? :)
  AST_EXPR_MEMBER,     // Member access (obj.field)
  AST_EXPR_INDEX,      // Array/object indexing (obj[index])
  AST_EXPR_GROUPING,   // Parenthesized expressions

  // Statement nodes
  AST_PROGRAM,         // Program root node
  AST_STMT_EXPRESSION, // Expression statements
  AST_STMT_VAR_DECL,   // Variable declarations
  AST_STMT_CONST_DECL, // Constant declarations
  AST_STMT_FUNCTION,   // Function definitions
  AST_STMT_IF,         // If statements
  AST_STMT_LOOP,        // Loop statements (while, for)
  AST_STMT_RETURN,     // Return statements
  AST_STMT_BREAK,      // Break statements
  AST_STMT_CONTINUE,   // Continue statements
  AST_STMT_BLOCK,      // Block statements
  AST_STMT_PRINT,      // Print statements
  AST_STMT_MODULE,     // Module declarations
  AST_STMT_ENUM,       // Enum declarations
  AST_STMT_STRUCT,     // Struct declarations

  // Type nodes
  AST_TYPE_BASIC,    // Basic types (int, float, string, etc.)
  AST_TYPE_POINTER,  // Pointer types
  AST_TYPE_ARRAY,    // Array types
  AST_TYPE_FUNCTION, // Function types
  AST_TYPE_STRUCT,   // Struct types
  AST_TYPE_ENUM,     // Enum types
} NodeType;

// Literal types
typedef enum {
  LITERAL_IDENT,
  LITERAL_INT,
  LITERAL_FLOAT,
  LITERAL_STRING,
  LITERAL_CHAR,
  LITERAL_BOOL,
  LITERAL_NULL
} LiteralType;

// Binary operators
typedef enum {
  BINOP_ADD,     // +
  BINOP_SUB,     // -
  BINOP_MUL,     // *
  BINOP_DIV,     // /
  BINOP_MOD,     // %
  BINOP_POW,     // **
  BINOP_EQ,      // ==
  BINOP_NE,      // !=
  BINOP_LT,      // <
  BINOP_LE,      // <=
  BINOP_GT,      // >
  BINOP_GE,      // >=
  BINOP_AND,     // &&
  BINOP_OR,      // ||
  BINOP_BIT_AND, // &
  BINOP_BIT_OR,  // |
  BINOP_BIT_XOR, // ^
  BINOP_SHL,     // <<
  BINOP_SHR,     // >>
} BinaryOp;

// Unary operators
typedef enum {
  UNOP_NOT,      // !
  UNOP_NEG,      // -
  UNOP_POS,      // +
  UNOP_BIT_NOT,  // ~
  UNOP_PRE_INC,  // ++x
  UNOP_PRE_DEC,  // --x
  UNOP_POST_INC, // x++
  UNOP_POST_DEC, // x--
  UNOP_DEREF,    // *x
  UNOP_ADDR,     // &x
} UnaryOp;

typedef enum {
  Node_Category_EXPR,
  Node_Category_STMT,
  Node_Category_TYPE
} NodeCategory;

// Base AST node structure
struct AstNode {
  NodeType type;
  size_t line;
  size_t column;
  NodeCategory category; // Category of the node (expression, statement, type)

  union {
    struct {
      // Expression-specific data
      union {
        // Literal expression
        struct {
          LiteralType lit_type;
          union {
            long long int_val;
            double float_val;
            char *string_val;
            char char_val;
            bool bool_val;
          } value;
        } literal;

        // Identifier expression
        struct {
          char *name;
        } identifier;

        // Binary expression
        struct {
          BinaryOp op;
          AstNode *left;  // Changed from Expr* to AstNode*
          AstNode *right; // Changed from Expr* to AstNode*
        } binary;

        // Unary expression
        struct {
          UnaryOp op;
          AstNode *operand; // Changed from Expr* to AstNode*
        } unary;

        // Function call expression
        struct {
          AstNode *callee; // Changed from Expr* to AstNode*
          AstNode **args;  // Changed from Expr** to AstNode**
          size_t arg_count;
        } call;

        // Assignment expression
        struct {
          AstNode *target; // Changed from Expr* to AstNode*
          AstNode *value;  // Changed from Expr* to AstNode*
        } assignment;

        // Ternary expression
        struct {
          AstNode *condition; // Changed from Expr* to AstNode*
          AstNode *then_expr; // Changed from Expr* to AstNode*
          AstNode *else_expr; // Changed from Expr* to AstNode*
        } ternary;

        // Member access expression
        struct {
          AstNode *object; // Changed from Expr* to AstNode*
          char *member;
        } member;

        // Index expression
        struct {
          AstNode *object; // Changed from Expr* to AstNode*
          AstNode *index;  // Changed from Expr* to AstNode*
        } index;

        // Grouping expression
        struct {
          AstNode *expr; // Changed from Expr* to AstNode*
        } grouping;
      };
    } expr;

    struct {
      // Statement-specific data
      union {
        // Program root node
        struct {
          AstNode **statements; // Changed from Stmt** to AstNode**
          size_t stmt_count;
        } program;

        // Expression statement
        struct {
          AstNode *expression; // Changed from Expr* to AstNode*
        } expr_stmt;

        // Variable declaration
        struct {
          const char *name;
          AstNode *var_type;    // Changed from Type* to AstNode*
          AstNode *initializer; // Changed from Expr* to AstNode*
          bool is_mutable; // Whether the variable is mutable
        } var_decl;

        // Enumeration declaration
        struct {
          const char *name;
          char **members; // Changed from char** to AstNode**
          size_t member_count;
        } enum_decl;

        // Function declaration
        struct {
          const char *name;
          char **param_names;
          AstNode **param_types; // Changed from Type** to AstNode**
          size_t param_count;
          AstNode *return_type; // Changed from Type* to AstNode*
          AstNode *body;        // Changed from Stmt* to AstNode*
        } func_decl;

        // If statement
        struct {
          AstNode *condition; // Changed from Expr* to AstNode*
          AstNode *then_stmt; // Changed from Stmt* to AstNode*
          AstNode *else_stmt; // Changed from Stmt* to AstNode*
        } if_stmt;

        // Loop statement (Combined while and for)
        struct {
          AstNode *condition; // Changed from Expr* to AstNode*
          AstNode *body;      // Changed from Stmt* to AstNode*
          // For loops can be represented as a while loop with an initializer and increment
          AstNode *initializer; // Optional initializer for for loops (Changed from Stmt* to AstNode*)
          AstNode *increment;  // Optional increment for for loops (Changed from Stmt* to AstNode*)
        } loop_stmt;

        // Return statement
        struct {
          AstNode *value; // Changed from Expr* to AstNode*
        } return_stmt;

        // Block statement
        struct {
          AstNode **statements; // Changed from Stmt** to AstNode**
          size_t stmt_count;
        } block;

        // Print statement
        struct {
          AstNode **expressions; // Changed from Expr** to AstNode**
          size_t expr_count;
        } print_stmt;
      };
    } stmt;

    struct {
      // Type-specific data
      union {
        // Basic type
        struct {
          const char *name;
        } basic;

        // Pointer type
        struct {
          AstNode *pointee_type; // Changed from Type* to AstNode*
        } pointer;

        // Array type
        struct {
          AstNode *element_type; // Changed from Type* to AstNode*
          AstNode *size;         // Changed from Expr* to AstNode*
        } array;

        // Function type
        struct {
          AstNode **param_types; // Changed from Type** to AstNode**
          size_t param_count;
          AstNode *return_type; // Changed from Type* to AstNode*
        } function;
      };
    } type_data;
  };
};

// Type aliases for cleaner code (defined AFTER the struct)
typedef AstNode Expr;
typedef AstNode Stmt;
typedef AstNode Type;

AstNode *create_expr_node(ArenaAllocator *arena, NodeType type, size_t line, size_t column);
AstNode *create_stmt_node(ArenaAllocator *arena, NodeType type, size_t line, size_t column);
AstNode *create_type_node(ArenaAllocator *arena, NodeType type, size_t line, size_t column);

// Helper macros for creating nodes
#define create_expr(arena, type, line, column) create_expr_node(arena, type, line, column)
#define create_stmt(arena, type, line, column) create_stmt_node(arena, type, line, column)
#define create_type(arena, type, line, column) create_type_node(arena, type, line, column)

// Create the AstNode
AstNode *create_ast_node(ArenaAllocator *arena, NodeType type, NodeCategory category, size_t line, size_t column);

// Expression creation macros
AstNode *create_literal_expr(ArenaAllocator *arena, LiteralType lit_type, void *value, size_t line, size_t column);
AstNode *create_identifier_expr(ArenaAllocator *arena, const char *name, size_t line, size_t column);
AstNode *create_binary_expr(ArenaAllocator *arena, BinaryOp op, Expr *left, Expr *right, size_t line, size_t column);
AstNode *create_unary_expr(ArenaAllocator *arena, UnaryOp op, Expr *operand, size_t line, size_t column);
AstNode *create_call_expr(ArenaAllocator *arena, Expr *callee, Expr **args, size_t arg_count, size_t line, size_t column);
AstNode *create_assignment_expr(ArenaAllocator *arena, Expr *target, Expr *value, size_t line, size_t column);
AstNode *create_ternary_expr(ArenaAllocator *arena, Expr *condition, Expr *then_expr, Expr *else_expr, size_t line, size_t column);
AstNode *create_member_expr(ArenaAllocator *arena, Expr *object, const char *member, size_t line, size_t column);
AstNode *create_index_expr(ArenaAllocator *arena, Expr *object, Expr *index, size_t line, size_t column);
AstNode *create_grouping_expr(ArenaAllocator *arena, Expr *expr, size_t line, size_t column);

// Statement creation macros
AstNode *create_program_node(ArenaAllocator *arena, AstNode **statements, size_t stmt_count, size_t line, size_t column);
AstNode *create_expr_stmt(ArenaAllocator *arena, Expr *expression, size_t line, size_t column);
AstNode *create_var_decl_stmt(ArenaAllocator *arena, const char *name, AstNode *var_type, Expr *initializer, bool is_mutable, size_t line, size_t column);
AstNode *create_func_decl_stmt(ArenaAllocator *arena, const char *name, char **param_names, AstNode **param_types, size_t param_count, AstNode *return_type, AstNode *body, size_t line, size_t column);
AstNode *create_enum_decl_stmt(ArenaAllocator *arena, const char *name, char **members, size_t member_count, size_t line, size_t column);
AstNode *create_if_stmt(ArenaAllocator *arena, Expr *condition, AstNode *then_stmt, AstNode *else_stmt, size_t line, size_t column);
AstNode *create_loop_stmt(ArenaAllocator *arena, Expr *condition, AstNode *body, AstNode *initializer, AstNode *increment, size_t line, size_t column);
AstNode *create_return_stmt(ArenaAllocator *arena, Expr *value, size_t line, size_t column);
AstNode *create_block_stmt(ArenaAllocator *arena, AstNode **statements, size_t stmt_count, size_t line, size_t column);
AstNode *create_print_stmt(ArenaAllocator *arena, Expr **expressions, size_t expr_count, size_t line, size_t column);

// Type creation macros
AstNode *create_basic_type(ArenaAllocator *arena, const char *name, size_t line, size_t column);
AstNode *create_pointer_type(ArenaAllocator *arena, AstNode *pointee_type, size_t line, size_t column);
AstNode *create_array_type(ArenaAllocator *arena, AstNode *element_type, Expr *size, size_t line, size_t column);
AstNode *create_function_type(ArenaAllocator *arena, AstNode **param_types, size_t param_count, AstNode *return_type, size_t line, size_t column);