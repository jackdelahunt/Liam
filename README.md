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
- [x] basic structs (no generics, or member functions)
- [x] basic functions (no generics)
- [x] size specific builting data types (u8, u16, u32, u64)
- [x] stack allocated non-dynamic arrays of arbitrarty types
- [x] multiple files using `import`
- [x] non-polluting import statements with namespace identifiers
- [x] out-of-order top level definitions
- [x] slice types
- [x] slicing syntax for arrays and slices
- [x] for and while loops

### Maybe [how easy/how useful for the current state/will not need to be redone/future work not harder]
- [ ] type safe continue, return and break [4/4/3/6] 19
- [ ] `-` unary expression [9/2/8/8] 27
- [ ] able to use `.` in number literals
- [ ] generic arguments for structs [2/5/4/2] 13
- [ ] generics arguments for functions [3/7/6/4] 20
- [ ] tagged union style enums (like rust enums) [2/3/6/6] 17
- [ ] enum pattern mathcing with a `match` keyword [3/2/5/6] 16
- [ ] iterators (non concreate idea no how these will work yet) [2/3/3/5] 13
    - [ ] for static arrays
    - [ ] a range given
    - [ ] user defined types
- [ ] number types coercing [3/10/4/4] 21
- [ ] `const` and `var` types [2/1/3/3] 9
- [ ] some error handling solution (zig, rust and go are all nice) [2/2/5/7] 16
- [ ] better stdlib [6/5/1/7] 19
    - [ ] dynamic array
    - [ ] `String` type
    - [ ] I/O
    - [ ] memory management
- [ ] bounds checking for container types only in debug mode with debug mode option [8/1/8/9] 26
- [ ] member functions for structs [4/2/5/5] 18
- [ ] global variables (const or maybe not) [5/1/8/7] 21
- [ ] `defer` keyword [1/1/4/7] 13
- [ ] some kind of optional types (either with tagged unions or builtin like zig) [2/3/7/6] 18
- [ ] calling into C/C++ from liam [5/4/6/7] 22
- [ ] inline custom struct types [1/2/7/2] 12
    

## Documentation
#### [The Wiki](https://github.com/jackdelahunt/Liam/wiki)
Some helpful documentation for getting started learning Liam.
