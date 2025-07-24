#include <stdio.h>

#include "../ast/ast.h"
#include "parser.h"

Type *pointer(Parser *parser) {
  // We already advanced past the '*' token
  Type *pointee_type = parse_type(parser);
  if (!pointee_type) {
    fprintf(stderr, "Expected type after '*'\n");
    return NULL;
  }

  AstNode *node =
      create_type_node(parser->arena, AST_TYPE_POINTER, p_current(parser).line,
                       p_current(parser).col);
  node->type_data.pointer.pointee_type = pointee_type;

  return node;
}

Type *tnud(Parser *parser) {
  switch (p_current(parser).type_) {
  case TOK_INT:
    return create_basic_type(parser->arena, "int", p_current(parser).line,
                             p_current(parser).col);
  case TOK_UINT:
    return create_basic_type(parser->arena, "uint", p_current(parser).line,
                             p_current(parser).col);
  case TOK_FLOAT:
    return create_basic_type(parser->arena, "float", p_current(parser).line,
                             p_current(parser).col);
  case TOK_BOOL:
    return create_basic_type(parser->arena, "bool", p_current(parser).line,
                             p_current(parser).col);
  case TOK_STRINGT:
    return create_basic_type(parser->arena, "str", p_current(parser).line,
                             p_current(parser).col);
  case TOK_VOID:
    return create_basic_type(parser->arena, "void", p_current(parser).line,
                             p_current(parser).col);
  case TOK_CHAR:
    return create_basic_type(parser->arena, "char", p_current(parser).line,
                             p_current(parser).col);
  case TOK_STAR:       // Pointer type
    p_advance(parser); // Consume the '*' token
    return pointer(parser);
  default:
    fprintf(stderr, "Unexpected token in type: %d\n", p_current(parser).type_);
    return NULL;
  }
}

Type *tled(Parser *parser, Type *left, BindingPower bp) {
  fprintf(stderr, "Parsing type led: %.*s\n", CURRENT_TOKEN_LENGTH(parser),
          CURRENT_TOKEN_VALUE(parser));
  return NULL; // No valid type found
}