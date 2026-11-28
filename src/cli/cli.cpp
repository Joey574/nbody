#include "cli.hpp"
#include <stdexcept>
#include <string>
#include <unistd.h>

void cliargs::parse(int argc, char* argv[]) {
    if (argc == 1) printUsage();

    for (int i = 1; i < argc; i++) {
        const std::string v(argv[i]);

        if (v == "-h" || v == "--help") {
            printUsage();
        } else if (v == "--cpu") {
            cpu = true;
        } else if (v == "--bh") {
            bh = true;
        } else if (v == "-q" || v == "--quiet") {
            quiet = true;
        } else if (v == "--refresh") {
            if (argc > i+1) refresh = std::stoul(argv[++i]);
        } else if (v == "-f" || v == "--file") {
            if (argc > i+1) config.Load(argv[++i]);
        } else if (v == "--pause") {
            pause = true;
        } else if (v == "--zoom") {
            if (argc > i+1) zoom = std::stof(argv[++i]);
        } else if (v == "-x") {
            if (argc > i+1) x = std::stof(argv[++i]);
        } else if (v == "-y") {
            if (argc > i+1) y = std::stof(argv[++i]);
        } else if (v == "--record") {
            if (argc > i+1) record = argv[++i];
        } else if (v == "--fps") {
            if (argc > i+1) fps = std::stoul(argv[++i]);
        } else if (v == "--max-frames") {
            if (argc > i+1) frames = std::stoul(argv[++i]);
        } else if (v == "--width") {
            if (argc > i+1) width = std::stoul(argv[++i]);
        } else if (v == "--height") {
            if (argc > i+1) height = std::stoul(argv[++i]);
        } else if (v == "--headless") {
            headless = true;
        } else {
            throw std::invalid_argument(("unrecognized argument \"" + v + "\"").c_str());
        }
    }
}

void cliargs::printUsage() const {
    printf(
        "nbody unstable"
        "\nUsage: nbody [Options] -f path.yml"
        "\n\nOPTIONS:"
        "\n\t-f, --file: config file for simulation"
        "\n\t--cpu: use cpu computation"
        "\n\t--bh: use barnes-hut algorithm"
        "\n\t-q, --quiet: quiet perf output"
        "\n\t--refresh: set refresh rate of perf output"
        "\n\t--pause: start the simulation paused"
        "\n\t--record: encode frames to this video file (requires ffmpeg)"
        "\n\t--headless: disable rendering of window"
        "\n\t--fps: playback fps on the video"
        "\n\t--max-frames: stop after this many frames"
        "\n\t--width: set the starting width of the window, must be even"
        "\n\t--height: set the starting height on the window, must be even"
        "\n\t--zoom: set the starting zoom value"
        "\n\t-x: set the camera's starting x position"
        "\n\t-y: set the camera's starting y position"
        "\n\n"
    );
    exit(0);
}
