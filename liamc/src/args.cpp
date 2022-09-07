#include "args.h"

Arguments *args = NULL;

void Arguments::New(int argc, char **argv) {
    args         = new Arguments{};
    auto options = new cxxopts::Options("liamc", "Liam programming language compiler");

    // required fields
    options->add_options()("i,in", "Input file path", cxxopts::value<std::string>());

    // optionals with defaults
    options->add_options()("out", "Output binary path", cxxopts::value<std::string>()->default_value("a.out"));
    options->add_options()("emit", "Emit codegen to stdout", cxxopts::value<bool>()->default_value("false"));
    options->add_options()("time", "Print times", cxxopts::value<bool>()->default_value("false"));
    options->add_options()("stdlib", "Get the stdlib location", cxxopts::value<std::string>());
    options->add_options()("help", "See this help screen", cxxopts::value<bool>()->default_value("false"));
    options->add_options()("include", "Include path", cxxopts::value<std::string>()->default_value("runtime/"));
    options->add_options(
    )("cpp-source", "Where to save the cpp file", cxxopts::value<std::string>()->default_value(""));
    options->add_options()("debug", "Build cpp in debug", cxxopts::value<bool>()->default_value("false"));
    options->add_options()("test", "Build binary to run tests", cxxopts::value<bool>()->default_value("false"));
    options->add_options()("llvm", "Use LLVM backend", cxxopts::value<bool>()->default_value("false"));
    options->add_options()("cpp", "Use C++ backend", cxxopts::value<bool>()->default_value("false"));
    args->result  = options->parse(argc, argv);
    args->options = options;

    // optional
    args->out_path   = args->value<std::string>("out");
    args->emit       = args->value<bool>("emit");
    args->time       = args->value<bool>("time");
    args->stdlib     = args->value<std::string>("stdlib");
    args->help       = args->value<bool>("help");
    args->include    = args->value<std::string>("include");
    args->cpp_source = args->value<std::string>("cpp-source");
    args->debug      = args->value<bool>("debug");
    args->test       = args->value<bool>("test");
    args->llvm       = args->value<bool>("llvm");
    args->cpp        = args->value<bool>("cpp");

    // required args
    if (args->result.hasValue<std::string>("in"))
    { args->in_path = args->value<std::string>("in"); }
    else
    {
        // if runtime or help is set then dont bother check for --in
        if (!args->help)
            panic("Not all required flags given");
    }
}
