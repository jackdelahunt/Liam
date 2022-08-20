#include "args.h"

Arguments *args = NULL;

void Arguments::New(int argc, char **argv) {
    args         = new Arguments{};
    auto options = new cxxopts::Options("liamc", "Liam programming language compiler");

    // required fields
    options->add_options()("i,in", "Input file path", cxxopts::value<std::string>());

    // optionals with defaults
    options->add_options()("o,out", "Output file path", cxxopts::value<std::string>()->default_value("out.cpp"));
    options->add_options()("c,codegen", "Print codegen to stdout", cxxopts::value<bool>()->default_value("false"));
    options->add_options()("t,time", "Print times", cxxopts::value<bool>()->default_value("false"));
    options->add_options()("s,stdlib", "Get the stdlib location", cxxopts::value<std::string>());
    options->add_options()("h,help", "See this help screen", cxxopts::value<bool>()->default_value("false"));
    options->add_options()("I,include", "Include path", cxxopts::value<std::string>()->default_value("runtime/"));
    options->add_options()("E,emit", "Emit the cpp file", cxxopts::value<std::string>()->default_value(""));
    options->add_options()("D,debug", "Build cpp in debug", cxxopts::value<bool>()->default_value("false"));
    options->add_options()("T,test", "Build binary to run tests", cxxopts::value<bool>()->default_value("false"));
    args->result  = options->parse(argc, argv);
    args->options = options;

    // optional
    args->out_path = args->value<std::string>("out");
    args->codegen  = args->value<bool>("codegen");
    args->time     = args->value<bool>("time");
    args->stdlib   = args->value<std::string>("stdlib");
    args->help     = args->value<bool>("help");
    args->include  = args->value<std::string>("include");
    args->emit     = args->value<std::string>("emit");
    args->debug    = args->value<bool>("debug");
    args->test     = args->value<bool>("test");

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
