#include "config.hpp"
#include "../galaxy/galaxy.hpp"

#include <stdexcept>

#define Y_GALAXIES "galaxies"
#define Y_FIXED_TIME "fixed_time"
#define Y_THETA "theta"

void Config::Load(const std::string& path) {
    conf = YAML::LoadFile(path);
}

data Config::New(bool init_acc) const {
    // TODO
    // currently, the memory management here stands to be improved, we're copying a lot of data in galaxy iterations
    // when points grows large and there are multiple galaxies, this will be incredibly slow
    // a better option is to first scan the number of points needed, create a data object that can hold all of it
    // and then have the galaxies write to the location in the buffer

    data sim;
    YAML::Node galaxies = this->conf[Y_GALAXIES];
    if (!galaxies || !galaxies.IsSequence() || galaxies.size() == 0) {
        throw std::invalid_argument("config must contain at least one nonempty galaxy sequence");
    }

    for (const auto gConf : galaxies) {
        Galaxy g;
        g.Load(gConf, this->FixedTime());

        data d = g.New(init_acc);
        sim.append(d, init_acc);
    }

    return sim;
}

float Config::FixedTime() const {
    return this->conf[Y_FIXED_TIME].as<float>(0.1f);
}

float Config::Theta() const {
    return this->conf[Y_THETA].as<float>(1.0f);
}

#undef Y_GALAXIES
#undef Y_FIXED_TIME
#undef Y_THETA
