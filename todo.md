# Lux Language Compiler TODO

## ✅ Implemented (Up to Codegen)
These AST node types are fully implemented in code generation:

### Expressions
- [x] `AST_EXPR_LITERAL`
- [x] `AST_EXPR_IDENTIFIER`
- [x] `AST_EXPR_BINARY`
- [x] `AST_EXPR_UNARY`
- [x] `AST_EXPR_CALL`
- [x] `AST_EXPR_ASSIGNMENT`
- [x] `AST_EXPR_GROUPING`
- [x] `AST_EXPR_CAST`
- [x] `AST_EXPR_SIZEOF`
- [x] `AST_EXPR_ALLOC`
- [x] `AST_EXPR_FREE`
- [x] `AST_EXPR_DEREF`
- [x] `AST_EXPR_ADDR`
- [x] `AST_EXPR_MEMBER`
- [x] `AST_EXPR_INC`
- [x] `AST_EXPR_DEC`

### Statements
- [x] `AST_PROGRAM` (multi-module support)
- [x] `AST_PREPROCESSOR_MODULE`
- [x] `AST_PREPROCESSOR_USE`
- [x] `AST_STMT_EXPRESSION`
- [x] `AST_STMT_VAR_DECL`
- [x] `AST_STMT_FUNCTION`
- [x] `AST_STMT_RETURN`
- [x] `AST_STMT_BLOCK`
- [x] `AST_STMT_IF`
- [x] `AST_STMT_PRINT`
- [x] `AST_STMT_DEFER`
- [x] `AST_STMT_LOOP` (this is for, while, and while-true)

### Types
- [x] `AST_TYPE_BASIC`
- [x] `AST_TYPE_POINTER`
- [x] `AST_TYPE_ARRAY`
- [x] `AST_TYPE_FUNCTION`

---

## 📝 Next Steps

### Parsing
- [ ] Add parsing for templates (`fn[T]`, `struct[T]`)  
- [ ] Add parsing for type aliases using `type` keyword  
- [ ] Add parsing for modules and imports refinements  
- [ ] Design and implement **union syntax**  

### Semantic Analysis
- [ ] Type inference for generics  
- [ ] Detect unused imports and symbols  

### Codegen
- [ ] Implement codegen for `switch` or `match` constructs  
- [ ] Support more LLVM optimizations  
- [ ] **Add structs and enums support** in codegen  
- [ ] **Add unions support** in codegen
- [ ] **Add in memcpy and streq** streq === strcmp

### Lexer & Parser
- [ ] Add tokens and grammar for unions  

### Type Checker
- [ ] Implement type checking for structs  
- [ ] Implement type checking for unions 

---

## 🚀 Future Features
- [ ] Investigate pattern matching  
- [ ] Build minimal standard library  

