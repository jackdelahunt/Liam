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

PointerTypeInfo::PointerTypeInfo(TypeInfo *to) {
    this->to   = to;
    this->type = TypeInfoType::POINTER;
}

PointerSliceTypeInfo::PointerSliceTypeInfo(TypeInfo *to) {
    this->to   = to;
    this->type = TypeInfoType::POINTER_SLICE;
}

StrTypeInfo::StrTypeInfo() {
    this->type = TypeInfoType::STRING;
}

TypeTypeInfo::TypeTypeInfo() {
}

StructTypeInfo::StructTypeInfo(
    u16 module_id, u16 file_id, std::vector<std::tuple<std::string, FnTypeInfo *>> memberFunctions,
    std::vector<std::tuple<std::string, TypeInfo *>> members, u64 genericCount
) {
    this->module_id        = module_id;
    this->file_id          = file_id;
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
    u16 module_id, u16 file_id, StructTypeInfo *parentType, TypeInfo *returnType,
    std::vector<TypeInfo *> genericTypeInfos, std::vector<TypeInfo *> args
) {
    this->module_id          = module_id;
    this->file_id            = file_id;
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

EnumTypeInfo::EnumTypeInfo(u16 module_id, u16 file_id, std::vector<EnumMember> members) {
    this->module_id = module_id;
    this->file_id   = file_id;
    this->members   = members;
    this->type      = TypeInfoType::ENUM;
}
