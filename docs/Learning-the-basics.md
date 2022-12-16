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
fn func_name(param: type) return_type {}
```

### Generic Functions
```rust
fn func_name[T](param: T) ^T {}
```

### Function expression
```rust
let func: fn (s64) void = fn (n: s64) void { ... }
```

### For Loops
```rust
for let i: s64 = 0; i < 10; i = i + 1; {
    println[s64](i);   
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