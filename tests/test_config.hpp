#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

struct temporary_config {
    public:
        explicit temporary_config(const std::string& contents) {
            const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
            path_ = std::filesystem::temp_directory_path() /
                ("nbody_test_config_" + std::to_string(stamp) + ".yml");

            std::ofstream file(path_);
            if (!file) {
                throw std::runtime_error("failed to create temporary config file");
            }

            file << contents;
            if (!file) {
                throw std::runtime_error("failed to write temporary config file");
            }
        }

        temporary_config(const temporary_config&) = delete;
        temporary_config& operator=(const temporary_config&) = delete;

        ~temporary_config() {
            std::error_code ec;
            std::filesystem::remove(path_, ec);
        }

        const std::filesystem::path& path() const noexcept {
            return path_;
        }

    private:
        std::filesystem::path path_;
};

inline constexpr char TWO_BODY_CONFIG[] = R"yaml(
fixed_time: 0.025
theta: 0.0
galaxies:
  - type: uniform
    points: 2
    seed: 1234
    inner_radius: 0.5
    outer_radius: 2.0
    x_pos: 1.0
    y_pos: -2.0
    x_vel: 0.5
    y_vel: -0.25
    center_mass: 10.0
)yaml";
