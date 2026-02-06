#pragma once
#include <cstddef>
#include <string>

#include "../definitions/cliargs.hpp"

namespace parser {
    cliargs parse(int argc, char* argv[]);        
} // namespace parser
