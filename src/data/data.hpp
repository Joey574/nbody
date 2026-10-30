#pragma once
#include <cstdlib>
#include <string.h>
#include <cstddef>
#include <vector>
#include <algorithm>
#include <omp.h>

#include "../definitions/macros.hpp"
#include "../matrix/matrix.hpp"
#include "../util/util.hpp"

/// @brief Hold the raw underlying simulation data and provides a simple interface to access it
struct data {
    private:
    size_t bodies_ = 0;
    float* __restrict posx_ = nullptr;
    float* __restrict posy_ = nullptr;
    float* __restrict velx_ = nullptr;
    float* __restrict vely_ = nullptr;
    float* __restrict mass_ = nullptr;
    matrix accx_;
    matrix accy_;

    static float* grow(float* old, size_t old_n, const float* add, size_t add_n) {
        const size_t new_n = old_n + add_n;
        float* tmp = (float*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(new_n * sizeof(float)));

        if (old_n) std::memcpy(tmp, old, old_n * sizeof(float));
        if (add_n) std::memcpy(tmp + old_n, add, add_n * sizeof(float));

        free(old);
        return tmp;
    }

    public:

    // default constructor
    data() :
        bodies_(0),
        posx_(nullptr),
        posy_(nullptr),
        velx_(nullptr),
        vely_(nullptr),
        mass_(nullptr),
        accx_(matrix()),
        accy_(matrix()) {}

    // move constructor
    data(data&& other) noexcept :
        bodies_(other.bodies_),
        posx_(other.posx_),
        posy_(other.posy_),
        velx_(other.velx_),
        vely_(other.vely_),
        mass_(other.mass_),
        accx_(other.accx_),
        accy_(other.accy_) {
            other.bodies_ = 0;
            other.posx_ = nullptr;
            other.posy_ = nullptr;
            other.velx_ = nullptr;
            other.vely_ = nullptr;
            other.mass_ = nullptr;
        }

    // copy constructor
    data(const data& other) noexcept :
        bodies_(other.bodies_),
        accx_(other.accx_),
        accy_(other.accy_) {
            posx_ = (float*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(bodies_*sizeof(float)));
            posy_ = (float*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(bodies_*sizeof(float)));
            velx_ = (float*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(bodies_*sizeof(float)));
            vely_ = (float*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(bodies_*sizeof(float)));
            mass_ = (float*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(bodies_*sizeof(float)));

            memcpy(posx_, other.posx_, bodies_*sizeof(float));
            memcpy(posy_, other.posy_, bodies_*sizeof(float));
            memcpy(velx_, other.velx_, bodies_*sizeof(float));
            memcpy(vely_, other.vely_, bodies_*sizeof(float));
            memcpy(mass_, other.mass_, bodies_*sizeof(float));
    }

    // parameterized constructor
    data(size_t n, bool init_acc) :
        bodies_(n),
        posx_((float*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(n*sizeof(float)))),
        posy_((float*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(n*sizeof(float)))),
        velx_((float*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(n*sizeof(float)))),
        vely_((float*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(n*sizeof(float)))),
        mass_((float*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(n*sizeof(float)))),
        accx_(init_acc ? matrix(omp_get_max_threads(), n) : matrix()),
        accy_(init_acc ? matrix(omp_get_max_threads(), n) : matrix()) {}


    // move operator
    data& operator = (data&& other) noexcept {
        bodies_= other.bodies_;
        posx_ = other.posx_;
        posy_ = other.posy_;
        velx_ = other.velx_;
        vely_ = other.vely_;
        mass_ = other.mass_;
        accx_ = other.accx_;
        accy_ = other.accy_;

        other.bodies_ = 0;
        other.posx_ = nullptr;
        other.posy_ = nullptr;
        other.velx_ = nullptr;
        other.vely_ = nullptr;
        other.mass_ = nullptr;
        return *this;
    }

    // copy operator
    data& operator = (const data& other) noexcept {
        bodies_ = other.bodies_;
        accx_ = other.accx_;
        accy_ = other.accy_;

        posx_ = (float*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(bodies_*sizeof(float)));
        posy_ = (float*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(bodies_*sizeof(float)));
        velx_ = (float*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(bodies_*sizeof(float)));
        vely_ = (float*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(bodies_*sizeof(float)));
        mass_ = (float*)aligned_alloc(MEM_ALIGNMENT, util::aligned_size(bodies_*sizeof(float)));

        memcpy(posx_, other.posx_, bodies_*sizeof(float));
        memcpy(posy_, other.posy_, bodies_*sizeof(float));
        memcpy(velx_, other.velx_, bodies_*sizeof(float));
        memcpy(vely_, other.vely_, bodies_*sizeof(float));
        memcpy(mass_, other.mass_, bodies_*sizeof(float));
        return *this;
    }

    // deconstructor
    ~data() {
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
    }

    const inline size_t bodies() const noexcept { return bodies_; }
    const inline float* __restrict posx() const noexcept { return posx_; }
    const inline float* __restrict posy() const noexcept { return posy_; }
    const inline float* __restrict velx() const noexcept { return velx_; }
    const inline float* __restrict vely() const noexcept { return vely_; }
    const inline float* __restrict mass() const noexcept { return mass_; }

    inline float* __restrict posx() noexcept { return posx_; }
    inline float* __restrict posy() noexcept { return posy_; }
    inline float* __restrict velx() noexcept { return velx_; }
    inline float* __restrict vely() noexcept { return vely_; }
    inline float* __restrict mass() noexcept { return mass_; }

    matrix& accx() noexcept { return accx_; }
    matrix& accy() noexcept { return accy_; }

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
    inline void append(data& other) {
        const size_t old_n = this->bodies_;
        const size_t add_n = other.bodies_;

        this->posx_ = grow(posx_, old_n, other.posx_, add_n);
        this->posy_ = grow(posy_, old_n, other.posy_, add_n);
        this->velx_ = grow(velx_, old_n, other.velx_, add_n);
        this->vely_ = grow(vely_, old_n, other.vely_, add_n);
        this->mass_ = grow(mass_, old_n, other.mass_, add_n);

        this->bodies_ = old_n + add_n;
        this->accx_ = matrix(omp_get_max_threads(), this->bodies_);
        this->accy_ = matrix(omp_get_max_threads(), this->bodies_);
    }
};
