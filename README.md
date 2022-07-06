<p align="center">
    <img src="extra/LiamLogo.png" alt="logo" style="width:70%; border-radius:2%">
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

```rust
import "stdlib/io"

struct Person[T] {
    name: ^char,
    data: T
}

fn main(): u64 {
    let p: Person[u64] = new Person[u64]{
        "liam",
        12
    };

    print[^char](p.name);
    print[u64](p.data);

    for let i := 0; i < p.data; i = i + 1; {
        print[u64](i);
    } 
}
```

## Building on Linux

### Requirements
- G++
- Make
- Bash

``` bash
cd liamc
```

``` bash
mkdir Code
```
``` bash
touch main.liam
```

``` bash
bash scripts/build_run.sh
```

## Building on Windows
### Requirements
- Visual Studio 2022

```powershell
cd liamc
```
```powershell
.\vendor\premake\premake5.exe vs2022
```
Creates Visual Studio project to use as normal

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

