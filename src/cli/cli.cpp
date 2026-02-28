#include "cli.hpp"
#include <string>
#include "../util/util.hpp"

void cliargs::parse(int argc, char* argv[]) {
    path = util::executable_path(argv);

    // parse out command line args
    for (int i = 1; i < argc; i++) {
        const std::string v(argv[i]);

        if (v == "-h" || v == "--help") {
            printf(
                "nbody v0"
                "\nUsage: nbody [Options]"
                "\n\nOPTIONS:"
                "\n\t--cluster: use cluster initialization for points (default)"
                "\n\t--spiral: use spiral initialization for points"
                "\n\t--cpu: use cpu side computation"
                "\n\t-b/--bodies: set number of points in simulation (default: 2)"
                "\n\t-t/--fixed-time: set the time step value used for the simulation (default: 0.0001)"
                "\n\n"
            );
            exit(0);
        } else if (v == "--cluster") {
            cluster = true;
        } else if (v == "--spiral") {
            spiral = true;
        } else if (v == "--cpu") {
            cpu = true;
        } else if (v == "-b" || v == "--bodies") {
            if (argc > i+1) {
                bodies = std::stoul(argv[i+1]);
                i++;
            }
        } else if (v == "-t" || v == "--fixed-time") {
            if (argc > i+1) {
                fixedtime = std::stof(argv[i+1]);
                i++;
            }
        } else if (v == "-q" || v == "--quiet") {
            quiet = true;
        } else if (v == "--refresh") {
            if (argc > i+1) {
                refresh = std::stoul(argv[i+1]);
                i++;                    
            }
        }else {
            std::__throw_runtime_error(("unrecognized argument \"" + v + "\"").c_str());
        }
    }
}