#pragma once

#include "expression.h"
#include "lexer.h"

struct TypeExpression;
struct StructTypeInfo;
struct FnTypeInfo;
struct EnumMember;

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
    POINTER,
    POINTER_SLICE,
    SLICE,
    GENERIC,
    ENUM,
};

struct TypeInfo {
    TypeInfoType type;
};

struct AnyTypeInfo : TypeInfo {};

struct VoidTypeInfo : TypeInfo {
    VoidTypeInfo();
};

struct NumberTypeInfo : TypeInfo {
    size_t size;
    NumberType number_type;

    NumberTypeInfo(size_t size, NumberType number_type);
};

struct BoolTypeInfo : TypeInfo {
    BoolTypeInfo();
};

struct PointerTypeInfo : TypeInfo {
    TypeInfo *to;

    PointerTypeInfo(TypeInfo *to);
};

struct PointerSliceTypeInfo : TypeInfo {
    TypeInfo *to;

    PointerSliceTypeInfo(TypeInfo *to);
};

struct StrTypeInfo : TypeInfo {
    StrTypeInfo();
};

struct TypeTypeInfo : TypeInfo {
    TypeTypeInfo();
};

struct StructTypeInfo : TypeInfo {
    u16 module_id;
    u16 file_id;

    std::vector<std::tuple<std::string, FnTypeInfo *>> member_functions;
    std::vector<std::tuple<std::string, TypeInfo *>> members;
    u64 generic_count;

    StructTypeInfo(
        u16 module_id, u16 file_id, std::vector<std::tuple<std::string, FnTypeInfo *>> memberFunctions,
        std::vector<std::tuple<std::string, TypeInfo *>> members, u64 genericCount
    );
};

struct StructInstanceTypeInfo : TypeInfo {
    StructTypeInfo *struct_type;
    std::vector<TypeInfo *> generic_types;

    StructInstanceTypeInfo(StructTypeInfo *structType, std::vector<TypeInfo *> genericTypes);
};

struct FnTypeInfo : TypeInfo {
    u16 module_id;
    u16 file_id;

    StructTypeInfo *parent_type;
    TypeInfo *return_type;
    std::vector<TypeInfo *> generic_type_infos;
    std::vector<TypeInfo *> args;

    FnTypeInfo(
        u16 module_id, u16 file_id, StructTypeInfo *parentType, TypeInfo *returnType,
        std::vector<TypeInfo *> genericTypeInfos, std::vector<TypeInfo *> args
    );
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

struct EnumTypeInfo : TypeInfo {
    u16 module_id;
    u16 file_id;

    std::vector<EnumMember> members;

    EnumTypeInfo(u16 module_id, u16 file_id, std::vector<EnumMember> members);
};
