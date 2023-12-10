#pragma once

#include "ast.h"
#include <vector>

struct SortingNode {
    StructTypeInfo *type_info;
    std::vector<SortingNode *> depends_on;
    bool permenent_mark;
    bool temperory_mark;

    SortingNode(StructTypeInfo *type_info) {
        this->type_info      = type_info;
        this->depends_on     = std::vector<SortingNode *>();
        this->permenent_mark = false;
        this->temperory_mark = false;
    }
};
