## Fixing C:
   - context free grammar
   - no preprocessor
   - strong typing
   - no declaration order
   - defined fixed sized primitives
   - replace unsigned with natural that can overflow
   - no integral promotion
   - checked integer arithmetic
   - bit array primitive, don't conflate bit operations with integers
   - texts literals not null terminated
   - remove varargs
   - named function parameters
   - defined expression evaluation order
   - remove comma operator
   - product type with undefined layout
   - object sizes can be 0
   - remove pointer arithmetic
   - arrays have value semantics, remove array implicit conversion to pointer
   - modules, including modulemap
   - (probably many more, maybe starting with C not a great idea)

## Linked List Ideas
```lux
;; Syntax will change on somethins
const Link = struct {
    tag: int,
    value = union {
        .nil = struct {},
        .node = struct {
            value: int,
            next: *Link
        }
    }
};

const link_list_length_rec = fn (list: *Link) int {
    if (list.tag == nil) {
        return 0;
    } else {
        return 1 + link_list_length_rec(list.value.node.next);
    }
};

const link_list_length_tail = fn (list: *Link, acc: int) int {
    loop [acc: int = 0, lt: *Link = list, i:int = 0](true) {
        if (lt.tag == nil) return acc;
        else {
            continue[lt.value.node.next, acc + 1, _]; ;;Underscore does not change the value
        }
    }
};

const link_list_length = fn (list: *Link) int {
    return link_list_length_tail(list, 0);
};
```

## Loop Syntax
```lux
const check = fn (i: *int, j: *int) bool {
    let res: bool = (&i < 10) ? false : true;
    i&++;
    return res;
};

pub const main = fn () int {
    loop [i: int = 0, j: int = 0, k: int = 0] with check {
        outputln(1, i);
    }

    loop [i: int = 0, j: int = 0, k: int = 0](check(&j, &k)) {
        outputln(1, i);
    }

    ;; for loop with optional condition
    loop [i: int = 0](i < 10) : (i++) {
        outputln(1, i);
    }

    ;; for loop without optional condition
    loop [i: int = 0](i < 10) {
        outputln(1, i);
        continue [i + 1];
    }

    ;; while loop
    let x: int = 0;
    loop (x < 10) {
        outputln(1, x);
        x++;
    }

    ;; while loop + optional condition
    let x: int = 0;
    loop (true) : (x++) {
        outputln(1, x);
    }
    
    return 0;
};
```