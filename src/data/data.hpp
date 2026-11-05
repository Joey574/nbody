#pragma once
#include <cstdlib>
#include <string.h>
#include <cstddef>
#include <algorithm>
#include <vector>
#include <omp.h>

#include "../matrix/matrix.hpp"
#include "../util/util.hpp"

/// @brief Hold the raw underlying simulation data and provides a simple interface to access it
struct data {
    public:
        data() = default;

        // move constructor
        data(data&& other) noexcept :
            bodies_(std::exchange(other.bodies_, 0)),
            posx_(std::exchange(other.posx_, nullptr)),
            posy_(std::exchange(other.posy_, nullptr)),
            velx_(std::exchange(other.velx_, nullptr)),
            vely_(std::exchange(other.vely_, nullptr)),
            mass_(std::exchange(other.mass_, nullptr)),
            accx_(std::exchange(other.accx_, matrix{})),
            accy_(std::exchange(other.accy_, matrix{}))
            {}

        // copy constructor
        data(const data& other) noexcept :
            bodies_(other.bodies_),
            accx_(other.accx_),
            accy_(other.accy_),
            posx_((float*)util::mem_aligned_alloc(other.bodies_*sizeof(float))),
            posy_((float*)util::mem_aligned_alloc(other.bodies_*sizeof(float))),
            velx_((float*)util::mem_aligned_alloc(other.bodies_*sizeof(float))),
            vely_((float*)util::mem_aligned_alloc(other.bodies_*sizeof(float))),
            mass_((float*)util::mem_aligned_alloc(other.bodies_*sizeof(float))) {
                const size_t bytes = bodies_*sizeof(float);
                memcpy(posx_, other.posx_, bytes);
                memcpy(posy_, other.posy_, bytes);
                memcpy(velx_, other.velx_, bytes);
                memcpy(vely_, other.vely_, bytes);
                memcpy(mass_, other.mass_, bytes);
        }

        // parameterized constructor
        data(size_t n, bool init_acc) :
            bodies_(n),
            posx_((float*)util::mem_aligned_alloc(n*sizeof(float))),
            posy_((float*)util::mem_aligned_alloc(n*sizeof(float))),
            velx_((float*)util::mem_aligned_alloc(n*sizeof(float))),
            vely_((float*)util::mem_aligned_alloc(n*sizeof(float))),
            mass_((float*)util::mem_aligned_alloc(n*sizeof(float))),
            accx_(init_acc ? matrix(omp_get_max_threads(), n) : matrix{}),
            accy_(init_acc ? matrix(omp_get_max_threads(), n) : matrix{})
            {}


        // move operator
        data& operator = (data&& other) noexcept {
            if (this == &other) return *this;
            clear();

            bodies_= std::exchange(other.bodies_, 0);
            posx_ = std::exchange(other.posx_, nullptr);
            posy_ = std::exchange(other.posy_, nullptr);
            velx_ = std::exchange(other.velx_, nullptr);
            vely_ = std::exchange(other.vely_, nullptr);
            mass_ = std::exchange(other.mass_, nullptr);
            accx_ = std::exchange(other.accx_, matrix{});
            accy_ = std::exchange(other.accy_, matrix{});
            return *this;
        }

        // copy operator
        data& operator = (const data& other) noexcept {
            if (this == &other) return *this;
            clear();

            bodies_ = other.bodies_;
            accx_ = other.accx_;
            accy_ = other.accy_;

            const size_t bytes = bodies_*sizeof(float);
            posx_ = (float*)util::mem_aligned_alloc(bytes);
            posy_ = (float*)util::mem_aligned_alloc(bytes);
            velx_ = (float*)util::mem_aligned_alloc(bytes);
            vely_ = (float*)util::mem_aligned_alloc(bytes);
            mass_ = (float*)util::mem_aligned_alloc(bytes);

            memcpy(posx_, other.posx_, bytes);
            memcpy(posy_, other.posy_, bytes);
            memcpy(velx_, other.velx_, bytes);
            memcpy(vely_, other.vely_, bytes);
            memcpy(mass_, other.mass_, bytes);
            return *this;
        }

