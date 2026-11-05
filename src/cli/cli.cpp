#include "cli.hpp"
#include <stdexcept>
#include <string>

void cliargs::parse(int argc, char* argv[]) {
    if (argc == 1) printUsage();

    for (int i = 1; i < argc; i++) {
        const std::string v(argv[i]);

        if (v == "-h" || v == "--help") {
            printUsage();
        } else if (v == "--cpu") {
            this->cpu = true;
        } else if (v == "--bh") {
            this->bh = true;
        } else if (v == "-q" || v == "--quiet") {
            this->quiet = true;
        } else if (v == "--refresh") {
            if (argc > i+1) {
                this->refresh = std::stoul(argv[++i]);
            }
        } else if (v == "-f" || v == "--file") {
            if (argc > i+1) {
                this->config.Load(argv[++i]);
            }
        } else {
            throw std::invalid_argument(("unrecognized argument \"" + v + "\"").c_str());
        }
    }
}

void cliargs::printUsage() const {
    printf(
        "nbody v0"
        "\nUsage: nbody [Options] -f path.yml"
        "\n\nOPTIONS:"
        "\n\t--cpu: use cpu computation"
        "\n\t--bh: use barnes-hut algorithm"
        "\n\t-q, --quiet: quiet perf output"
        "\n\t--refresh: set refresh rate of perf output"
        "\n\t-f, --file: config file for simulation"
        "\n\n"
    );
    exit(0);
}
