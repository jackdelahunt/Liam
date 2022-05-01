#include<stdio.h>
#include "liam_string.h"
#define u64 int

void str_print(string* s){
printf("%s\n", s->str);
}

u64 str_length(string* s){
return s->size;
}

void u64_print(u64 n){
printf("%d", n);
}

u64 main(){
string s = "this is a string";
str_print(&s);
u64_print(str_length(&s));

}

