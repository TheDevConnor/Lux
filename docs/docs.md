# Lux Language Documentation

Lux is a statically typed, compiled programming language designed for systems programming. It combines the low-level control of C with a strong type system that eliminates many common runtime errors.

## Table of Contents

- [Top-Level Bindings with `const`](#top-level-bindings-with-const)
- [Control Flow ('if', 'elif', 'else', 'loop')](#control-flow-if-elif-else-loop)
- [Type System](#type-system)
- [Memory Management](#memory-management)
- [Error Handling](#error-handling)
- [Performance](#performance)
- [Safety](#safety)

## Top-Level Bindings with `const`

Lux uses the `const` keyword as a **unified way** to declare all top-level bindings — whether they are immutable variables, functions, enums, or structs. This design keeps the language syntax minimal, predictable, and semantically clear.

```lux
const NUM: int = 42;                  ;; Immutable variable
const Direction = enum { ... };       ;; Enum definition
const Point = struct { ... };         ;; Struct definition
const add = fn (a: int, b: int) int { ;; Function definition
    return a + b; 
};
```
This means all names bound at the top level are immutable and cannot be rebound later, which helps prevent many common bugs and aids compiler optimizations.

### Why `const`?

- **Unified syntax:** One parsing rule handles all top-level declarations, simplifying the compiler and making code easier to read
- **Semantic immutability:** The binding itself is immutable — you cannot reassign or shadow a top-level const
- **Clear internal model:** The type of the binding (struct, function, enum, literal) is determined by its initializer expression
- **Future-proof:** Supports compile-time metaprogramming, type-level declarations, and uniform caching/import behavior

### Examples

```lux
const x: int = 5;
x = 10; ;; ❌ Error: `x` is immutable

const add = fn (a: int, b: int) int { return a + b; };
add = something_else; ;; ❌ Error: cannot reassign function binding

const Point = struct { x: int, y: int };
;; This binds a Struct type to the name `Point`.

const origin: Point = Point { x: 0, y: 0 };
;; This is a true immutable variable binding.
```

## Control Flow ('if', 'elif', 'else', 'loop')

Lux provides flexible control flow constructs for conditional execution and looping:

- **`if`** — Executes a block if a condition is true
- **`elif`** — Checks an additional condition if previous if or elif failed
- **`else`** — Executes if all previous conditions were false
- **`loop`** — Repeats a block of code, supporting for-style and while-style loops

### Conditional Example

```lux
const x: int = 5;

if x > 10 {
    print("x is greater than 10");
} elif x > 5 {
    print("x is greater than 5");
} else {
    print("x is 5 or less");
}
```

### Loop Variants

**For-style loop:**
```lux
loop (i: int = 0; i < x) : (i++) {
    outputln(1, "Iteration: ", i);
}
```

**While-style loop (condition only):**
```lux
let i: int = 0;
loop (i < x) {
    outputln(1, "Iteration: ", i);
    i++;
}
```

**While-style loop with post-action:**
```lux
let j: int = 0;
loop (j < x) : (j++) {
    outputln(1, "Iteration: ", j);
}
```

> **Note:** In Lux, `;;` is the comment delimiter inside code blocks.

## Type System

*[This section would contain information about Lux's type system - to be documented]*

## Memory Management

*[This section would contain information about Lux's memory management - to be documented]*

## Error Handling

*[This section would contain information about Lux's error handling - to be documented]*

## Performance

*[This section would contain information about Lux's performance characteristics - to be documented]*

## Safety

*[This section would contain information about Lux's safety features - to be documented]*