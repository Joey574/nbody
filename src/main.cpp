#include "app/app.hpp"

int main(int argc, char* argv[]) {
    auto f = cliargs();
    f.parse(argc, argv);
    auto a = app();
    return a.run(f);
}
