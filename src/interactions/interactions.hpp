#pragma ocne
#include "definitions/graphics.hpp" // IWYU pragma: keep

struct interactions {
    public:
        interactions() = default;
        interactions(bool paused) : paused(paused) {}

        void update(GLFWwindow* window, float dt);
        void callback(GLFWwindow* w, int key, int scancode, int action, int mods);

        bool Paused() const { return paused; }
    private:

        bool paused = false;
};
