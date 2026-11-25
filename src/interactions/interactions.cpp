#include "interactions.hpp"

void interactions::update(GLFWwindow* window, float dt) {
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_PAUSE) == GLFW_PRESS) {
            //paused = !paused;
            paused = true;
            printf("event\n");
        }
}

void interactions::callback(GLFWwindow* w, int key, int scancode, int action, int mods) {
    if ((key == GLFW_KEY_SPACE || key == GLFW_KEY_PAUSE) && action == GLFW_PRESS) {
        paused = !paused;
    }
}
