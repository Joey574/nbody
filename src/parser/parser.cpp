#include "parser.hpp"

namespace parser {
    cliargs parse(int argc, char* argv[]) {
        auto f = cliargs();

        // parse out command line args
        for (int i = 1; i < argc; i++) {
            if (std::string(argv[i]) == "--cluster") {
                f.cluster = true;
            } else if (std::string(argv[i]) == "--spiral") {
                f.spiral = true;
            } else if (std::string(argv[i]) == "--cpu") {
                f.cpu = true;
            } else if (std::string(argv[i]) == "-b" || std::string(argv[i]) == "--bodies") {
                if (argc > i+1) {
                    f.bodies = std::stoul(argv[i+1]);
                    i++;
                }
            } else if (std::string(argv[i]) == "-t" || std::string(argv[i]) == "--fixed-time") {
                if (argc > i+1) {
                    f.fixedtime = std::stof(argv[i+1]);
                    i++;
                }
            } else {
                std::__throw_runtime_error(("unrecognized argument " + std::string(argv[i])).c_str());
            }
        }

        return f;
    }
}