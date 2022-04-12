#pragma once

#include "liam.h"
#include <map>
#include <fstream>
#include <string>
#include <optional>
#include <tuple>

namespace liam {
    struct OpCommand {
        std::string name;
        OpCode op;
        size_t arguments;
    };
    
    static OpCommand operations[] = {
        OpCommand{"add", OpCode::ADD, 0},
        OpCommand{"print", OpCode::PRINT, 0},
        OpCommand{"push", OpCode::PUSH, 1},
    };

    std::vector<std::string> extract_words(const char* path) {
        auto vec = std::vector<std::string>();

        std::ifstream file;  
        file.open(path);
    
        std::string word;
        while (file >> word)
        {
            vec.push_back(word);
        }

        file.close();
        return vec;
    }

    struct Generator
    {
        std::map<std::string, Byte> labels;

        std::tuple<VM, const char*> generate(const char* path) {
            auto words = extract_words(path);
            VM vm{0};

            int index = 0;
            while(index < words.size()) {
                auto word = words.at(index);

                if(word.c_str()[0] == '#') {
                    labels[word] = vm.memory_ptr;
                    goto next;
                }

                {
                    auto command = get_command(word);
                    if(command.has_value()) {
                        auto value = command.value();
                        vm.push_byte(value.op);

                        if(value.arguments > 0) {
                            for(int j = 1; j <= value.arguments; j++) {
                                auto arg = words.at(index + j);
                                Byte next_byte;

                                if(arg.c_str()[0] == '#') {
                                    next_byte = labels[arg];
                                } else {
                                    next_byte = stoi(arg);
                                }

                                vm.push_byte(next_byte);
                            }

                            index += value.arguments;
                        }
                    } else {
                        return {VM{0}, "Cannot find command"};
                    }
                }

                next:
                    index++;
            }

            return {vm, nullptr};
        }

        std::optional<OpCommand> get_command(const std::string& str) {
            for(auto& command : operations) {
                if(command.name == str)
                return command;
            }

            return {};
        }
    };
    
}
