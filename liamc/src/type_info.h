#pragma once

#include "expression.h"
#include "lexer.h"

struct TypeExpression;
struct StructTypeInfo;
struct FnTypeInfo;

enum class TypeInfoType {
    ANY,
    VOID,
    NUMBER,
    BOOLEAN,
    STRING,
    FN,
    FN_EXPRESSION,
    STRUCT,
    STRUCT_INSTANCE,
    WEAK_POINTER,
    OWNED_POINTER,
    SLICE,
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
    NumberType number_type;

    NumberTypeInfo(size_t size, NumberType number_type);
};

struct BoolTypeInfo : TypeInfo {
    BoolTypeInfo();
};

struct WeakPointerTypeInfo : TypeInfo {
    TypeInfo *to;

    WeakPointerTypeInfo(TypeInfo* to);
};

struct OwnedPointerTypeInfo : TypeInfo {
    TypeInfo *to;

    OwnedPointerTypeInfo(TypeInfo* to);
};

struct SliceTypeInfo : TypeInfo {
    TypeInfo *to;
    SliceTypeInfo(TypeInfo *to);
};

struct StrTypeInfo : TypeInfo {
    StrTypeInfo();
};

struct TypeTypeInfo : TypeInfo {
    TypeTypeInfo();
};

struct StructTypeInfo : TypeInfo {
    std::vector<std::tuple<std::string, FnTypeInfo *>> member_functions;
    std::vector<std::tuple<std::string, TypeInfo *>> members;
    u64 generic_count;

    StructTypeInfo(std::vector<std::tuple<std::string, FnTypeInfo *>> memberFunctions,
                   std::vector<std::tuple<std::string, TypeInfo *>> members, u64 genericCount);
};

struct StructInstanceTypeInfo : TypeInfo {
    StructTypeInfo *struct_type;
    std::vector<TypeInfo *> generic_types;

    StructInstanceTypeInfo(StructTypeInfo *structType, std::vector<TypeInfo *> genericTypes);
};

struct FnTypeInfo : TypeInfo {
    StructTypeInfo *parent_type;
    TypeInfo *return_type;
    std::vector<TypeInfo *> generic_type_infos;
    std::vector<TypeInfo *> args;

    FnTypeInfo(StructTypeInfo *parentType, TypeInfo *returnType, std::vector<TypeInfo *> genericTypeInfos,
               std::vector<TypeInfo *> args);
};

struct FnExpressionTypeInfo : TypeInfo {
    TypeInfo *return_type;
    std::vector<TypeInfo *> args;

    FnExpressionTypeInfo(TypeInfo *returnType, std::vector<TypeInfo *> args);
};

struct GenericTypeInfo : TypeInfo {
    u64 id;

    GenericTypeInfo(u64 id);
};

struct UnionTypeInfo : TypeInfo {
    std::vector<TypeInfo *> types;

    UnionTypeInfo(std::vector<TypeInfo *> types);
};

struct EnumTypeInfo : TypeInfo {
    std::string identifier;
    std::vector<std::string> instances;

    EnumTypeInfo(std::string identifier, std::vector<std::string> instances);
};