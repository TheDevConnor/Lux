# Luma Language Documentation

Luma is a statically typed, compiled programming language designed for systems programming. It combines the low-level control of C with a strong type system and modern safety features that eliminate many common runtime errors.

## Table of Contents

- [Language Philosophy](#language-philosophy)
- [Top-Level Bindings with `const`](#top-level-bindings-with-const)
- [Control Flow](#control-flow)
- [Type System](#type-system)
- [Memory Management](#memory-management)
- [Error Handling](#error-handling)
- [Performance](#performance)
- [Safety Features](#safety-features)

## Language Philosophy

Luma is built on three core principles:

- **Simplicity**: Minimal syntax with consistent patterns
- **Safety**: Strong typing and memory safety features
- **Performance**: Zero-cost abstractions and predictable performance

## Top-Level Bindings with `const`

Luma uses the `const` keyword as a **unified declaration mechanism** for all top-level bindings. Whether you're declaring variables, functions, types, or enums, `const` provides a consistent syntax that enforces immutability at the binding level.

### Basic Syntax

```Luma
const NUM: int = 42;                    // Immutable variable
const Direction = enum { North, South, East, West };  // Enum definition
const Point = struct { x: int, y: int }; // Struct definition
const add = fn (a: int, b: int) int {   // Function definition
    return a + b; 
};
```

### Why This Design?

**Unified syntax**: One parsing rule handles all top-level declarations, simplifying both the compiler and developer experience.

**Semantic clarity**: The binding itself is immutable—you cannot reassign or shadow a top-level `const`. This prevents accidental redefinition bugs.

**Compiler optimization**: Immutable bindings enable better optimization opportunities.

**Future extensibility**: This approach naturally supports compile-time metaprogramming and uniform import behavior.

### Important Notes

```Luma
const x: int = 5;
x = 10; // ❌ Error: `x` is immutable

const add = fn (a: int, b: int) int { return a + b; };
add = something_else; // ❌ Error: cannot reassign function binding
```

## Control Flow

Luma provides clean, flexible control flow constructs that handle most programming patterns without unnecessary complexity.

### Conditional Statements

Use `if`, `elif`, and `else` for branching logic:

```Luma
const x: int = 7;

if x > 10 {
    print("Large number");
} elif x > 5 {
    print("Medium number");  // This will execute
} else {
    print("Small number");
}
```

### Loop Constructs

The `loop` keyword provides several iteration patterns:

#### For-Style Loops

```Luma
// Basic for loop
loop [i: int = 0](i < 10) {
    outputln("Iteration: ", i);
    ++i;
}

// For loop with post-increment
loop [i: int = 0](i < 10) : (++i) {
    outputln("i = ", i);
}

// Multiple loop variables
loop [i: int = 0, j: int = 0](i < 10) : (++i) {
    outputln("i = ", i, ", j = ", j);
    ++j;
}
```

#### While-Style Loops

```Luma
// Condition-only loop
let counter: int = 0;
loop (counter < 5) {
    outputln("Count: ", counter);
    counter++;
}

// While loop with post-action
let j: int = 0;
loop (j < 10) : (j++) {
    outputln("Processing: ", j);
}
```

#### Infinite Loops

```Luma
loop {
    // Runs forever until `break` is encountered
    if should_exit() {
        break;
    }
    do_work();
}
```

> **Note**: In Luma, `//` starts single-line comments, and `/* */` are used for multi-line comments.

## Type System

Luma provides a straightforward type system with both primitive and compound types.

### Primitive Types

| Type | Description | Size |
|------|-------------|------|
| `int` | Signed integer | 64-bit |
| `uint` | Unsigned integer | 64-bit |
| `float` | Floating point | 64-bit |
| `bool` | Boolean | 1 byte |
| `str` | String | Variable |

### Enumerations

Enums provide type-safe constants with clean syntax:

```Luma
const Direction = enum {
    North,
    South,
    East,
    West
};

const current_direction: Direction = Direction.North;
```

### Structures

Structures group related data with optional access control:

```Luma
const Point = struct {
    x: int,
    y: int
};

// With explicit access modifiers
const Player = struct {
public:
    name: str,
    score: int,
private:
    internal_id: uint,
    
    // Methods can be defined inside structs
    get_display_name = fn () str {
        return name + " (" + str(score) + " pts)";
    }
};
```

### Using Types

```Luma
const origin: Point = Point { x: 0, y: 0 };
const player: Player = Player { 
    name: "Alice", 
    score: 100,
    internal_id: 12345 
};
```

## Memory Management

Luma provides explicit memory management with safety-oriented features. While manual, it includes tools to prevent common memory errors.

### Basic Memory Operations

```Luma
alloc(size: uint) -> *void    // Allocate memory
free(ptr: *void)              // Deallocate memory
cast<T>(ptr: *void) -> *T     // Type casting
sizeof(type) -> uint          // Size of type in bytes
memcpy(dest: *void, src: *void, size: uint)  // Memory copy
```

### Example Usage

```Luma
const main = fn () int {
    // Allocate memory for an integer
    let ptr: *int = cast<*int>(alloc(sizeof(int)));
    
    // Use the memory
    *ptr = 42;
    outputln("Value: ", *ptr);
    
    // Clean up
    free(ptr);
    return 0;
}
```

### The `defer` Statement

To prevent memory leaks and ensure cleanup, Luma provides `defer` statements that execute when leaving the current scope:

```Luma
const process_data = fn () {
    let buffer: *int = cast<*int>(alloc(sizeof(int) * 100));
    defer free(buffer);  // Guaranteed to run when function exits
    
    let file: *File = open_file("data.txt");
    defer close_file(file);  // Will run even if early return
    
    // Complex processing...
    if error_condition {
        return; // defer statements still execute
    }
    
    // More processing...
    // defer statements execute here automatically
}
```

You can also defer multiple statements:

```Luma
defer {
    close_file(file);
    cleanup_resources();
    log("Operation completed");
}
```

**Key Benefits of `defer`:**
- Ensures cleanup code runs regardless of how the function exits
- Keeps allocation and deallocation code close together
- Prevents resource leaks from early returns or error conditions
- Executes in reverse order (LIFO - Last In, First Out)

### Size Queries

```Luma
const check_sizes = fn () {
    outputln("int size: ", sizeof(int));        // 8 bytes
    outputln("Point size: ", sizeof(Point));    // 16 bytes
    outputln("Direction size: ", sizeof(Direction)); // 4 bytes
}
```

## Error Handling

*[This section would cover Luma's approach to error handling - perhaps result types, error propagation, or exception mechanisms]*

## Performance

*[This section would detail Luma's performance characteristics, optimization strategies, and benchmarking approach]*

## Safety Features

*[This section would cover additional safety features like bounds checking, null pointer prevention, and compile-time guarantees]*

---

## Quick Start Example

```Luma
const Point = struct {
    x: int,
    y: int,
    
    distance_to = fn (other: Point) float {
        let dx: int = other.x - x;
        let dy: int = other.y - y;
        return sqrt(cast<float>(dx * dx + dy * dy));
    }
};

const main = fn () int {
    let origin: Point = Point { x: 0, y: 0 };
    let destination: Point = Point { x: 3, y: 4 };
    
    outputln("Distance: ", origin.distance_to(destination));
    return 0;
}
```

## Error Handling

*[This section would contain information about Luma's error handling - to be documented]*

## Performance

*[This section would contain information about Luma's performance characteristics - to be documented]*

## Safety

*[This section would contain information about Luma's safety features - to be documented]*