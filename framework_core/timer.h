#pragma once
#include <cassert>
#include <cstdint>

template <typename type_t = uint64_t>
struct timing_t {
    typedef type_t (*tick_func_t)();

    timing_t(tick_func_t func = nullptr, type_t period = 0)
        : func_(func)
        , period_(period)
    {
        reset();
    }

    float deltaf() const
    {
        type_t nval = get_time();
        if (old_ > nval)
            return 0.f;
        const type_t diff = nval - old_;
        return float(diff) / float(period_ ? period_ : 1);
    }

    void reset()
    {
        old_ = get_time();
    }

    void step()
    {
        old_ += period_;
    }

    bool done() const {
        const type_t nval = get_time();
        return (nval-old_) > period_;
    }

    uint64_t deltai() const
    {
        const type_t nval = get_time();
        if (old_ > nval)
            return 0ull;
        const type_t diff = nval - old_;
        return period_ ? (diff / period_) : 0ull;
    }

    bool frame() {
        float delta = deltaf();
        if (delta < 1.f) {
            return false;
        }
        else {
            // watch out for explosions
            if (delta>20.f) {
                reset();
            }
            else {
                step();
            }
            return true;
        }
    }

    type_t period_;
    tick_func_t func_;

protected:

    type_t get_time() const {
        return func_ ? func_() : 0;
    }

    type_t old_;
};
