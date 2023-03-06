#include "type_info.h"

VoidTypeInfo::VoidTypeInfo() {
    this->type = TypeInfoType::VOID; 
}

NumberTypeInfo::NumberTypeInfo(size_t size, NumberType number_type) {
    this->size        = size;
    this->number_type = number_type;
    this->type        = TypeInfoType::NUMBER;
}

BoolTypeInfo::BoolTypeInfo() {
    this->type = TypeInfoType::BOOLEAN;
}

WeakPointerTypeInfo::WeakPointerTypeInfo(TypeInfo *to) {
    this->to   = to;
    this->type = TypeInfoType::WEAK_POINTER;
}

StrTypeInfo::StrTypeInfo() {
    this->type = TypeInfoType::STRING;
}

TypeTypeInfo::TypeTypeInfo() {
}

StructTypeInfo::StructTypeInfo(
    std::vector<std::tuple<std::string, FnTypeInfo *>> memberFunctions,
    std::vector<std::tuple<std::string, TypeInfo *>> members, u64 genericCount
) {
    this->member_functions = memberFunctions;
    this->members          = members;
    this->generic_count    = genericCount;
    this->type             = TypeInfoType::STRUCT;
}

StructInstanceTypeInfo::StructInstanceTypeInfo(StructTypeInfo *structType, std::vector<TypeInfo *> genericTypes) {
    this->struct_type   = structType;
    this->generic_types = genericTypes;
    this->type          = TypeInfoType::STRUCT_INSTANCE;
}

FnTypeInfo::FnTypeInfo(
    StructTypeInfo *parentType, TypeInfo *returnType, std::vector<TypeInfo *> genericTypeInfos,
    std::vector<TypeInfo *> args
) {
    this->parent_type        = parentType;
    this->return_type        = returnType;
    this->generic_type_infos = genericTypeInfos;
    this->args               = args;
    this->type               = TypeInfoType::FN;
}

FnExpressionTypeInfo::FnExpressionTypeInfo(TypeInfo *returnType, std::vector<TypeInfo *> args) {
    this->return_type = returnType;
    this->args        = args;
    this->type        = TypeInfoType::FN_EXPRESSION;
}

GenericTypeInfo::GenericTypeInfo(u64 id) {
    this->id   = id;
    this->type = TypeInfoType::GENERIC;
}

UnionTypeInfo::UnionTypeInfo(std::vector<TypeInfo *> types) {
    this->types = types;
    this->type  = TypeInfoType::UNION;
}

EnumTypeInfo::EnumTypeInfo(std::vector<EnumMember> members) {
    this->members = members;
    this->type    = TypeInfoType::ENUM;
}
