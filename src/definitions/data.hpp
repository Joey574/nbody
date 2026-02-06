#pragma once
#include <cstddef>
#include <omp.h>
#include <iterator>

#include "matrix.hpp"
#include "macros.hpp"
#include "../util/util.hpp"

/// @brief Hold the raw underlying simulation data and provides a simple interface to access it
struct data {
    public:
    struct point {
        float x;
        float y;
        float m;
    };

    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = point;
        using pointer = void;
        using reference = point&;

        // constructor
        Iterator(float* xptr, float* yptr, float* mptr) : xptr_(xptr), yptr_(yptr), mptr_(mptr) {}

        point operator*() const { return point{*xptr_, *yptr_, *mptr_}; }

        Iterator& operator++() {
            xptr_++;
            yptr_++;
            mptr_++;
            return *this;
        }

        friend bool operator == (const Iterator& a, const Iterator& b) {
            return a.xptr_ == b.xptr_;
        }
        friend bool operator != (const Iterator& a, const Iterator& b) {
            return !(a == b);
        }

        private:
        float* xptr_;
        float* yptr_;
        float* mptr_;
    };

    Iterator begin() { return Iterator{posx_, posy_, mass_}; }
    Iterator end() { return Iterator{posx_+bodies_, posy_+bodies_, mass_+bodies_}; }

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

            std::memcpy(posx_, other.posx_, bodies_*sizeof(float));
            std::memcpy(posy_, other.posy_, bodies_*sizeof(float));
            std::memcpy(velx_, other.velx_, bodies_*sizeof(float));
            std::memcpy(vely_, other.vely_, bodies_*sizeof(float));
            std::memcpy(mass_, other.mass_, bodies_*sizeof(float));
    }

    // custom constructor
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
            
        std::memcpy(posx_, other.posx_, bodies_*sizeof(float));
        std::memcpy(posy_, other.posy_, bodies_*sizeof(float));
        std::memcpy(velx_, other.velx_, bodies_*sizeof(float));
        std::memcpy(vely_, other.vely_, bodies_*sizeof(float));
        std::memcpy(mass_, other.mass_, bodies_*sizeof(float));
        return *this;
    }

    // deconstructor
    ~data() {
        bodies_ = 0;
        if (posx_) { free(posx_); posx_ =  nullptr; }
        if (posy_) { free(posy_); posy_ =  nullptr; }
        if (velx_) { free(velx_); velx_ =  nullptr; }
        if (vely_) { free(vely_); vely_ =  nullptr; }
        if (mass_) { free(mass_); mass_ =  nullptr; }
    }

    constexpr inline size_t bodies() const noexcept { return bodies_; }
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

    private:
    size_t bodies_;
    float* __restrict posx_;
    float* __restrict posy_;
    float* __restrict velx_;
    float* __restrict vely_;
    float* __restrict mass_;
    matrix accx_;
    matrix accy_;
};