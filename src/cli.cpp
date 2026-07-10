#include "pylon.hpp"

#include <iostream>

namespace pylon {

CliOptions parse_cli(int argc, char** argv) {
    CliOptions options;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--compact") {
            options.verbosity = ReportVerbosity::Compact;
            continue;
        }
        if (arg == "--full") {
            options.verbosity = ReportVerbosity::Full;
            continue;
        }
        if (arg == "--standard") {
            options.verbosity = ReportVerbosity::Standard;
            continue;
        }
        if (arg == "--script") {
            if (i + 1 >= argc) {
                throw RuntimeError("--script requires a path");
            }
            options.command = "script";
            options.script_path = argv[++i];
            continue;
        }
        if (arg == "script") {
            if (i + 1 >= argc) {
                throw RuntimeError("script requires a path");
            }
            options.command = "script";
            options.script_path = argv[++i];
            continue;
        }
        options.command = arg;
    }
    return options;
}

int run_cli(int argc, char** argv) {
    try {
        const CliOptions options = parse_cli(argc, argv);
        ScenarioRunner scenarios;
        ScenarioReport report;
        if (options.command == "script") {
            report = scenarios.script(options.script_path);
        } else {
            report = scenarios.run(options.command);
        }
        ReportBuilder builder;
        std::cout << builder.build(report, options.verbosity);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "pylon-dtl: " << error.what() << "\n";
        return 1;
    }
}

}  // namespace pylon
