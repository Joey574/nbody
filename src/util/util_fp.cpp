#include "util.hpp"

#if (defined(_WIN32) || defined(_WIN64))
    #define NOMINMAX
    #include <windows.h>

    std::string util::executable_path(char* argv[]) {
        wchar_t buffer[MAX_PATH];
        DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        if (len > 0) {
            auto ws = std::wstring(buffer, len)
            return std::string(ws.begin(), ws.end());
        }
        return "";
    }
    #elif (defined(LINUX) || defined(__linux__))
    #include <unistd.h>
    #include <limits.h>

    std::string util::executable_path(char* argv[]) {
        char buffer[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (len != -1) {
            buffer[len] = '\0';
            return std::string(buffer);
        }
        return "";
    }
    #else
    std::string util::executable_path(char* argv[]) {
        return std::string(argv[0]);
    }
    #endif