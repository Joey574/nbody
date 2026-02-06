#include "util.hpp"

namespace util {
    size_t aligned_size(size_t size) {
        return (size + 31) & ~31;
    }
} // namespace util
