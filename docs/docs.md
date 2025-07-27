# Lux Language Documentation

Lux is a statically typed, compiled programming language designed for systems programming. It combines the low-level control of C with a strong type system that eliminates many common runtime errors.

## Table of Contents
- [Why `const` is Used for Top-Level Definitions in Lux](#why-const-is-used-for-top-level-definitions-in-lux)
- [1. Declaration of Top-Level Bindings(`const`)](#1-declaration-of-top-level-bindingsconst)
- [2. Control Flow ('if', 'elif', 'else', 'loop')](#2-control-flow-if-elif-else-loop)
- [3. Type System](#3-type-system)
- [4. Memory Management](#4-memory-management)
- [5. Error Handling](#5-error-handling)
- [6. Performance](#6-performance)
- [7. Safety](#7-safety)

## Why `const` is Used for Top-Level Definitions in Lux

In Lux, all top-level bindings — including functions, enums, structs, and immutable variables — are declared using the `const` keyword:

```lux
const NAME: TYPE = expression;
```

This doesn't mean they are all variable declarations in the traditional sense — in fact, most are not. Instead, `const` serves as a generic binding construct that immutably associates a name with a value, type, or definition at the top level.

### 1. Unified Top-Level Binding Syntax

Lux uses `const` as the only keyword for binding names at the top level, no matter what is being defined — a function, an enum, a literal, or a complex type.

```lux
const NUM: int = 42;                  ;; Immutable variable
const Direction = enum { ... };       ;; Enum definition
const Point = struct { ... };         ;; Struct definition
const add = fn (a: int, b: int) int { return a + b; }; ;; Function
```

**Benefit:** Keeps the language minimal and predictable. Everything defined at the top level looks the same structurally.

### 2. Semantic Immutability

Using `const` communicates to both the programmer and the compiler that the name cannot be rebound.

Even though you are not always declaring a "variable," the binding itself is immutable:

```lux
const x: int = 5;
x = 10; ;; ❌ Error: `x` is immutable

const add = fn (a: int, b: int) int { return a + b; };
add = something_else; ;; ❌ Error
```

### 3. Cleaner Internal Model

From the compiler's point of view, using `const` for all top-level bindings means:

- A single parsing rule handles all top-level declarations
- The type of the value (struct, fn, enum, literal, etc.) is determined by the initializer expression
- The AST reflects that not everything is a "VarDecl" — only constructs that actually bind a typed value are

```lux
const Point = struct { x: int, y: int };
;; Not a "VarDecl" — it's a StructExpr bound to the name `Point`.

const origin: Point = Point { x: 0, y: 0 };
;; ✅ This *is* a true VarDecl
```

### 4. Support for Future Features

This model opens the door for:

- Compile-time metaprogramming and reflection
- Type-level declarations and generic parameters
- Uniform caching and import behavior for top-level bindings

# 1. Declaration of Top-Level Bindings(`const`)

In Lux, all top-level bindings are declared using the `const` keyword, regardless of their type:

```lux
const NAME: TYPE = expression;
```

This applies to functions, enums, structs, and immutable variables. The `const` keyword serves as a unified binding construct that immutably associates a name with a value, type, or definition at the top level.

```lux
const NUM: int = 42;                  ;; Immutable variable
const Direction = enum { ... };       ;; Enum definition
const Point = struct { ... };         ;; Struct definition
const add = fn (a: int, b: int) int { return a + b; }; ;; Function
```

**Benefit:** Keeps the language minimal and predictable. Everything defined at the top level looks the same structurally.

# 2. Control Flow ('if', 'elif', 'else', 'loop')

Control flow in Lux is managed through a series of keywords that allow for conditional execution and looping. The primary constructs are:

- `if` - Executes a block of code if a condition is true.
- `elif` - Checks another condition if the previous `if` was false.
- `else` - Executes a block of code if all previous conditions were false.
- `loop` - Repeatedly executes a block of code.

### Example

```lux
const x: int = 5;

if x > 10 {
    print("x is greater than 10");
} elif x > 5 {
    print("x is greater than 5");
} else {
    print("x is 5 or less");
}

;; For loops in Lux 
loop (i: int = 0; i < x) : (i++) {
    outputln(1, "Iteration: ", i);
}

;; While loops in Lux
let i: int = 0;
loop (i < x) {
    outputln(1, "Iteration: ", i);
    i++;
}

;; While loops with an optional condition
let j: int = 0;
loop (j < x) : (j++) {
    outputln(1, "Iteration: ", j);
}
```
