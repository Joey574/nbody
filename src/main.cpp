#include "app/app.hpp"

int main(int argc, char* argv[]) {
    auto f = cliargs();
    auto a = app();

    f.Parse(argc, argv);
    return a.Run(f);
}
