#pragma once
#include "../definitions/graphics.hpp" // IWYU pragma: keep

struct camera {
    private:
    glm::vec2 position = { 0.0f, 0.0f };
    float zoom         = 1.0f;
    float speed        = 500.0f;
    float zoomSpeed    = 0.1f;

    public:
    void update(GLFWwindow* window, float dt) {
        float step = speed * dt / zoom;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) position.y += step;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) position.y -= step;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) position.x -= step;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) position.x += step;
    }

    void onScroll(float yOffset) {
        zoom *= (1.0f + yOffset * zoomSpeed);
        zoom = glm::clamp(zoom, 1e-9f, 100.0f);
    }

    glm::mat4 viewMatrix() const {
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::scale(view, glm::vec3(zoom, zoom, 1.0f));
        view = glm::translate(view, glm::vec3(-position, 0.0f));
        return view;
    }

    glm::mat4 projMatrix(float w, float h) const {
        auto p = glm::ortho(
            -w / 2.0f, w / 2.0f,
            -h / 2.0f, h / 2.0f,
            -1.0f, 1.0f
        );

        p[1][1] *= -1.0f;
        return p;
    }
};
