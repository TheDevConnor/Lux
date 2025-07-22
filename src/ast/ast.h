#pragma once

#include <stdbool.h>
#include <stddef.h>

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
  AST_STMT_EXPRESSION, // Expression statements
  AST_STMT_VAR_DECL,   // Variable declarations
  AST_STMT_CONST_DECL, // Constant declarations
  AST_STMT_FUNCTION,   // Function definitions
  AST_STMT_IF,         // If statements
  AST_STMT_WHILE,      // While loops
  AST_STMT_FOR,        // For loops
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

// Base AST node structure
struct AstNode {
  NodeType type;
  size_t line;
  size_t column;

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
        // Expression statement
        struct {
          AstNode *expression; // Changed from Expr* to AstNode*
        } expr_stmt;

        // Variable declaration
        struct {
          char *name;
          AstNode *var_type;    // Changed from Type* to AstNode*
          AstNode *initializer; // Changed from Expr* to AstNode*
          bool is_mutable;
        } var_decl;

        // Function declaration
        struct {
          char *name;
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

        // While loop
        struct {
          AstNode *condition; // Changed from Expr* to AstNode*
          AstNode *body;      // Changed from Stmt* to AstNode*
        } while_stmt;

        // For loop
        struct {
          AstNode *init;      // Changed from Stmt* to AstNode*
          AstNode *condition; // Changed from Expr* to AstNode*
          AstNode *increment; // Changed from Expr* to AstNode*
          AstNode *body;      // Changed from Stmt* to AstNode*
        } for_stmt;

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
          char *name;
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
    } type_data; // Good change - avoiding name conflict with 'type' field
  };
};

// Type aliases for cleaner code (defined AFTER the struct)
typedef AstNode Expr;
typedef AstNode Stmt;
typedef AstNode Type;
