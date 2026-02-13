#include "app/app.hpp"
#include "parser/parser.hpp"

#include <unordered_set>

int main(int argc, char* argv[]) {
    auto f = parser::parse(argc, argv);
    auto a = app();
    return a.run(f);
}
