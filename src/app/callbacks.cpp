#include "app.hpp"

void app::PauseCallback(GLFWwindow* w, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        app::paused = !app::paused;
    }
}
