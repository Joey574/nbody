#include "camera.hpp"

void camera::update(GLFWwindow* window, float dt) {
    const float zoomBlend = 1.0f - std::exp(-12.0f * dt);
    zoom = std::exp(std::log(zoom) + (std::log(targetZoom) - std::log(zoom)) * zoomBlend);

    float step = speed * dt / zoom;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) position.y += step;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) position.y -= step;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) position.x -= step;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) position.x += step;
}

void camera::onScroll(float yOffset) {
    targetZoom *= (1.0f + yOffset * zoomSpeed);
    targetZoom = glm::clamp(targetZoom, 1e-9f, 100.0f);
}

glm::mat4 camera::viewMatrix() const {
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::scale(view, glm::vec3(zoom, zoom, 1.0f));
    view = glm::translate(view, glm::vec3(-position, 0.0f));
    return view;
}

glm::mat4 camera::projMatrix(float w, float h) const {
    auto p = glm::ortho(
        -w / 2.0f, w / 2.0f,
        -h / 2.0f, h / 2.0f,
        -1.0f, 1.0f
    );

    p[1][1] *= -1.0f;
    return p;
}
