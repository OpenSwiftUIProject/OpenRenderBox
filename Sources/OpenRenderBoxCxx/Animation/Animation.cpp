//
//  Animation.cpp
//  OpenRenderBox

#include <OpenRenderBoxCxx/Animation/Animation.hpp>
#include <cstring>

namespace ORB {

Animation::Animation(const Animation &other) {
    *this = other;
}

Animation &Animation::operator=(const Animation &other) {
    if (this != &other) {
        _terms.reserve(other._terms.size());
        _terms.clear();
        for (uint32_t i = 0; i < other._terms.size(); ++i) {
            _terms.push_back(other._terms[i]);
        }
    }
    return *this;
}

float Animation::active_duration() const {
    return _terms.empty() ? 0 : _terms[1].argument;
}

void Animation::add_spring(double duration, double mass, double stiffness, double damping, double velocity) {
    bool has_velocity = velocity != 0;
    _terms.reserve(_terms.size() + (has_velocity ? 6 : 5));
    _terms.push_back(TermOrArg(uint32_t(has_velocity ? 10 : 9)));
    _terms.push_back(TermOrArg(float(duration)));
    _terms.push_back(TermOrArg(float(mass)));
    _terms.push_back(TermOrArg(float(stiffness)));
    _terms.push_back(TermOrArg(float(damping)));
    if (has_velocity) _terms.push_back(TermOrArg(float(velocity)));
}

bool Animation::operator==(const Animation &other) const {
    return _terms.size() == other._terms.size()
        && (_terms.empty() || std::memcmp(_terms.data(), other._terms.data(), _terms.size() * sizeof(TermOrArg)) == 0);
}

size_t Animation::hash() const {
    size_t hash = 2166136261;
    for (uint32_t i = 0; i < _terms.size(); ++i) {
        uint32_t word;
        std::memcpy(&word, &_terms[i], sizeof(word));
        hash = hash * 16777619 ^ word;
    }
    return hash;
}

} /* ORB */
