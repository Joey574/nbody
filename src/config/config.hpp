#pragma once
#include "../data/data.hpp"

#include <yaml-cpp/yaml.h>
#include <string>

struct Config {
    public:
        void Load(const std::string& path);
        data New(bool init_acc) const;

        float FixedTime() const;
        float Theta() const;

    private:
        YAML::Node conf;
};
