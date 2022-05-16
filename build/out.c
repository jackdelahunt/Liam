#include <stdio.h>
typedef unsigned long u64;
#define true 1
#define false 0


typedef struct Person {
u64 age;
} Person;

void print_age(Person* p){

        printf("%d", p->age);
    
}

u64 main(){
Person p = (Person) {21};
print_age(&(p));
return 0;
}