        // deconstructor
        ~data() {
            clear();
        }

        inline void clear() noexcept {
            bodies_ = 0;
            free(posx_);
            free(posy_);
            free(velx_);
            free(vely_);
            free(mass_);

            posx_ =  nullptr;
            posy_ =  nullptr;
            velx_ =  nullptr;
            vely_ =  nullptr;
            mass_ =  nullptr;
            accx_ = matrix{};
            accy_ = matrix{};
        }

        const inline size_t bodies() const noexcept { return bodies_; }
        const inline float* __restrict posx() const noexcept { return posx_; }
        const inline float* __restrict posy() const noexcept { return posy_; }
        const inline float* __restrict velx() const noexcept { return velx_; }
        const inline float* __restrict vely() const noexcept { return vely_; }
        const inline float* __restrict mass() const noexcept { return mass_; }

        const inline matrix& accx() const noexcept { return accx_; }
        const inline matrix& accy() const noexcept { return accy_; }

        inline float* __restrict posx() noexcept { return posx_; }
        inline float* __restrict posy() noexcept { return posy_; }
        inline float* __restrict velx() noexcept { return velx_; }
        inline float* __restrict vely() noexcept { return vely_; }
        inline float* __restrict mass() noexcept { return mass_; }

        inline matrix& accx() noexcept { return accx_; }
        inline matrix& accy() noexcept { return accy_; }

        inline void zero_acc() noexcept { accx_.zero(); accy_.zero(); }

        inline void sort() noexcept {
            std::vector<std::pair<float, size_t>> values(bodies_);

            #pragma omp parallel for simd schedule(static)
            for (size_t i = 0; i < bodies_; i++) {
                values[i] = { posx_[i]*posx_[i]+posy_[i]*posy_[i], i };
            }

            std::sort(values.begin(), values.end());

            for (size_t i = 0; i < bodies_-1; i++) {
                size_t cur = i;
                size_t next = values[cur].second;

                while (next != i) {
                    std::swap(posx_[cur], posx_[next]);
                    std::swap(posy_[cur], posy_[next]);
                    std::swap(velx_[cur], velx_[next]);
                    std::swap(vely_[cur], vely_[next]);
                    std::swap(mass_[cur], mass_[next]);
                    values[cur].second = cur;
                    cur = next;
                    next = values[cur].second;
                }
                values[cur].second = cur;
            }
        }

        // appends an existing data object to itself
        inline void append(const data& other, bool init_acc) {
            const size_t n = this->bodies_;
            const size_t n_append = other.bodies_;

            this->posx_ = grow(posx_, other.posx_, n, n_append);
            this->posy_ = grow(posy_, other.posy_, n, n_append);
            this->velx_ = grow(velx_, other.velx_, n, n_append);
            this->vely_ = grow(vely_, other.vely_, n, n_append);
            this->mass_ = grow(mass_, other.mass_, n, n_append);

            this->bodies_ = n + n_append;
            this->accx_ = init_acc ? matrix(omp_get_max_threads(), this->bodies_) : matrix{};
            this->accy_ = init_acc ? matrix(omp_get_max_threads(), this->bodies_) : matrix{};
        }

    private:
        size_t bodies_ = 0;
        float* __restrict posx_ = nullptr;
        float* __restrict posy_ = nullptr;
        float* __restrict velx_ = nullptr;
        float* __restrict vely_ = nullptr;
        float* __restrict mass_ = nullptr;
        matrix accx_ = matrix{};
        matrix accy_ = matrix{};

        static float* grow(float* __restrict data, const float* __restrict append, size_t n, size_t n_append) {
            const size_t total = n + n_append;
            float* tmp = (float*)util::mem_aligned_alloc(total*sizeof(float));

            if (n) memcpy(tmp, data, n*sizeof(float));
            if (n_append) memcpy(tmp + n, append, n_append*sizeof(float));

            free(data);
            return tmp;
        }
};
