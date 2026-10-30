#include "config.hpp"

#include "../galaxy/galaxy.hpp"

#define Y_GALAXIES "galaxies"
#define Y_FIXED_TIME "fixed_time"

void Config::Load(const std::string& path) {
    conf = YAML::LoadFile(path);
}

data Config::New() const {
    // TODO
    // currently, the memory management here stands to be improved, we're copying a lot of data in galaxy iterations
    // when points grows large and there are multiple galaxies, this will be incredibly slow
    // a better option is to first scan the number of points needed, create a data object that can hold all of it
    // and then have the galaxies write to the location in the buffer

    data sim;
    YAML::Node galaxies = this->conf[Y_GALAXIES];


    for (const auto& gConf : galaxies) {
        Galaxy g(gConf);
        data d = g.New();

        sim.append(d);
    }

    return sim;
}

float Config::FixedTime() const {
    return this->conf[Y_FIXED_TIME].as<float>(0.1f);
}

#undef Y_GALAXIES
#undef Y_FIXED_TIME
