#include "args.h"

Arguments *args = NULL;

std::string zen_text = R"(
    * There should only be one obvious way to do things
    * Simplicity and speed should be prioritised more then anything else
    * Longer doesn't mean less readable and shorter doesn't mean smarter
    * A more readable solution will help you in the long run
    * Do what is best for the problem not what is trendy
    * Errors are not exceptional
    * Reduce developer pain as much as possible)";

void Arguments::New(int argc, char **argv) {
    args         = new Arguments{};
    auto options = new cxxopts::Options("liamc", "Liam programming language compiler");

    // required fields
    options->add_options()("i,in", "[REQUIRED] Input file path", cxxopts::value<std::string>()->default_value(""));
    options->add_options()(
        "s,stdlib", "[REQUIRED] Get the stdlib location", cxxopts::value<std::string>()->default_value("")
    );

    // optionals with defaults
    options->add_options()("o,out", "Output file path", cxxopts::value<std::string>()->default_value("out.cpp"));
    options->add_options()("e,emit", "Print emitted C++ to stdout", cxxopts::value<bool>()->default_value("false"));
    options->add_options()("t,time", "Print times", cxxopts::value<bool>()->default_value("false"));
    options->add_options()("h,help", "See this help screen", cxxopts::value<bool>()->default_value("false"));
    options->add_options()("z,zen", "See the zen of Liam", cxxopts::value<bool>()->default_value("false"));
    options->add_options()("T,test", "Build binary to run tests", cxxopts::value<bool>()->default_value("false"));

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

    // required
    args->stdlib  = args->value<std::string>("stdlib");
    args->in_path = args->value<std::string>("in");

    // optional
    args->out_path = args->value<std::string>("out");
    args->emit     = args->value<bool>("emit");
    args->time     = args->value<bool>("time");
    args->test     = args->value<bool>("test");

    if (args->in_path.empty())
    {
        panic("--in is a required flag use --help for more info");
    }

    if (args->stdlib.empty())
    {
        panic("--stdlib is a required flag use --help for more info");
    }
}
