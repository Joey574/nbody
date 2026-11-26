#pragma once

#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

struct VideoEncoder {
    public:
        VideoEncoder() = default;
        VideoEncoder(const VideoEncoder&) = delete;
        VideoEncoder& operator=(const VideoEncoder&) = delete;
        ~VideoEncoder() { close(); }

        void open(const std::string& path, uint32_t width, uint32_t height, uint32_t fps, const char* pixFmt);
        void push(const void* pixels);
        void close();

        inline bool isOpen() const noexcept { return pipe_ != nullptr; }
    private:
        static constexpr size_t MAX_QUEUED = 8;

        FILE* pipe_ = nullptr;
        size_t frameBytes_ = 0;

        std::thread thread_;
        std::mutex m_;
        std::condition_variable cvPop_;
        std::condition_variable cvPush_;

        std::deque<std::vector<uint8_t>> queue_;
        std::vector<std::vector<uint8_t>> pool_;

        bool done_ = false;
        bool failed_ = false;

        void writer();
};
