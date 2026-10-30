#pragma once
#include <cstddef>
#include <string>
#include <yaml-cpp/yaml.h>

#include "../data/data.hpp"

struct Config {
    private:
        YAML::Node conf;

    public:
        void Load(const std::string& path);
        data New() const;

        float FixedTime() const;
};
