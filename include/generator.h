#pragma once

#include "liam.h"
#include <map>

namespace liam {
    struct OpCommand {
        char* name;
        OpCode op;
        size_t arguments;
    };
    
    static OpCommand operations[] = {
        OpCommand{"add", OpCode::ADD, 0},
        OpCommand{"print", OpCode::PRINT, 0},
        OpCommand{"push", OpCode::PUSH, 1},
    };

    struct Generator
    {
        std::map<char*, Byte> labels;

        void generate() {
            
        }
    };
    
}
