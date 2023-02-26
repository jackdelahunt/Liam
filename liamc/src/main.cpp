#include <chrono>
#include <filesystem>
#include <iostream>

#include "args.h"
#include "cpp_backend.h"
#include "compiler.h"
#include "liam.h"
#include <functional>

#ifndef TEST

/*
 * enum Expr {
 *      Number(i64),
 *      String,
 *      Compound(i64, i64, i64)
 * }


namespace ExprMembers {
    struct Number {
        i64 __1;
    };

    struct String {
    };

    struct Compound {
        i64 __1;
        i64 __2;
        i64 __3;
    };
}

struct Expr {
    u64 index;

    union Members {
        ExprMembers::Number __Number;
        ExprMembers::String __String;
        ExprMembers::Compound __Compound;

    } members;
};
 */

/*
 * let expr := Expr::Number(100);
 * Expr expr = Expr{0, ExprMembers::Number{100}};
 */

/*
 * match e {
 *      Number(n) => {...},
 *      String => {...},
 *      Compound(_, y, _) => {...},
 *      else => {...}
 * }
 *
 * if(expr.index == 0) { // Number
    auto n = expr.members.__Number.__1;
    } else if(expr.index == 1) {// String

    } else if(expr.index == 2) {
    auto y = expr.members.__Compound.__2;
    } else {

    }
 */

i32 main(i32 argc, char **argv) {

    TIME_START(total_time);

    Arguments::New(argc, argv);

    TIME_START(l_p_time);
    auto files = lex_parse(std::filesystem::absolute(args->in_path));
    TIME_END(l_p_time, "Lex and parsing time");

    TIME_START(type_time);
    type_check(&files);
    TIME_END(type_time, "Type checking time");

    TIME_START(borrow_checking_time);
    borrow_check(&files);
    TIME_END(borrow_checking_time, "Borrow checking time");

    TIME_START(code_gen);
    auto code = CppBackend().emit(&files);
    TIME_END(code_gen, "Code generation time");

    if (args->emit)
    { std::cout << code << "\n"; }

    std::ofstream out_file;
    out_file = std::ofstream(args->out_path);

    out_file << code;
    out_file.close();

    TIME_END(total_time, "Total compile time");

    return 0;
}
#endif
