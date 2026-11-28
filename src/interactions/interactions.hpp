#pragma ocne
#include "definitions/graphics.hpp" // IWYU pragma: keep

#include <functional>
#include <map>

struct interactions {
    using KeyCallback = std::function<void(GLFWwindow*, int, int, int)>;

    public:
        interactions() = default;

        void Register(int key, KeyCallback callback);
        void Handler(GLFWwindow* w, int key, int scancode, int action, int mods);

    private:
        std::map<int, KeyCallback> registry;
};
