<p align="center">
    <img src="docs/LiamLogo.png" alt="logo" style="width:70%; border-radius:2%">
    <br/>
   A Data-Oriented Programming Language for modern developers.
    <br/>
    <br/>
    <a href="https://github.com/jackdelahunt/Liam">
        <img src="https://img.shields.io/github/v/release/jackdelahunt/Liam?display_name=tag&include_prereleases">
    </a>
    <a href="https://github.com/jackdelahunt/Liam">
        <img src="https://img.shields.io/github/last-commit/jackdelahunt/Liam/main">
    </a>
    <a href="https://github.com/jackdelahunt/Liam">
        <img src="https://img.shields.io/github/repo-size/jackdelahunt/Liam">
    </a>
    <a href="https://github.com/jackdelahunt/Liam">
        <img src="https://img.shields.io/badge/platforms-Windows%20|%20Linux%20|%20macOS-green.svg">
    </a>
    <br>
</p>


# The Liam Programming Language
Liam is a strongly typed compiled language intended for performance comparable to C++ but less pain along the way. Liam generates and outputs `c++` which is then compiled to a final binary.

Stack allocated linked list example:

```rust
struct NodeAllocator {
    buffer: [100]Node,
    used: i64
}

fn make_node(allocator: ^NodeAllocator, value: i64) ^Node {
    let node : ^Node = &allocator.buffer[allocator.used];
    allocator.used = allocator.used + 1;

    *node = new Node{
        value: value,
        next: null
    };

    return node;
}

struct Node {
    value: i64,
    next: ^Node
}

struct LinkedList {
    head: ^Node,
    size: i64
}

fn make_linked_list() LinkedList {
    return new LinkedList {
        head: null,
        size: 0
    };
}

fn push(list: ^LinkedList, allocator: ^NodeAllocator, value: i64) void {
    let new_node := make_node(allocator, value);
    if list.head == null {
        list.head = new_node;
        return;
    }

    let current_node := list.head;

    // there are no while loops yet so having a dummy 
    // varaible here is needed
    for let dummy := 0; current_node.next != null; current_node = current_node.next; {}

    current_node.next = new_node;
}

fn main() void {
    let allocator : NodeAllocator = zero;
    let list := make_linked_list();
    push(&list, &allocator, 10);
    push(&list, &allocator, 20);
    push(&list, &allocator, 30);

    for let node := list.head; node != null; node = node.next; {

    }
}
```

## Features and possbile future ones
### Added
- [x] control flow (if, c-stlye for, continue, break, return)
- [x] basic structs (no generics, or member functions)
- [x] basic functions (no generics)
- [x] size specific builting data types (u8, u16, u32, u64)
- [x] stack allocated non-dynamic arrays of arbitrarty types
- [x] multiple files using `import`
- [x] non-polluting import statements with namespace identifiers
- [x] out-of-order top level definitions
- [x] builtin `str` type

### Maybe 
- [ ] control flow (while, better for loops for types)
- [ ] generic arguments for structs
- [ ] generics arguments for functions
- [ ] tagged union style enums (like rust enums)
- [ ] enum pattern mathcing with a `match` keyword
- [ ] iterators (non concreate idea no how these will work yet)
    - [ ] for static arrays
    - [ ] a range given
    - [ ] user defined types
- [ ] number types coercing
- [ ] `const` and `var` types
- [ ] some error handling solution (zig, rust and go are all nice)
- [ ] better stdlib
    - [ ] dynamic array
    - [ ] `String` type
    - [ ] I/O
    - [ ] memory management
- [ ] bounds checking for container types only in debug mode
- [ ] member functions for structs
- [ ] global variables (const or maybe not)
- [ ] `defer` keyword
- [ ] some kind of optional types (either with tagged unions or builtin like zig)
- [ ] calling into C/C++ from liam
- [ ] inline custom struct types
    

## Documentation
#### [Learning the basics](docs/Learning-the-basics.md)
Some helpful documentation for getting started learning Liam.
