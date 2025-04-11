#include "subcmd.hpp"

#include <iostream>

auto main(int argc, char *argv[]) -> int {
    if (argc < 2) {
        std::cerr << "No subcommand provided. Use --help for usage." << std::endl;
        return 1;
    }
    auto maybe_subcmd = SubCmd::get(argv[1]);
    if (!maybe_subcmd) {
        std::cerr << to_string(maybe_subcmd.error()) << " " << argv[1] << "\n";
        std::terminate();
    }

    auto subcmd = maybe_subcmd.value();
    auto result = subcmd->options().parse(argc, argv);
    auto context = subcmd->prepare(result);
    if (!context) {
        std::terminate();
    }

    if (!subcmd->execute(std::move(context))) {
        std::terminate();
    };

    return 0;
}