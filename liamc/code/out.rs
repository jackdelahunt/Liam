#![allow(unused_unsafe)]#![allow(non_camel_case_types)]#![allow(dead_code)]#![allow(unused_mut)]#![allow(unused_variables)]#![allow(unused_assignments)]#![allow(non_snake_case)]#![allow(unused_braces)]#![allow(unreachable_code)]#![allow(unused_imports)]#![allow(unused_parens)]type void = ();#[derive(Clone)]struct Person{age: u64,}impl Person {fn new(age: u64) -> Self {Self{age}}}fn print_age(p: *mut Person) -> void { unsafe {        printf("%d", p->age);    }}fn main() -> u64 { unsafe {let mut p: Person = Person::new(21);print_age((&mut p) as *mut _);return 0;}}