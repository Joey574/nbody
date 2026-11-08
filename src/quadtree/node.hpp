#pragma once
#include <utility>
#include <cstddef>

struct nodes {
    public:
        // default constructor
        nodes() = default;

        // copies
        nodes(const nodes& other) noexcept = delete;
        nodes& operator=(const nodes& other) noexcept = delete;

        // move constructor
        nodes(nodes&& other) noexcept :
            size_(std::exchange(other.size_, 0)),
            capacity_(std::exchange(other.capacity_, 0)),
            node_x_(std::exchange(other.node_x_, nullptr)),
            node_y_(std::exchange(other.node_y_, nullptr)),
            node_m_(std::exchange(other.node_m_, nullptr)),
            node_next_(std::exchange(other.node_next_, nullptr)),
            node_children_(std::exchange(other.node_children_, nullptr))
        {}

        // move operator
        nodes& operator=(nodes&& other) noexcept {
            if (this == &other) return *this;
            cleanup();

            size_ = std::exchange(other.size_, 0);
            capacity_ = std::exchange(other.capacity_, 0);
            node_x_ = std::exchange(other.node_x_, nullptr);
            node_y_ = std::exchange(other.node_y_, nullptr);
            node_m_ = std::exchange(other.node_m_, nullptr);
            node_next_ = std::exchange(other.node_next_, nullptr);
            node_children_ = std::exchange(other.node_children_, nullptr);
            return *this;
        }

        // parameterized constructor
        nodes(size_t n) { reserve(n); }

        void reserve(size_t n);

        void clear();
        void zero();

        void push(float x, float y, float m);
        void push_empty(size_t next);

        void set(size_t i, float x, float y, float m);

        inline bool is_leaf(size_t i) const { return node_children_[i] == 0; }
        inline bool is_branch(size_t i) const { return node_children_[i] != 0; }
        inline bool is_empty(size_t i) const { return node_m_[i] == 0; }

        inline size_t size() const { return size_; }
        inline const float* __restrict X() const { return node_x_; }
        inline const float* __restrict Y() const { return node_y_; }
        inline const float* __restrict M() const { return node_m_; }
        inline const size_t* __restrict N() const { return node_next_; }
        inline const size_t* __restrict C() const { return node_children_; }

        inline float* __restrict X() { return node_x_; }
        inline float* __restrict Y() { return node_y_; }
        inline float* __restrict M() { return node_m_; }
        inline size_t* __restrict N() { return node_next_; }
        inline size_t* __restrict C() { return node_children_; }

        ~nodes() { cleanup(); }
    private:
        size_t size_ = 0;
        size_t capacity_ = 0;

        float* __restrict node_x_ = nullptr;
        float* __restrict node_y_ = nullptr;
        float* __restrict node_m_ = nullptr;
        size_t* __restrict node_next_ = nullptr;
        size_t* __restrict node_children_ = nullptr;

        void cleanup();
};
