#pragma once
#include <variant>
#include <cstddef>

struct spiral_config {
    float rx = 0.5f;
    float ry = 0.5f;
    float rot = 0.0f;
    float inc = 0.075f;
    float pos_meam = 0.001f;
    float pos_std = 0.075f;
    float mass_mean = 0.025f;
    float mass_std = 0.005f;
    size_t ellipses = 25;
    size_t segments = 100;
};

struct cluster_config {

};

struct config {
    private:
    std::variant<spiral_config, cluster_config> conf;

    public:
    std::variant<spiral_config, cluster_config> Config() const {
        return conf;
    }    
};
