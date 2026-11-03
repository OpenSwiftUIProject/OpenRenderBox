//
//  refcounted_ptr.hpp
//  OpenRenderBox

#pragma once

#include <utility>

namespace ORB {

template <typename T>
class refcounted_ptr {
public:
    refcounted_ptr() noexcept = default;

    // Adopt the reference held by a newly created value.
    explicit refcounted_ptr(T *value) noexcept : _p(value) {}

    refcounted_ptr(const refcounted_ptr &other) noexcept : _p(other._p) {
        if (_p) _p->retain();
    }

    refcounted_ptr(refcounted_ptr &&other) noexcept
        : _p(std::exchange(other._p, nullptr)) {}

    ~refcounted_ptr() {
        if (_p) _p->release();
    }

    refcounted_ptr &operator=(const refcounted_ptr &other) noexcept {
        if (_p != other._p) {
            T *value = other._p;
            if (value) value->retain();
            if (_p) _p->release();
            _p = value;
        }
        return *this;
    }

    refcounted_ptr &operator=(refcounted_ptr &&other) noexcept {
        if (this != &other) {
            if (_p) _p->release();
            _p = std::exchange(other._p, nullptr);
        }
        return *this;
    }

    T *get() const noexcept { return _p; }
    T *operator->() const noexcept { return _p; }
    T &operator*() const noexcept { return *_p; }

private:
    T *_p = nullptr;
};

} /* ORB */
