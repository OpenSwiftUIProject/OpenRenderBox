//
//  Transition.hpp
//  OpenRenderBox

#pragma once

#include <OpenRenderBoxCxx/Animation/Animation.hpp>
#include <atomic>

namespace ORB {

class Transition {
public:
    enum class Method : uint8_t {
        Default, Diff, Forwards, Backwards, PrefixSuffix, Prefix, Suffix, Binary, None,
    };

    struct Effect {
        uint16_t bits = 0x301;
        uint8_t begin = 0;
        uint8_t duration = 255;

        uint8_t type() const { return bits & type_mask; }
        void set_type(uint8_t value) { bits = (bits & ~type_mask) | (value & type_mask); }
        uint8_t flags() const { return bits >> 14; }
        void set_flags(uint8_t value) { bits = (bits & ~flags_mask) | (value << 14); }
        uint8_t events() const { return (bits & events_mask) >> 8; }
        void set_events(uint8_t value) { bits = (bits & ~events_mask) | ((value << 8) & events_mask); }
        uint8_t argument_count() const;
        bool uses_animation_indices() const { return (bits & animation_indices_mask) != 0; }
        float begin_time() const { return uses_animation_indices() ? 0 : begin * (1.0f / 255); }
        float duration_value() const { return uses_animation_indices() ? 1 : duration * (1.0f / 255); }
        void set_begin_time(float value);
        void set_duration(float value);
        uint8_t animation_index() const { return uses_animation_indices() && begin == duration ? begin : 0; }
        void set_animation_index(uint8_t value) {
            set_insert_animation(value);
            set_remove_animation(value);
        }
        uint8_t insert_animation() const { return uses_animation_indices() ? begin : 0; }
        void set_insert_animation(uint8_t value);
        uint8_t remove_animation() const { return uses_animation_indices() ? duration : 0; }
        void set_remove_animation(uint8_t value);
        size_t hash() const;

    private:
        static constexpr uint16_t type_mask = 0x007f;
        static constexpr uint16_t animation_indices_mask = 0x0080;
        static constexpr uint16_t events_mask = 0x3f00;
        static constexpr uint16_t flags_mask = 0xc000;
    };

    union EffectOrArg {
        Effect effect;
        uint32_t argument;

        EffectOrArg() {}
        explicit EffectOrArg(Effect value) : effect(value) {}
        explicit EffectOrArg(uint32_t value) : argument(value) {}
    };

    Transition() = default;
    Transition(const Transition &other);
    Transition &operator=(const Transition &) = delete;
    virtual ~Transition();

    void retain() { _ref_count.fetch_add(1, std::memory_order_relaxed); }
    void release() {
        if (_ref_count.fetch_sub(1, std::memory_order_acq_rel) == 1) delete this;
    }
    bool is_unique() const {
        if (_ref_count.load(std::memory_order_relaxed) != 1) return false;
        std::atomic_thread_fence(std::memory_order_acquire);
        return true;
    }

    Method method() const { return _method; }
    void set_method(Method value) { _method = value; }
    bool replaceable() const { return _replaceable; }
    void set_replaceable(bool value) { _replaceable = value; }
    uint32_t max_changes() const { return _max_changes; }
    void set_max_changes(uint32_t value) { _max_changes = value; }
    float add_remove_duration() const { return _add_remove_duration * (1.0f / 255); }
    void set_add_remove_duration(float value);

    const vector<EffectOrArg, 4, uint32_t> &effects() const { return _effects; }
    void add_effect(Effect effect, const uint32_t *arguments);
    const Animation &animation() const { return _animation; }
    void set_animation(const Animation &value) { _animation = value; }
    void clear_animation() { _animation.clear(); }
    bool operator==(const Transition &other) const;
    size_t hash() const;

private:
    std::atomic<uint32_t> _ref_count{1};
    Method _method = Method::Default;
    bool _replaceable = false;
    uint8_t _add_remove_duration = 32;
    uint32_t _max_changes = UINT32_MAX;
    vector<EffectOrArg, 4, uint32_t> _effects;
    Animation _animation;
};

static_assert(sizeof(Transition::Effect) == 4);
static_assert(sizeof(Transition::EffectOrArg) == 4);
#if ORB_TARGET_RT_64_BIT
static_assert(sizeof(Transition) == 0x60);
#endif

} /* ORB */
