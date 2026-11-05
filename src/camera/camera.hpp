#pragma once
#include "../definitions/graphics.hpp" // IWYU pragma: keep

struct camera {
    public:
        camera() = default;

        void update(GLFWwindow* window, float dt);
        void onScroll(float yOffset);

        glm::mat4 viewMatrix() const;
        glm::mat4 projMatrix(float w, float h) const;

    private:
        glm::vec2 position = { 0.0f, 0.0f };
        float zoom = 1.0f;
        float targetZoom = 1.0f;
        float speed = 500.0f;
        float zoomSpeed = 0.1f;
};
