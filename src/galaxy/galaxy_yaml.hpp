#pragma once

#include <stdexcept>
#include <string>
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/exceptions.h>

namespace galaxy_yaml {
    inline void Check(bool ok, const char* ctx, const std::string& msg) {
        if (!ok) throw std::invalid_argument(std::string(ctx) + ": " + msg);
    }

    template<typename T>
    T Require(const YAML::Node& n, const char* key, const char* ctx) {
        const YAML::Node v = n[key];
        if (!v) throw std::invalid_argument(std::string(ctx) + ": missing required key '" + key + "'");

        try {
            return v.as<T>();
        } catch (const YAML::Exception& e) {
            throw std::invalid_argument(std::string(ctx) + ": bad value for '" + key + "' (" + e.msg + ")");
        }
    }

    template<typename T>
    T Optional(const YAML::Node& n, const char* key, T fallback, const char* ctx) {
        const YAML::Node v = n[key];
        if (!v) return fallback;

        try {
            return v.as<T>();
        } catch (const YAML::Exception& e) {
            throw std::invalid_argument(std::string(ctx) + ": bad value for '" + key + "' (" + e.msg + ")");
        }
    }
};
