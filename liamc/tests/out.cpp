#include "lib.h"

// enum forward declarations
// struct forward declarations
// typedefs

// function forward declarations
void  __liam__main__();

// Source
void __liam__main__(){
if (true) {
println<str>(make_str((char*)"true", 4));

}

if (!(false)) {
println<str>(make_str((char*)"!false", 6));

}

if (true && true) {
println<str>(make_str((char*)"true and true", 13));

}

if (false || true) {
println<str>(make_str((char*)"false or true", 13));

}

if (_s64(4) == _s64(4)) {
println<str>(make_str((char*)"4 == 4", 6));

}

if (_s64(1) < _s64(10)) {
println<str>(make_str((char*)"1 < 10", 6));

}

if (_s64(10) > _s64(1)) {
println<str>(make_str((char*)"10 > 1", 6));

}

if (_s64(10) <= _s64(10)) {
println<str>(make_str((char*)"10 <= 10", 8));

}

if (_s64(8) <= _s64(9)) {
println<str>(make_str((char*)"8 <= 9", 6));

}

if (_s64(100) >= _s64(100)) {
println<str>(make_str((char*)"100 >= 100", 10));

}

if (_s64(90) >= _s64(80)) {
println<str>(make_str((char*)"90 >= 80", 8));

}


}

int main(int argc, char **argv) { __liam__main__(); }