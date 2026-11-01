#pragma once
#include <string>
#include <yaml-cpp/yaml.h>

#include "../data/data.hpp"

struct Config {
    public:
        void Load(const std::string& path);
        data New(bool init_acc) const;

        float FixedTime() const;
        float Theta() const;

    private:
        YAML::Node conf;
};
