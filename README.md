# Liam
Liam is a strongly typed compiled language intended for performance comparable to C++ but less pain along the way. Liam generates and outputs `c++` which is then compiled to a final binary. 

## Examples
### Hello world
```rust
fn main(): u64 {
    print[^char]("Hello world");
}
```

### Structs
```rust
struct Person {
    name: ^char
}

fn main(): u64 {
    let s: Person = new Person{"liam"};
    print[^char](s.name);
}
```

## Syntax
### Variables
```rust
let var_name: type = ... ;
```
```rust
let var_name := ... ; // type inferance
```

### Functions
```rust
fn func_name(param: type): return_type {}
```

### Generic Functions
```rust
fn func_name[T](param: T): ^T {}
```

### For Loops
```rust
for let i: u64 = 0; i < 10; i = i + 1; {
    print[u64](i);   
}
```

### Structs
```rust
struct struct_name {
    member1: type,
    member2: type
}
```

### Generic Structs
```rust
struct struct_name[T] {
    member1: T,
    member2: type
}
```

### Conditionals
```rust
if condition or another_condition {
    ...
}
```

### Pointers
```rust
let x_ptr: ^type = @x;
let y: type = *x_ptr;
```

