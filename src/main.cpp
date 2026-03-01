#include "app/app.hpp"

int main(int argc, char* argv[]) {
    auto f = cliargs();
    auto a = app();
    
    f.parse(argc, argv);
    return a.run(f);
}
