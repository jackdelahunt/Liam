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
    ARRAY,
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

struct ArrayTypeInfo : TypeInfo {
    TypeInfo* array_type;
};

struct FnTypeInfo : TypeInfo {
    TypeInfo* return_type;
    std::vector<TypeInfo*> args;
};

struct StructTypeInfo : TypeInfo {
    std::vector<std::tuple<std::string, TypeInfo*>> members;
};