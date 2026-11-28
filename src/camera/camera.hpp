#pragma once
#include "definitions/graphics.hpp" // IWYU pragma: keep

struct camera {
    public:
        camera() = default;
        camera(float x, float y, float zoom) : position({x, y}), zoom(zoom), targetZoom(zoom) {}

        void Update(GLFWwindow* window, float dt);
        void OnScroll(float yOffset);

        glm::mat4 ViewMatrix() const;
        glm::mat4 ProjMatrix(float w, float h) const;

    private:
        glm::vec2 position = { 0.0f, 0.0f };
        float zoom = 1.0f;
        float targetZoom = 1.0f;
        float speed = 500.0f;
        float zoomSpeed = 0.1f;
};
