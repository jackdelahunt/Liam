## Syntax

### Comments
```rust
// Just a normal 'C' style comment
```

### Variables
```rust
let var_name: i64 = 100 ;
```
```rust
let var_name := 100 ; // type inferance
```

### Functions
```rust
fn func_name(param: type) return_type {
    ...
}
```

### For Loops
```rust
for let i := 0; i < 10; i = i + 1; {
    println::<i64>(i);   
}
```

### Structs
```rust
struct struct_name {
    member1: type,
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
let x_ptr: ^type = &x;
let y: type = *x_ptr;
```
