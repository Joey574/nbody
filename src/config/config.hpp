#pragma once
#include <yaml-cpp/yaml.h>

struct SpiralConfig {
    float rx;
    float rx_scale;
    float ry;
    float rot_start;
    float rot_delta;
    float inc_start;
    float inc_delta;
    float pos_mean;
    float pos_std;
    float mass_mean;
    float mass_std;
    size_t ellipses;
    size_t segments;
};

struct ClusterConfig {

};


struct Config {
    private:
    YAML::Node conf;

    public:
    void Load(const std::string& path);

    size_t Seed() const noexcept;
    size_t Points() const noexcept;
    float Fixedtime() const noexcept;
    std::string Type() const noexcept;

    SpiralConfig Spiral() const noexcept;
    ClusterConfig Cluster() const noexcept;
};