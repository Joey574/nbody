#include "config.hpp"
#include <random>

#define POINTS "points"
#define FIXEDTIME "fixedtime"
#define TYPE "type"
#define SEED "seed"

// spiral config data
#define RX "rx"
#define RXSCALE "rxscale"
#define RY "ry"
#define ROTSTART "rotstart"
#define ROTDELTA "rotdelta"
#define INCSTART "incstart"
#define INCDELTA "incdelta"
#define POSMEAN "posmean"
#define POSSTD "posstd"
#define MASSMEAN "massmean"
#define MASSSTD "massstd"
#define ELLIPSES "ellipses"
#define SEGMENTS "segments"

void Config::Load(const std::string& path) {
    conf = YAML::LoadFile(path);
}

size_t Config::Seed() const noexcept {
    std::default_random_engine urng(std::random_device{}());
    std::uniform_int_distribution<size_t> gen(0, std::numeric_limits<size_t>::max());
    return conf[SEED].as<size_t>(gen(urng));
}
size_t Config::Points() const noexcept {
    return conf[POINTS].as<size_t>(0);
}
float Config::Fixedtime() const noexcept {
    return conf[FIXEDTIME].as<float>(0.00001f);
}
std::string Config::Type() const noexcept {
    return conf[TYPE].as<std::string>("cluster");
}

SpiralConfig Config::Spiral() const noexcept {
    return {
        .rx        = conf[RX      ].as<float>(0.5f),
        .rx_scale  = conf[RXSCALE ].as<float>(1.2f),
        .ry        = conf[RY      ].as<float>(0.5f),
        .rot_start = conf[ROTSTART].as<float>(0.0f),
        .rot_delta = conf[ROTDELTA].as<float>(0.35f),
        .inc_start = conf[INCSTART].as<float>(0.075f),
        .inc_delta = conf[INCDELTA].as<float>(0.001f),
        .pos_mean  = conf[POSMEAN ].as<float>(0.001f),
        .pos_std   = conf[POSSTD  ].as<float>(0.075f),
        .mass_mean = conf[MASSMEAN].as<float>(0.025f),
        .mass_std  = conf[MASSSTD ].as<float>(0.005f),
        .ellipses  = conf[ELLIPSES].as<size_t>(25),
        .segments  = conf[SEGMENTS].as<size_t>(100)
    };
}

ClusterConfig Config::Cluster() const noexcept {
    return {

    };
}