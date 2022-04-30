#include<stdio.h>
#include "string.h"
#define u64 int

void print_number(u64 n){
printf("%d", n);
}

struct Stats{
u64 age;
string name;

};

struct Person{
Stats stats;

};

u64 main(){
Person p = {{21, "jack"}};
u64 x = p.stats.age;
print_number(x);

}

