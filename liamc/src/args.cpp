#include "args.h"
#include "cxxopts/cxxopts.h"
#include <vector>

Arguments *args = NULL;

std::string zen_text = R"(
    * There should only be one obvious way to do things
    * Simplicity and speed should be prioritised more then anything else
    * Longer doesn't mean less readable and shorter doesn't mean smarter
    * A more readable solution will help you in the long run
    * Do what is best for the problem not what is trendy
    * Errors are not exceptional
    * Reduce developer pain as much as possible)";

void Arguments::make(int argc, char **argv) {
    args         = new Arguments{};
    auto options = new cxxopts::Options("liamc", "Liam programming language compiler");

    // optionals with defaults
    options->add_options(
    )("o,out", "Output compilation_unit path", cxxopts::value<std::string>()->default_value("out.cpp"));
    options->add_options()("e,emit", "Print emitted C++ to stdout", cxxopts::value<bool>()->default_value("false"));
    options->add_options()("t,time", "Print times", cxxopts::value<bool>()->default_value("false"));
    options->add_options()("h,help", "See this help screen", cxxopts::value<bool>()->default_value("false"));
    options->add_options()("z,zen", "See the zen of Liam", cxxopts::value<bool>()->default_value("false"));
    options->add_options()("T,test", "Build binary to run tests", cxxopts::value<bool>()->default_value("false"));
    options->add_options(
    )("f,files", "Input files to compile", cxxopts::value<std::vector<std::string>>()->default_value({}));

    options->parse_positional({"files"});

    args->result  = options->parse(argc, argv);
    args->options = options;

    if (args->result.count("help"))
    {
        std::cout << options->help() << std::endl;
        exit(0);
    }

    if (args->result.count("zen"))
    {
        std::cout << zen_text << std::endl;
        exit(0);
    }

    // optional
    args->out_path = args->value<std::string>("out");
    args->emit     = args->value<bool>("emit");
    args->time     = args->value<bool>("time");
    args->test     = args->value<bool>("test");
    args->files    = args->value<std::vector<std::string>>("files");
}
