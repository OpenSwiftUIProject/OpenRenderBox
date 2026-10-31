//
//  Transition.cpp
//  OpenRenderBox

#include <OpenRenderBoxCxx/Animation/Transition.hpp>
#include <cmath>
#include <cstring>

namespace ORB {

static uint8_t transition_time(float value) {
    if (std::isnan(value)) return 0;
    return uint8_t(std::fma(std::fmin(std::fmax(value, 0.0f), 1.0f), 255.0f, 0.5f));
}

Transition::Transition(const Transition &other)
    : _method(other._method), _replaceable(other._replaceable),
      _add_remove_duration(other._add_remove_duration), _max_changes(other._max_changes),
      _animation(other._animation) {
    _effects.reserve(other._effects.size());
    for (uint32_t i = 0; i < other._effects.size(); ++i) {
        _effects.push_back(other._effects[i]);
    }
}

Transition::~Transition() = default;

void Transition::set_add_remove_duration(float value) {
    _add_remove_duration = transition_time(value);
}

void Transition::add_effect(Effect effect, const uint32_t *arguments) {
    uint8_t count = effect.argument_count();
    _effects.reserve(_effects.size() + 1 + count);
    _effects.push_back(EffectOrArg(effect));
    for (uint8_t i = 0; i < count; ++i) {
        _effects.push_back(EffectOrArg(arguments[i]));
    }
}

bool Transition::operator==(const Transition &other) const {
    return _method == other._method && _replaceable == other._replaceable
        && _add_remove_duration == other._add_remove_duration && _max_changes == other._max_changes
        && _animation == other._animation && _effects.size() == other._effects.size()
        && (_effects.empty() || std::memcmp(_effects.data(), other._effects.data(), _effects.size() * sizeof(EffectOrArg)) == 0);
}

size_t Transition::hash() const {
    size_t hash = (size_t(_method) ^ (size_t(0) - size_t(_replaceable))) * 33 ^ _animation.hash();
    for (uint32_t i = 0; i < _effects.size();) {
        const Effect &effect = _effects[i++].effect;
        hash = hash * 33 ^ effect.hash();
        for (uint8_t count = effect.argument_count(); count != 0; --count) {
            hash = hash * 33 ^ _effects[i++].argument;
        }
    }
    return hash;
}

uint8_t Transition::Effect::argument_count() const {
    static constexpr uint8_t counts[] = {0, 0, 1, 2, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 0, 0};
    return type() < sizeof(counts) ? counts[type()] : 2;
}

void Transition::Effect::set_begin_time(float value) {
    if (uses_animation_indices()) {
        bits &= ~animation_indices_mask;
        duration = 255;
    }
    begin = transition_time(value);
}

void Transition::Effect::set_duration(float value) {
    if (uses_animation_indices()) {
        bits &= ~animation_indices_mask;
        begin = 0;
    }
    duration = transition_time(value);
}

void Transition::Effect::set_insert_animation(uint8_t value) {
    if (!uses_animation_indices()) {
        bits |= animation_indices_mask;
        duration = 0;
    }
    begin = value;
}

void Transition::Effect::set_remove_animation(uint8_t value) {
    if (!uses_animation_indices()) {
        bits |= animation_indices_mask;
        begin = 0;
    }
    duration = value;
}

size_t Transition::Effect::hash() const {
    return (bits & 0x3fff) | ((bits >> 2) & 0x3000) | (size_t(begin) << 16) | (size_t(duration) << 24);
}

} /* ORB */
