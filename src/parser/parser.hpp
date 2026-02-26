#pragma once
#include <cstddef>
#include <string>

import cli_module;

namespace parser {
    cliargs parse(int argc, char* argv[]);        
} // namespace parser
