#pragma once
#include <cassert>
#include <cstdint>

struct timing_t {
    typedef uint64_t (*tick_func_t)();

    timing_t(tick_func_t func, uint64_t period = 0)
        : get_time_(func)
        , period_(period)
    {
        reset();
    }

    float deltaf() const
    {
        assert(get_time_);
        uint64_t nval = get_time_();
        if (old_ > nval)
            return 0.f;
        const uint64_t diff = nval - old_;
        return float(diff) / float(period_ ? period_ : 1);
    }

    void reset()
    {
        assert(get_time_);
        old_ = get_time_();
    }

    void step()
    {
        old_ += period_;
    }

    bool done() const {
        assert(get_time_);
        const uint64_t nval = get_time_();
        return (nval-old_) > period_;
    }

    uint64_t deltai() const
    {
        assert(get_time_);
        const uint64_t nval = get_time_();
        if (old_ > nval)
            return 0ull;
        const uint64_t diff = nval - old_;
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

    uint64_t period_;

protected:
    uint64_t old_;
    tick_func_t get_time_;
};
