#include "Window.hpp"

#include <functional>
#include <stdexcept>

std::map<int, Window::KeyCallback> Window::registry;

void Window::Init(WindowCreateInfo createInfo) {
    if (createInfo.width % 2 || createInfo.height % 2) throw std::invalid_argument("width & height must both be even");
    if (!glfwInit()) throw std::runtime_error("failed to init glfw");

    initialized = true;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, createInfo.resizable);

    window = glfwCreateWindow(createInfo.width, createInfo.height, "nbody", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        initialized = false;
        throw std::runtime_error("failed to create window");
    }

    glfwSetWindowUserPointer(window, createInfo.userPtr);
    glfwSetFramebufferSizeCallback(window, createInfo.frameBufferSizeFn);

    // TODO
    //glfwSetScrollCallback(window, [](GLFWwindow* w, double x, double y){
    //   auto r = static_cast<renderer*>(glfwGetWindowUserPointer(w));
    //   r->cam.OnScroll(y);
    //});

    glfwSetKeyCallback(window, Window::Handler);

    if (createInfo.headless) {
        glfwHideWindow(window);
    } else {
        glfwShowWindow(window);
    }
}

void Window::Register(int key, KeyCallback callback) {
    if (registry.contains(key)) {
        throw std::invalid_argument("attempted to set multiple callbacks for keycode " + std::to_string(key));
    }

    registry[key] = callback;
}

void Window::Handler(GLFWwindow* w, int key, int scancode, int action, int mods) {
    if (registry.contains(key)) {
        auto fn = registry.at(key);
        std::invoke(fn, w, scancode, action, mods);
    }
}
