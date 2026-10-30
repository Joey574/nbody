#pragma once
#include <concepts>
#include <cstdint>
#include <cstddef>
#include <cstdlib>

/*
 * The main loop should go something like this
 *
 * Sort points in some way such that points near each other in 2d space are near each other in memory
 * Compute CoM of all objects in the grid
 * Compare CoM against some value (stddev?) to know if error is within threshold
 * If some_eror > threshold, recurse
 * Else: break
 *
 */

template <class T>
    requires requires (const T& a, const T& b) {
        a.x <=> b.x;
        a.y <=> b.y;
    }
struct quadtree {
    public:
        quadtree() noexcept : data(nullptr)
            {}

        /// @brief Reserves space for at least n elements, if n is less than the current size, nothing changes
        /// @parameter n Number of elements to reserve space for
        void reserve(size_t n) noexcept {
            if (n < capacity) {
                return;
            }

            capacity = n;
            data = realloc(data, capacity*sizeof(Node));
        }

        /// @brief Inserts value into quadtree based on equality
        /// @parameter value Value to insert
        void insert(T value) {
            size_t head_idx = 0;
            Node& n = data[head_idx];

            while (n != nullptr) {
                Node& next = nullptr;
                size_t j = 0;

                if (value.x < n.value.x) {
                    // West Hemisphere
                    if (value.y < n.value.y) {
                        // South West
                        j = n.sw;
                    } else {
                        // North West
                        j = n.nw;
                    }
                } else {
                    // East Hemisphere
                    if (value.y < n.value.y) {
                        // South East
                        j = n.se;
                    } else {
                        // North East
                        j = n.ne;
                    }
                }

                if (j) {
                    // node already exists, continue
                    head_idx = j;
                    n = data[head_idx];
                    continue;
                }

                // j now points to a unique idx
                if (j > capacity) {
                    data = realloc(data, 2*capacity);
                    capacity *= 2;
                }

                size++;
                data[j] = {
                    .value = value,
                    .nw = 0,
                    .ne = 0,
                    .sw = 0,
                    .ne = 0
                };
            }
        }

    private:
        struct Node {
            T value;
            uint32_t nw;
            uint32_t ne;
            uint32_t sw;
            uint32_t se;
        };

        Node* data;
        size_t size;
        size_t capacity;
};
