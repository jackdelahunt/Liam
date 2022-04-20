#include<stdio.h>
#include "string.h"
#define u64 int

void print_number(u64 n){
printf("%d", n);
}

void print_number_ptr(u64* n){
print_number(*n);

}

u64 main(){
u64 x = 100;
print_number_ptr(&x);

}

