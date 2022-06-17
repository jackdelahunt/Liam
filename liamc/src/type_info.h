#pragma once

enum class TypeInfoType {
    ANY,
    VOID,
    INT,
    CHAR,
    BOOL,
    FN,
    STRUCT,
    POINTER,
    GENERIC,
};

struct TypeInfo {
    TypeInfoType type;
};

struct AnyTypeInfo : TypeInfo {};

struct VoidTypeInfo : TypeInfo {};

struct IntTypeInfo : TypeInfo {
    bool is_signed;
    size_t size;
};

struct BoolTypeInfo : TypeInfo {};

struct StringTypeInfo : TypeInfo {};

struct CharTypeInfo : TypeInfo {};


struct PointerTypeInfo : TypeInfo {
    TypeInfo* to;
};

struct TypeTypeInfo : TypeInfo {
};

struct FnTypeInfo : TypeInfo {
    TypeInfo* return_type;
    std::vector<TypeInfo*> args;
    u64 generic_count;
};

struct StructTypeInfo : TypeInfo {
    std::vector<std::tuple<std::string, TypeInfo*>> members;
};

struct GenericTypeInfo : TypeInfo {};
