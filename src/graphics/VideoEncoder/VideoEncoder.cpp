#include "VideoEncoder.hpp"

#include <cstdlib>
#include <cstring>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#ifdef _WIN32
    #define REC_POPEN _popen
    #define REC_PCLOSE _pclose
    #define REC_MODE "wb"
    #define REC_PROBE "ffmpeg -version >NUL 2>&1"
#else
    #include <csignal>
    #define REC_POPEN popen
    #define REC_PCLOSE pclose
    #define REC_MODE "w"
    #define REC_PROBE "ffmpeg -version >/dev/null 2>&1"
#endif

void VideoEncoder::open(const std::string& path, uint32_t width, uint32_t height, uint32_t fps, const char* pixFmt) {
    close();

    if (path.empty() || path.find('"') != std::string::npos) throw std::invalid_argument("invalid recording path");
    if (std::system(REC_PROBE) != 0) throw std::runtime_error("recording needs ffmpeg on the path, but it could not be found");

    #ifndef _WIN32
        std::signal(SIGPIPE, SIG_IGN);
    #endif


    const std::string cmd =
        std::string("ffmpeg -y -hide_banner -loglevel error -nostats")
        + " -f rawvideo -pixel_format " + pixFmt
        + " -video_size " + std::to_string(width) + "x" + std::to_string(height)
        + " -framerate " + std::to_string(fps)
        + " -i pipe:0"
        + " -vf \"pad=ceil(iw/2)*2:ceil(ih/2)*2\""
        + " -c:v libx264 -preset fast -crf 18 -pix_fmt yuv420p"
        + " \"" + path + "\"";

    pipe_ = REC_POPEN(cmd.c_str(), REC_MODE);
    if (!pipe_) throw std::runtime_error("failed to start ffmpeg");

    frameBytes_ = static_cast<size_t>(width) * height * 4;
    done_ = false;
    failed_ = false;
    thread_ = std::thread(&VideoEncoder::writer, this);
}

void VideoEncoder::push(const void* pixels) {
    std::vector<uint8_t> buf;
    {
        std::unique_lock lk(m_);
        cvPush_.wait(lk, [&] { return queue_.size() < MAX_QUEUED || failed_; });
        if (failed_) throw std::runtime_error("ffmpeg stopped accepting frames");

        if (!pool_.empty()) {
            buf = std::move(pool_.back());
            pool_.pop_back();
        }
    }

    buf.resize(frameBytes_);
    std::memcpy(buf.data(), pixels, frameBytes_);

    {
        std::lock_guard lk(m_);
        queue_.push_back(std::move(buf));
    }

    cvPop_.notify_one();
}

void VideoEncoder::writer() {
    std::vector<uint8_t> buf;

    for (;;) {
        {
            std::unique_lock lk(m_);
            cvPop_.wait(lk, [&] { return !queue_.empty() || done_;  });
            if (queue_.empty()) return;

            buf = std::move(queue_.front());
            queue_.pop_front();
        }
        cvPush_.notify_one();

        if (std::fwrite(buf.data(), 1, buf.size(), pipe_) != buf.size()) {
            {
                std::lock_guard lk(m_);
                failed_ = true;
                queue_.clear();
            }

            cvPush_.notify_all();
            return;
        }

        std::lock_guard lk(m_);
        pool_.push_back(std::move(buf));
    }
}

void VideoEncoder::close() {
    if (!pipe_) return;

    {
        std::lock_guard lk(m_);
        done_ = true;
    }
    cvPop_.notify_all();

    if (thread_.joinable()) thread_.join();

    const int rc = REC_PCLOSE(pipe_);
    pipe_ = nullptr;

    if (rc != 0 || failed_) {
        std::fprintf(stderr, "ffmpeg reported an error, the recording may be incomplete\n");
    }

    queue_.clear();
    pool_.clear();
    done_ = false;
    failed_ = false;
}
