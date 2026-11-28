#include "interactions.hpp"

void interactions::Register(int key, KeyCallback callback) {
    if (registry.contains(key)) {
        throw std::invalid_argument("attempted to set multiple callbacks for keycode " + std::to_string(key));
    }

    registry[key] = callback;
}

void interactions::Handler(GLFWwindow* w, int key, int scancode, int action, int mods) {
    if (registry.contains(key)) {
        auto fn = registry.at(key);
        std::invoke(fn, w, scancode, action, mods);
    }
}
