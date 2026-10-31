//
//  Animation.hpp
//  OpenRenderBox

#pragma once

#include <OpenRenderBoxCxx/Vector/vector.hpp>

namespace ORB {

class Animation {
public:
    union TermOrArg {
        uint32_t type;
        float argument;

        TermOrArg() {}
        explicit TermOrArg(uint32_t value) : type(value) {}
        explicit TermOrArg(float value) : argument(value) {}
    };

    Animation() = default;
    Animation(const Animation &other);
    Animation &operator=(const Animation &other);

    bool empty() const { return _terms.empty(); }
    void clear() { _terms.clear(); }
    float active_duration() const;
    void add_spring(double duration, double mass, double stiffness, double damping, double velocity);
    bool operator==(const Animation &other) const;
    size_t hash() const;

private:
    vector<TermOrArg, 6, uint32_t> _terms;
};

static_assert(sizeof(Animation::TermOrArg) == 4);
#if ORB_TARGET_RT_64_BIT
static_assert(sizeof(Animation) == 0x28);
#endif

} /* ORB */
