#include "module.h"

Module::Module(std::string name, std::filesystem::path path, std::vector<File *> files) {
    this->name  = name;
    this->path  = path;
    this->files = files;

    this->builtin_type_table   = std::unordered_map<std::string, TypeInfo *>();
    this->top_level_type_table = std::unordered_map<std::string, TypeInfo *>();
    this->top_level_type_table = std::unordered_map<std::string, TypeInfo *>();

    this->builtin_type_table["void"] = new VoidTypeInfo();
    this->builtin_type_table["bool"] = new BoolTypeInfo();
    this->builtin_type_table["str"]  = new StrTypeInfo();
    this->builtin_type_table["u8"]   = new NumberTypeInfo(8, NumberType::UNSIGNED);
    this->builtin_type_table["i8"]   = new NumberTypeInfo(8, NumberType::SIGNED);
    this->builtin_type_table["u16"]  = new NumberTypeInfo(16, NumberType::UNSIGNED);
    this->builtin_type_table["i16"]  = new NumberTypeInfo(16, NumberType::SIGNED);
    this->builtin_type_table["u32"]  = new NumberTypeInfo(32, NumberType::UNSIGNED);
    this->builtin_type_table["i32"]  = new NumberTypeInfo(32, NumberType::SIGNED);
    this->builtin_type_table["f32"]  = new NumberTypeInfo(32, NumberType::FLOAT);
    this->builtin_type_table["u64"]  = new NumberTypeInfo(64, NumberType::UNSIGNED);
    this->builtin_type_table["i64"]  = new NumberTypeInfo(64, NumberType::SIGNED);
    this->builtin_type_table["f64"]  = new NumberTypeInfo(64, NumberType::FLOAT);
}

void Module::add_type(Token token, TypeInfo *type_info) {
    if (this->top_level_type_table.count(token.string) > 0)
    {
        panic(
            "Duplcate creation of type: " + token.string + " at (" + std::to_string(token.span.line) + "," +
            std::to_string(token.span.start) + ")"
        );
    }

    this->top_level_type_table[token.string] = type_info;
}

void Module::add_function(Token token, TypeInfo *type_info) {
    if (this->top_level_function_table.count(token.string) > 0)
    {
        panic(
            "Duplicate creation of function: " + token.string + " at (" + std::to_string(token.span.line) + "," +
            std::to_string(token.span.start) + ")"
        );
    }

    this->top_level_function_table[token.string] = type_info;
}

std::tuple<TypeInfo *, bool> Module::get_type(Token *identifier) {
    return get_type(identifier->string);
}

std::tuple<TypeInfo *, bool> Module::get_type(std::string identifier) {
    if (this->top_level_type_table.count(identifier) > 0)
    { return {this->top_level_type_table[identifier], false}; }

    return {nullptr, true};
}