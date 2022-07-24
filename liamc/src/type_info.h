#pragma once

#include "lexer.h"

enum class TypeInfoType {
    ANY,
    VOID,
    NUMBER,
    CHAR,
    BOOLEAN,
    STRING,
    FN,
    STRUCT,
    STRUCT_INSTANCE,
    POINTER,
    GENERIC,
    UNION,
    ENUM,
};

struct TypeInfo {
    TypeInfoType type;
};

struct AnyTypeInfo : TypeInfo {};

struct VoidTypeInfo : TypeInfo {};

struct NumberTypeInfo : TypeInfo {
    size_t size;
    NumberType type;
};

struct BoolTypeInfo : TypeInfo {};

struct CharTypeInfo : TypeInfo {};

struct PointerTypeInfo : TypeInfo {
    TypeInfo *to;
};

struct StrTypeInfo : TypeInfo {};

struct TypeTypeInfo : TypeInfo {};

struct FnTypeInfo : TypeInfo {
    TypeInfo *return_type;
    std::vector<TypeInfo *> args;
    u64 generic_count;
};

struct StructTypeInfo : TypeInfo {
    std::vector<std::tuple<std::string, TypeInfo *>> members;
    u64 generic_count;
};

struct StructInstanceTypeInfo : TypeInfo {
    TypeInfo *struct_type;
    std::vector<TypeInfo *> generic_types;
};

struct GenericTypeInfo : TypeInfo {
    u64 id;
};

struct UnionTypeInfo : TypeInfo {
    std::vector<TypeInfo *> types;
};

struct EnumTypeInfo : TypeInfo {
    std::string identifier;
    std::vector<std::string> instances;
};
