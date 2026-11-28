// #pragma once
// #include "definitions/graphics.hpp" // IWYU pragma: keep
//
// #include <map>
//
// struct WindowCreateInfo {
//     size_t width = 800;
//     size_t height = 800;
//     bool headless = false;
//     bool resizable = true;
//     void* userPtr = nullptr;
//
//     GLFWframebuffersizefun frameBufferSizeFn = nullptr;
// };
//
// struct Window {
//     using KeyCallback = std::function<void(GLFWwindow*, int, int, int)>;
//
//     public:
//
//         void Init(WindowCreateInfo createInfo);
//         static void Register(int key, KeyCallback callback);
//
//         GLFWwindow* GetWindow();
//
//     private:
//         GLFWwindow* window = nullptr;
//         bool initialized = false;
//
//         static std::map<int, KeyCallback> registry;
//         static void Handler(GLFWwindow* w, int key, int scancode, int action, int mods);
// };
