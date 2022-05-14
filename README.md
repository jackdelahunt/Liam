# Liam
Liam is a strongly typed compiled language intended for performance comparable to C++ but less pain along the way. Liam generates and outputs `rust` which is then compiled to a final binary. 

## Examples
### Hello world
```rust
fn main(): void {
    print_string(@make_string(@"Hello world"));
}
```

### Structs
```rust
struct Person {
    name: string
}

fn main(): void {
    let s: Person = new Person{"liam"};
    print_string(@s.name);
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

### For Loops
```rust
for str index in ["car", "dog", "house"] {
    print_u64(index);
    print_string(*str);   
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

### Array
```rust
let arr : [type] = [...];
```

### Pointers
```rust
let x_ptr: ^type = @x;
let y: type = *x_ptr;
```

